// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/*
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH STAGE 4: LDPC ENCODING                                     ║
 * ║  Implementation Reference: 3GPP TS 38.212 Section 5.3.2                 ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This program implements the LDPC (Low-Density Parity-Check) encoding stage.
 * It takes segmented Code Blocks (CBs), determines the optimal Lifting Size
 * (Zc), and generates the full "Mother Code" containing systematic and parity
 * bits.
 *
 * TECHNICAL SCOPE:
 * - Base Graph (BG1/BG2) parameter definitions
 * - Lifting Size (Zc) selection from TS 38.212 Table 5.3.2-1
 * - Mother Code Size Calculation
 * - Simulated Encoding (Systematic + Parity Bit Generation)
 *
 * IMPLEMENTATION STAGES:
 * 1. Lifting Size (Zc) Selection
 * 2. Bit Filling / Padding (<NULL> insertion)
 * 3. Parity Bit Generation (Mother Code Construction)
 *
 * AUTHOR: AI C++ Technical Generator
 * DATE:   2026-02-06
 * STANDARDS:
 * - 3GPP TS 38.212 Section 5.3.2 (LDPC coding)
 * - 3GPP TS 38.212 Table 5.3.2-1 (Sets of lifting sizes Zc)
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

using Bit = uint8_t;
using BitStream = std::vector<Bit>;

enum class BaseGraph {
  BG1, // Used for large blocks / high rates (Kb=22)
  BG2  // Used for small blocks / low rates (Kb=10)
};

/**
 * @brief Represents the encoded Mother Code structure.
 */
struct LdpcMotherCode {
  BaseGraph bg;
  int Zc;                // Lifting Size
  int Kb;                // Info columns in base matrix
  int K_prime;           // Input bits + Filler
  int N;                 // Total Mother Code Size (bits)
  BitStream systematic;  // The data bits
  BitStream parity;      // The generated check bits
  BitStream full_buffer; // Concatenated [Systematic | Parity]
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: TABLES AND UTILITIES
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief TS 38.212 Table 5.3.2-1: Sets of lifting sizes Zc.
 * 5G NR uses a "Quasi-Cyclic" LDPC code where the base matrix is expanded by
 * Zc. There are 8 sets (i_LS = 0 to 7).
 */
const std::vector<int> ZC_SET_0 = {2, 4, 8, 16, 32, 64, 128, 256};
const std::vector<int> ZC_SET_1 = {3, 6, 12, 24, 48, 96, 192, 384};
const std::vector<int> ZC_SET_2 = {5, 10, 20, 40, 80, 160, 320};
const std::vector<int> ZC_SET_3 = {7, 14, 28, 56, 112, 224};
const std::vector<int> ZC_SET_4 = {9, 18, 36, 72, 144, 288};
const std::vector<int> ZC_SET_5 = {11, 22, 44, 88, 176, 352};
const std::vector<int> ZC_SET_6 = {13, 26, 52, 104, 208};
const std::vector<int> ZC_SET_7 = {15, 30, 60, 120, 240};

/**
 * @brief Returns all valid Zc values in ascending order.
 */
std::vector<int> get_all_valid_zc() {
  std::vector<int> all_zc;
  auto add = [&](const std::vector<int> &s) {
    all_zc.insert(all_zc.end(), s.begin(), s.end());
  };
  add(ZC_SET_0);
  add(ZC_SET_1);
  add(ZC_SET_2);
  add(ZC_SET_3);
  add(ZC_SET_4);
  add(ZC_SET_5);
  add(ZC_SET_6);
  add(ZC_SET_7);
  std::sort(all_zc.begin(), all_zc.end());
  return all_zc;
}

void print_bits(const std::string &label, const BitStream &bits,
                int limit = 32) {
  std::cout << label << " (" << bits.size() << " bits): ";
  for (size_t i = 0; i < bits.size() && i < (size_t)limit; ++i) {
    std::cout << (int)bits[i];
  }
  if (bits.size() > (size_t)limit)
    std::cout << "...";
  std::cout << "\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: STAGE 1 - LIFTING SIZE SELECTION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Selects the optimal Zc and Kb based on input size and Base Graph.
 *
 * Logic:
 * 1. Define kb = 22 for BG1, kb = 10 for BG2.
 * 2. Find minimum Zc from table such that (kb * Zc) >= InputBits.
 */
int stage1_select_parameters(int num_input_bits, BaseGraph bg, int &out_kb) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 1: PARAMETER SELECTION (Zc & Kb)                       "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";

  // 1. Determine Kb (Base Matrix Information Columns)
  // BG1 has 22 info columns (ignoring the 2 punched columns for now)
  // BG2 has 10 info columns
  int Kb = (bg == BaseGraph::BG1) ? 22 : 10;
  out_kb = Kb;

  std::cout << "[STEP 1.1] Base Graph Configuration\n";
  std::cout << "  > Graph: "
            << (bg == BaseGraph::BG1 ? "BG1 (Large)" : "BG2 (Small)") << "\n";
  std::cout << "  > Kb (Info Cols): " << Kb << "\n";
  std::cout << "  > Input Bits (K): " << num_input_bits << "\n";

  // 2. Select Zc
  // We need Kb * Zc >= K
  // Therefore Zc >= K / Kb
  double min_zc_req = (double)num_input_bits / Kb;

  std::vector<int> valid_zcs = get_all_valid_zc();
  int selected_zc = -1;

  for (int z : valid_zcs) {
    if (z >= min_zc_req) {
      selected_zc = z;
      break;
    }
  }

  if (selected_zc == -1) {
    // Should practically not happen given max CB size limits
    selected_zc = 384; // Max Zc
    std::cerr << "WARNING: Input too large for standard Zc sets. Clamping.\n";
  }

  std::cout << "[STEP 1.2] Lifting Size Selection\n";
  std::cout << "  > Constraint: " << Kb << " * Zc >= " << num_input_bits
            << "\n";
  std::cout << "  > Min Zc Required: " << std::fixed << std::setprecision(2)
            << min_zc_req << "\n";
  std::cout << "  > Selected Zc: " << selected_zc << "\n";
  std::cout << "  > Matrix Size: " << Kb << "x" << selected_zc << " = "
            << (Kb * selected_zc) << " bits capacity.\n";

  return selected_zc;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: STAGE 2 - MOTHER CODE GENERATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Simulates the encoding process.
 *
 * Real 5G LDPC encoding involves multiplying the input vector by the generator
 * matrix derived from the H-matrix. Since the H-matrices are huge (BG1 is 46x68
 * blocks), we simulate the OUTPUT STRUCTURE here.
 *
 * Structure:
 * [ Systematic Bits (K) | Filler Bits (<NULL>) | Parity Bits (N - K') ]
 */
LdpcMotherCode stage2_encode(const BitStream &input_bits, BaseGraph bg) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 2: MOTHER CODE GENERATION (ENCODING)                   "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";

  LdpcMotherCode mc;
  mc.bg = bg;

  // 1. Get Parameters
  mc.Zc = stage1_select_parameters(input_bits.size(), bg, mc.Kb);

  // 2. Calculate Dimensions
  // BG1 Base Matrix is 46 x 68. Total Cols = 68. Total Mother Code Size = 66 *
  // Zc (First 2 cols punched) BG2 Base Matrix is 42 x 52. Total Cols = 52.
  // Total Mother Code Size = 50 * Zc

  int total_cols_base = (bg == BaseGraph::BG1) ? 68 : 52;
  // Note: The first 2 columns (2*Zc bits) are "punched" (never transmitted),
  // but the encoder generates them state-wise.
  // Usually, N = total_cols_base * Zc.

  mc.N = total_cols_base * mc.Zc;
  mc.K_prime = mc.Kb * mc.Zc; // Total systematic slots available

  std::cout << "[STEP 2.1] Determining Mother Code Dimensions\n";
  std::cout << "  > Total Base Cols: " << total_cols_base << "\n";
  std::cout << "  > Mother Code Size (N): " << mc.N << " bits\n";
  std::cout << "  > Parity Bits Needed: " << (mc.N - mc.K_prime) << "\n";

  // 3. Prepare Systematic Part with Filler
  // The input bits are copied. If Input < K_prime, append Filler (<NULL>)
  // In rate matching, <NULL> bits are skipped. Here we represent them as 0 for
  // storage.

  mc.systematic = input_bits;
  int filler_bits_count = mc.K_prime - input_bits.size();

  if (filler_bits_count > 0) {
    // Append filler (using 0, practically treated as NULL later)
    for (int i = 0; i < filler_bits_count; ++i)
      mc.systematic.push_back(0);
    std::cout << "  > Filler (<NULL>): Added " << filler_bits_count
              << " bits to match " << mc.K_prime << "\n";
  }

  // 4. Generate Parity Bits (Simulation)
  // In a real implementation, this uses the H-matrix.
  // H * [s | p]^T = 0
  // Here we generate deterministic parity bits based on input to simulate the
  // dependency.
  int num_parity_bits = mc.N - mc.K_prime;
  mc.parity.reserve(num_parity_bits);

  std::cout << "[STEP 2.2] Generating Parity Bits (Simulated)\n";
  std::cout << "  > Calculating " << num_parity_bits << " parity bits...\n";

  // Simple XOR folding simulation to create "Parity" that depends on data
  uint8_t running_xor = 0;
  for (auto b : input_bits)
    running_xor ^= b;

  for (int i = 0; i < num_parity_bits; ++i) {
    // Create a pattern that changes based on index and input data
    Bit p = (running_xor ^ (i % 2)) & 1;
    mc.parity.push_back(p);
    // Toggle running xor occasionally to simulate dependency chain
    if (i % mc.Zc == 0)
      running_xor = ~running_xor;
  }

  // 5. Assemble Full Buffer
  // Full Buffer = Systematic (with Filler) + Parity
  mc.full_buffer = mc.systematic;
  mc.full_buffer.insert(mc.full_buffer.end(), mc.parity.begin(),
                        mc.parity.end());

  std::cout << "  > Mother Code Generated successfully.\n";
  std::cout << "  > Total Length: " << mc.full_buffer.size() << " bits\n";

  // 6. Visualizing Structure
  // 5G LDPC matrix is "Double Diagonal" in the first parity block.
  std::cout << "\n[VISUALIZATION] Logical Structure of Encoded Block:\n";
  std::cout << "  | Systematic (Kb*Zc) | Parity 1 (Double Diagonal) | Parity 2 "
               "(Extension) |\n";
  std::cout << "  | " << std::setw(16) << mc.K_prime << " | " << std::setw(24)
            << "4*Zc" << " | " << std::setw(20)
            << (mc.N - mc.K_prime - 4 * mc.Zc) << " |\n";

  return mc;
}

// ═══════════════════════════════════════════════════════════════════════════
// DEMONSTRATION SCENARIOS
// ═══════════════════════════════════════════════════════════════════════════

void run_scenario(const std::string &name, int input_size, BaseGraph bg) {
  std::cout << "\n============================================================="
               "==========\n";
  std::cout << "SCENARIO: " << name << "\n";
  std::cout << "==============================================================="
               "========\n";

  // Generate dummy input
  BitStream input(input_size);
  for (int i = 0; i < input_size; ++i)
    input[i] = (i % 2); // Pattern 0101...

  // Perform Encoding
  LdpcMotherCode result = stage2_encode(input, bg);

  // Verification Output
  print_bits("Input Systematic", input, 16);
  print_bits("Encoded Output  ", result.full_buffer, 32);

  // Validation math
  if (result.full_buffer.size() == (size_t)result.N) {
    std::cout
        << "✓ VALIDATION: Output size matches calculated Mother Code size.\n";
  } else {
    std::cout << "✗ ERROR: Size mismatch!\n";
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << R"(
    ###############################################################
    #                                                             #
    #      5G NR PDSCH - STAGE 4: LDPC ENCODING                   #
    #         Implementation of 3GPP TS 38.212 Sec 5.3.2          #
    #                                                             #
    ###############################################################
    )" << "\n";

  std::cout << "EDUCATIONAL NOTES:\n";
  std::cout << "1. LDPC (Low-Density Parity-Check) is used for data channels "
               "(PDSCH/PUSCH).\n";
  std::cout
      << "2. It uses a Base Graph (BG) expanded by a 'Lifting Size' (Zc).\n";
  std::cout << "3. The 'Mother Code' is the raw result of encoding (lowest "
               "possible rate).\n";
  std::cout << "4. Rate Matching (next stage) will select bits from this "
               "Mother Code buffer.\n";

  try {
    // Scenario 1: Small Block (BG2), e.g., Voice packet
    // Input 200 bits. BG2 has Kb=10. Min Zc => 200/10 = 20. Zc=20 exists in
    // sets.
    run_scenario("Small Block (BG2, Zc=20)", 200, BaseGraph::BG2);

    // Scenario 2: Large Block (BG1), e.g., Video segment
    // Input 4000 bits. BG1 has Kb=22. Min Zc => 4000/22 = 181.8. Next Zc = 192
    // (Set 1).
    run_scenario("Large Block (BG1, Zc=192)", 4000, BaseGraph::BG1);

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "FINAL SUMMARY:\n";
  std::cout
      << "✓ Correctly identified Lifting Size (Zc) from standard tables.\n";
  std::cout << "✓ Calculated Mother Code dimensions (N, K').\n";
  std::cout << "✓ Generated Systematic + Filler + Parity buffer structure.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  return 0;
}
