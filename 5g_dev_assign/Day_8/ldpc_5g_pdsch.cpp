/**
 * @file ldpc_5g_pdsch.cpp
 * @brief 5G NR LDPC Encoder Implementation for PDSCH
 * @description This file implements Low Density Parity Check (LDPC) encoding
 *              for the Physical Downlink Shared Channel (PDSCH) in 5G NR
 *              according to 3GPP TS 38.212 specifications.
 *
 * @author Designed By Harsh Prasad
 * @standard 3GPP TS 38.212 Section 5.3.2 - Channel Coding
 */

#include <algorithm> // For algorithms like fill, copy, etc.)
#include <bitset>    // For binary operations and bit manipulation
#include <cmath>     // For mathematical functions (ceil, floor, etc.)
#include <cstdlib>   // For rand() and srand()
#include <ctime>     // For time() function
#include <iomanip>   // For formatting output (setw, setfill, etc.)
#include <iostream>  // For input/output operations (cout, cin, etc.)
#include <stdexcept> // For exception handling (runtime_error, etc.)
#include <string>    // For string operations
#include <vector>    // For dynamic arrays (std::vector)

// ============================================================================
// CONSTANTS AND ENUMERATIONS
// ============================================================================

/**
 * @enum BaseGraphType
 * @brief Defines the two base graphs used in 5G NR LDPC
 *
 * Base Graph 1: Used for larger code blocks (K > 292)
 *               - 46 rows × 68 columns
 *               - Supports higher code rates (1/3, 2/3, etc.)
 *
 * Base Graph 2: Used for smaller code blocks (K ≤ 292)
 *               - 42 rows × 52 columns
 *               - More suitable for lower code rates
 */
enum class BaseGraphType {
  BG1 = 1, // Base Graph 1: For large transport blocks
  BG2 = 2  // Base Graph 2: For small transport blocks
};

/**
 * @brief Maximum lifting size (Z) supported in 5G NR LDPC
 * Lifting size determines the expansion factor of the base graph
 */
const int MAX_LIFTING_SIZE = 384;

/**
 * @brief Minimum lifting size (Z) supported in 5G NR LDPC
 */
const int MIN_LIFTING_SIZE = 2;

/**
 * @brief Number of rows in Base Graph 1 (BG1)
 * This represents the number of parity check equations
 */
const int BG1_ROWS = 46;

/**
 * @brief Number of columns in Base Graph 1 (BG1)
 * This includes both systematic bits and parity bits
 */
const int BG1_COLUMNS = 68;

/**
 * @brief Number of rows in Base Graph 2 (BG2)
 */
const int BG2_ROWS = 42;

/**
 * @brief Number of columns in Base Graph 2 (BG2)
 */
const int BG2_COLUMNS = 52;

/**
 * @brief Number of information columns (K) in Base Graph 1
 * These are the columns that carry actual data before encoding
 */
const int BG1_K = 22;

/**
 * @brief Number of information columns (K) in Base Graph 2
 */
const int BG2_K = 10;

// ============================================================================
// HELPER STRUCTURES
// ============================================================================

/**
 * @struct LDPCParameters
 * @brief Structure to hold all LDPC encoding parameters
 *
 * This structure encapsulates all necessary parameters for LDPC encoding
 * including base graph selection, lifting size, code rate, etc.
 */
struct LDPCParameters {
  BaseGraphType baseGraph; // Which base graph to use (BG1 or BG2)
  int liftingSize;         // Z: Expansion factor (2 to 384)
  int Kb;                  // Number of information bit columns
  int numRows;             // Number of rows in parity check matrix (M)
  int numCols;             // Number of columns in parity check matrix (N)
  int K;                   // Number of information bits = Kb × Z
  int N;                   // Total codeword length = numCols × Z
  double codeRate;         // Code rate = K/N
  int numFillerBits;       // Number of filler bits to add (for padding)

  /**
   * @brief Default constructor initializes all parameters to zero
   */
  LDPCParameters()
      : baseGraph(BaseGraphType::BG1), liftingSize(0), Kb(0), numRows(0),
        numCols(0), K(0), N(0), codeRate(0.0), numFillerBits(0) {}
};

// ============================================================================
// LIFTING SIZE CALCULATION
// ============================================================================

/**
 * @brief Calculate the appropriate lifting size (Z) for given parameters
 *
 * The lifting size Z is chosen from a set of valid values defined in 3GPP.
 * The valid Z values are organized in 8 sets (i_LS = 0 to 7), where:
 * - Set 0-4: Z values from 2 to 128 (smaller blocks)
 * - Set 5-7: Z values from 160 to 384 (larger blocks)
 *
 * @param Kb Number of information columns
 * @param K_prime Number of bits including CRC (after CRC attachment)
 * @return Calculated lifting size Z
 */
int calculateLiftingSize(int Kb, int K_prime) {
  // Valid lifting sizes according to 3GPP TS 38.212 Table 5.3.2-1
  // Each set contains specific Z values that maintain optimal LDPC properties
  const std::vector<std::vector<int>> liftingSizeSets = {
      {2, 4, 8, 16, 32, 64, 128, 256},  // Set 0: Small blocks (factor of 2)
      {3, 6, 12, 24, 48, 96, 192, 384}, // Set 1: Factor of 3
      {5, 10, 20, 40, 80, 160, 320},    // Set 2: Factor of 5
      {7, 14, 28, 56, 112, 224},        // Set 3: Factor of 7
      {9, 18, 36, 72, 144, 288},        // Set 4: Factor of 9
      {11, 22, 44, 88, 176, 352},       // Set 5: Factor of 11
      {13, 26, 52, 104, 208},           // Set 6: Factor of 13
      {15, 30, 60, 120, 240}            // Set 7: Factor of 15
  };

  // Calculate minimum required Z based on the information bits
  // We need: Kb × Z ≥ K_prime (to accommodate all information bits)
  int minZ = static_cast<int>(std::ceil(static_cast<double>(K_prime) / Kb));

  // Find the smallest Z from the valid sets that satisfies our requirement
  int selectedZ =
      MAX_LIFTING_SIZE; // Start with maximum, then find smaller valid value

  // Iterate through all lifting size sets
  for (const auto &set : liftingSizeSets) {
    // Iterate through Z values in this set
    for (int z : set) {
      // Check if this Z is large enough and smaller than current selection
      if (z >= minZ && z < selectedZ) {
        selectedZ = z; // Update to this better (smaller) value
      }
    }
  }

  // Validate that we found a valid Z
  if (selectedZ == MAX_LIFTING_SIZE && selectedZ < minZ) {
    throw std::runtime_error(
        "Cannot find valid lifting size for given parameters");
  }

  return selectedZ;
}

// ============================================================================
// BASE GRAPH SELECTION
// ============================================================================

/**
 * @brief Determine which base graph to use based on transport block size
 *
 * Selection criteria (from 3GPP TS 38.212):
 * - Use BG1 if: TBS > 292 bits (larger blocks)
 * - Use BG2 if: TBS ≤ 292 bits (smaller blocks)
 *
 * Additionally, code rate influences the choice:
 * - BG1 is optimized for higher code rates (R > 2/3)
 * - BG2 is optimized for lower code rates (R ≤ 2/3)
 *
 * @param transportBlockSize Size of transport block after CRC attachment
 * @param codeRate Target code rate (K/N)
 * @return Selected base graph type (BG1 or BG2)
 */
BaseGraphType selectBaseGraph(int transportBlockSize, double codeRate) {
  // Primary criterion: Transport block size threshold
  if (transportBlockSize > 292) {
    return BaseGraphType::BG1; // Large blocks use BG1
  } else {
    return BaseGraphType::BG2; // Small blocks use BG2
  }

  // Note: In a more sophisticated implementation, you might also consider:
  // - Code rate (BG1 for high rates, BG2 for low rates)
  // - Modulation order
  // - Number of layers
  // - Available resources in the time-frequency grid
}

// ============================================================================
// LDPC ENCODER CLASS
// ============================================================================

/**
 * @class LDPCEncoder
 * @brief Main class for LDPC encoding operations
 *
 * This class handles the complete LDPC encoding process:
 * 1. Parameter calculation and base graph selection
 * 2. Filler bit insertion for padding
 * 3. Systematic encoding using the parity check matrix
 * 4. Rate matching (if needed)
 */
class LDPCEncoder {
private:
  LDPCParameters params; // Encoding parameters

  /**
   * @brief Calculate all LDPC parameters based on input requirements
   *
   * This method determines:
   * - Which base graph to use (BG1 or BG2)
   * - Lifting size Z
   * - Number of information bit columns Kb
   * - Total dimensions of the lifted matrix
   * - Required filler bits
   *
   * @param inputBits Number of input bits (after CRC attachment)
   * @param targetCodeRate Desired code rate
   */
  void calculateParameters(int inputBits, double targetCodeRate) {
    // Step 1: Select base graph based on input size
    params.baseGraph = selectBaseGraph(inputBits, targetCodeRate);

    // Step 2: Determine Kb (number of information columns)
    if (params.baseGraph == BaseGraphType::BG1) {
      params.Kb = BG1_K;            // 22 columns for BG1
      params.numRows = BG1_ROWS;    // 46 rows
      params.numCols = BG1_COLUMNS; // 68 columns
    } else {
      params.Kb = BG2_K;            // 10 columns for BG2
      params.numRows = BG2_ROWS;    // 42 rows
      params.numCols = BG2_COLUMNS; // 52 columns
    }

    // Step 3: Calculate lifting size Z
    // Z must satisfy: Kb × Z ≥ inputBits (to fit all input bits)
    params.liftingSize = calculateLiftingSize(params.Kb, inputBits);

    // Step 4: Calculate dimensions of the lifted LDPC matrix
    params.K = params.Kb * params.liftingSize;      // Total information bits
    params.N = params.numCols * params.liftingSize; // Total codeword length

    // Step 5: Calculate number of filler bits needed
    // Filler bits pad the input to exactly Kb×Z bits
    params.numFillerBits = params.K - inputBits;

    // Step 6: Calculate actual code rate
    params.codeRate = static_cast<double>(params.K) / params.N;

    // Validate that filler bits are non-negative
    if (params.numFillerBits < 0) {
      throw std::runtime_error("Invalid filler bit calculation");
    }
  }

  /**
   * @brief Insert filler bits into the input bit sequence
   *
   * Filler bits are NULL bits that pad the input to the required size.
   * According to 3GPP, filler bits:
   * - Are inserted at the beginning of the information bit sequence
   * - Have value 0 (but marked as NULL internally)
   * - Are NOT transmitted (removed during rate matching)
   * - Ensure the input fits exactly into Kb × Z bits
   *
   * @param inputBits Original input bits
   * @return Padded bit sequence with filler bits
   */
  std::vector<int> insertFillerBits(const std::vector<int> &inputBits) {
    // Create output vector with total size Kb × Z
    std::vector<int> paddedBits(params.K, 0);

    // Insert filler bits at the beginning (set to 0)
    // In production, these might be marked specially as NULL
    for (int i = 0; i < params.numFillerBits; i++) {
      paddedBits[i] = 0; // Filler bit (NULL)
    }

    // Copy actual information bits after the filler bits
    for (size_t i = 0; i < inputBits.size(); i++) {
      paddedBits[params.numFillerBits + i] = inputBits[i];
    }

    return paddedBits;
  }

  /**
   * @brief Perform simplified systematic LDPC encoding
   *
   * This is a simplified version that demonstrates the encoding concept:
   * 1. Copy systematic bits (input) to codeword
   * 2. Generate parity bits using simple XOR operations
   *
   * Note: In production, this would use the actual base graph structure
   * with circulant matrix multiplications.
   *
   * @param inputBits Input bits (after filler bit insertion)
   * @return Complete codeword (input bits + parity bits)
   */
  std::vector<int> systematicEncode(const std::vector<int> &inputBits) {
    int Z = params.liftingSize; // Lifting size
    int Kb = params.Kb;         // Number of information columns
    int M = params.numRows;     // Number of parity check equations

    // Initialize output codeword (systematic part + parity part)
    std::vector<int> codeword(params.N, 0);

    // Step 1: Copy systematic bits (input) to the codeword
    // First K bits are the input bits (systematic part)
    for (int i = 0; i < params.K && i < static_cast<int>(inputBits.size());
         i++) {
      codeword[i] = inputBits[i];
    }

    // Step 2: Generate parity bits using simplified algorithm
    // In production, this would use the actual base graph structure
    // Here we use a simplified approach for demonstration

    // Calculate number of parity bits
    int numParityBits = params.N - params.K;

    // Generate parity bits using XOR of systematic bits
    // This is a simplified approach; actual LDPC uses base graph structure
    for (int i = 0; i < numParityBits; i++) {
      int parityBit = 0; // Initialize parity bit to 0

      // XOR several systematic bits to generate this parity bit
      // Use a simple pattern that varies with position
      for (int j = 0; j < params.K; j += (i % 7 + 1)) {
        parityBit ^= codeword[j]; // XOR operation in GF(2)
      }

      // Store parity bit in codeword
      codeword[params.K + i] = parityBit;
    }

    return codeword;
  }

public:
  /**
   * @brief Constructor for LDPCEncoder
   * Initializes the encoder
   */
  LDPCEncoder() {}

  /**
   * @brief Main encoding function - encodes input bits using LDPC
   *
   * This is the main entry point for LDPC encoding. It orchestrates:
   * 1. Parameter calculation
   * 2. Filler bit insertion
   * 3. Systematic encoding
   *
   * @param inputBits Input bit sequence (after CRC attachment)
   * @param targetCodeRate Target code rate (e.g., 0.33, 0.5, 0.67)
   * @return Encoded codeword (systematic bits + parity bits)
   */
  std::vector<int> encode(const std::vector<int> &inputBits,
                          double targetCodeRate) {
    // Step 1: Calculate all encoding parameters
    std::cout << "\n[LDPC] Calculating encoding parameters..." << std::endl;
    calculateParameters(inputBits.size(), targetCodeRate);

    // Step 2: Insert filler bits to pad input to required size
    std::cout << "[LDPC] Inserting " << params.numFillerBits
              << " filler bits..." << std::endl;
    std::vector<int> paddedBits = insertFillerBits(inputBits);

    // Step 3: Perform systematic LDPC encoding
    std::cout << "[LDPC] Performing systematic encoding..." << std::endl;
    std::vector<int> codeword = systematicEncode(paddedBits);

    std::cout << "[LDPC] Encoding complete!" << std::endl;
    return codeword;
  }

  /**
   * @brief Get current encoding parameters
   * @return LDPCParameters structure with all encoding parameters
   */
  LDPCParameters getParameters() const { return params; }

  /**
   * @brief Print encoding parameters for debugging
   */
  void printParameters() const {
    std::cout << "\n========================================" << std::endl;
    std::cout << "LDPC ENCODING PARAMETERS" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Base Graph:        BG"
              << (params.baseGraph == BaseGraphType::BG1 ? "1" : "2")
              << std::endl;
    std::cout << "Lifting Size (Z):  " << params.liftingSize << std::endl;
    std::cout << "Information cols:  Kb = " << params.Kb << std::endl;
    std::cout << "Matrix rows:       M = " << params.numRows << std::endl;
    std::cout << "Matrix columns:    N_col = " << params.numCols << std::endl;
    std::cout << "Information bits:  K = " << params.K << std::endl;
    std::cout << "Codeword length:   N = " << params.N << std::endl;
    std::cout << "Code rate:         R = " << std::fixed << std::setprecision(4)
              << params.codeRate << std::endl;
    std::cout << "Filler bits:       " << params.numFillerBits << std::endl;
    std::cout << "Parity bits:       " << (params.N - params.K) << std::endl;
    std::cout << "========================================\n" << std::endl;
  }
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Generate random binary sequence for testing
 * @param length Number of bits to generate
 * @return Vector of random bits (0 or 1)
 */
std::vector<int> generateRandomBits(int length) {
  std::vector<int> bits(length);
  // Use current time as seed for randomization
  srand(time(nullptr));

  // Generate random 0 or 1 for each bit
  for (int i = 0; i < length; i++) {
    bits[i] = rand() % 2; // Random value 0 or 1
  }

  return bits;
}

/**
 * @brief Calculate Hamming weight (number of 1s) in bit sequence
 * @param bits Bit sequence
 * @return Number of 1s in the sequence
 */
int calculateHammingWeight(const std::vector<int> &bits) {
  int weight = 0;
  // Count all 1s in the sequence
  for (int bit : bits) {
    if (bit == 1) {
      weight++;
    }
  }
  return weight;
}

// ============================================================================
// MAIN FUNCTION - DEMONSTRATION
// ============================================================================

/**
 * @brief Main function demonstrating LDPC encoding
 *
 * This function demonstrates the complete LDPC encoding process:
 * 1. Generate/input data bits
 * 2. Configure encoding parameters
 * 3. Perform LDPC encoding
 * 4. Display results
 */
int main() {
  try {
    std::cout << "========================================" << std::endl;
    std::cout << "5G NR LDPC ENCODER DEMONSTRATION" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Based on 3GPP TS 38.212 Section 5.3.2" << std::endl;
    std::cout << "Designed By Harsh Prasad" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // ====================================================================
    // STEP 1: Generate input data
    // ====================================================================

    // In real 5G, this would be the transport block after CRC24A attachment
    // For this demo, we'll generate random bits
    int inputSize = 500; // Example: 500 bits after CRC attachment
    std::cout << "Generating " << inputSize << " random input bits..."
              << std::endl;
    std::vector<int> inputBits = generateRandomBits(inputSize);

    // Display first few input bits
    std::cout << "\nInput bits (first 64):" << std::endl;
    for (int i = 0; i < std::min(64, static_cast<int>(inputBits.size())); i++) {
      std::cout << inputBits[i];
      if ((i + 1) % 8 == 0)
        std::cout << " ";
      if ((i + 1) % 64 == 0)
        std::cout << std::endl;
    }
    std::cout << "..." << std::endl;

    // ====================================================================
    // STEP 2: Configure LDPC encoder
    // ====================================================================

    // Create LDPC encoder instance
    LDPCEncoder encoder;

    // Set target code rate
    // Common 5G code rates: 1/3 (0.333), 1/2 (0.5), 2/3 (0.667), 3/4 (0.75)
    double targetCodeRate = 0.5; // Using 1/2 rate for this example

    std::cout << "\nTarget code rate: " << targetCodeRate << std::endl;

    // ====================================================================
    // STEP 3: Perform LDPC encoding
    // ====================================================================

    std::cout << "\nStarting LDPC encoding process..." << std::endl;
    std::cout << "========================================" << std::endl;

    // Encode the input bits
    std::vector<int> encodedBits = encoder.encode(inputBits, targetCodeRate);

    // ====================================================================
    // STEP 4: Display results
    // ====================================================================

    // Print encoding parameters
    encoder.printParameters();

    // Get encoding parameters for analysis
    LDPCParameters params = encoder.getParameters();

    // Display output statistics
    std::cout << "========================================" << std::endl;
    std::cout << "ENCODING RESULTS" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Input bits:        " << inputBits.size() << " bits"
              << std::endl;
    std::cout << "Output bits:       " << encodedBits.size() << " bits"
              << std::endl;
    std::cout << "Parity bits added: " << (encodedBits.size() - params.K)
              << " bits" << std::endl;
    std::cout << "Actual code rate:  " << std::fixed << std::setprecision(4)
              << static_cast<double>(params.K) / encodedBits.size()
              << std::endl;
    std::cout << "Overhead:          " << std::fixed << std::setprecision(2)
              << (static_cast<double>(encodedBits.size() - params.K) /
                  params.K * 100)
              << "%" << std::endl;

    // Display Hamming weights (for information)
    int inputWeight = calculateHammingWeight(inputBits);
    int outputWeight = calculateHammingWeight(encodedBits);
    std::cout << "\nHamming weight (input):  " << inputWeight << " ("
              << std::fixed << std::setprecision(1)
              << (100.0 * inputWeight / inputBits.size()) << "%)" << std::endl;
    std::cout << "Hamming weight (output): " << outputWeight << " ("
              << std::fixed << std::setprecision(1)
              << (100.0 * outputWeight / encodedBits.size()) << "%)"
              << std::endl;

    // Display first few output bits
    std::cout << "\nEncoded bits (first 64):" << std::endl;
    for (int i = 0; i < std::min(64, static_cast<int>(encodedBits.size()));
         i++) {
      std::cout << encodedBits[i];
      if ((i + 1) % 8 == 0)
        std::cout << " ";
      if ((i + 1) % 64 == 0)
        std::cout << std::endl;
    }
    std::cout << "..." << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "LDPC ENCODING DEMONSTRATION COMPLETE!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0; // Successful execution

  } catch (const std::exception &e) {
    // Catch and display any errors that occurred
    std::cerr << "\n❌ ERROR: " << e.what() << std::endl;
    return 1; // Error exit code
  }
}
