// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH IMPLEMENTATION: BIT LEVEL SCRAMBLING                       ║
 * ║  Reference: 3GPP TS 38.211 Section 7.3.1.1                              ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This code implements the Scrambling stage. It generates a Gold Sequence
 * (Pseudo-Random Binary Sequence) and XORs it with the input bitstream.
 *
 * KEY PARAMETERS:
 * - RNTI (Radio Network Temporary Identifier): Identifies the User (UE).
 * - Cell ID (Physical Cell Identity): Identifies the base station.
 * - q: Codeword index (usually 0 for single stream).
 *
 * ALGORITHM:
 * 1. Calculate c_init based on RNTI and CellID.
 * 2. Initialize two m-sequences (x1 and x2) of length 31.
 * 3. Fast-forward the sequence by Nc = 1600 samples (as per standard).
 * 4. Generate random bit c(n) and XOR with input b(n).
 *
 * AUTHOR: Technical Documentation Bot
 * DATE:   2026-02-06
 */

#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

using BitVector = std::vector<uint8_t>;

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: GOLD SEQUENCE GENERATOR
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Implements the Length-31 Gold Sequence Generator (TS 38.211 Sec 5.2.1)
 */
class GoldSequenceGenerator {
private:
  // Registers to store the state of m-sequences x1 and x2
  // We use vector for educational clarity (mapping indices to standard),
  // though bit-shifting integers is faster in production.
  uint8_t x1[31];
  uint8_t x2[31];

  const int Nc = 1600; // Standard offset

  /**
   * @brief Advance the registers by one step and return the new bits
   * x1(n+31) = (x1(n+3) + x1(n)) mod 2
   * x2(n+31) = (x2(n+3) + x2(n+2) + x2(n+1) + x2(n)) mod 2
   */
  void advance() {
    // Calculate the "next" bit based on the standard polynomials
    // Note: Array index 0 represents n, index 3 represents n+3, etc.
    uint8_t next_x1 = (x1[3] + x1[0]) % 2;
    uint8_t next_x2 = (x2[3] + x2[2] + x2[1] + x2[0]) % 2;

    // Shift elements to the left (simulating time passing)
    // x[0] disappears, x[1] becomes x[0], ..., new bit becomes x[30]
    for (int i = 0; i < 30; i++) {
      x1[i] = x1[i + 1];
      x2[i] = x2[i + 1];
    }

    // Insert new bits at the end
    x1[30] = next_x1;
    x2[30] = next_x2;
  }

public:
  /**
   * @brief Initialize generator with c_init
   */
  GoldSequenceGenerator(uint32_t c_init) {
    // 1. Initialize x1: x1(0)=1, all others 0
    for (int i = 0; i < 31; i++)
      x1[i] = 0;
    x1[0] = 1;

    // 2. Initialize x2: Based on c_init
    // c_init = sum(x2(i) * 2^i)
    for (int i = 0; i < 31; i++) {
      x2[i] = (c_init >> i) & 1;
    }

    // 3. Skip the first Nc = 1600 bits
    for (int i = 0; i < Nc; i++) {
      advance();
    }
  }

  /**
   * @brief Get the next scrambling bit c(n)
   * c(n) = (x1(n+Nc) + x2(n+Nc)) mod 2
   */
  uint8_t get_next_bit() {
    // The output is the sum of the 0-th index of the current state
    // (Since we have already advanced/shifted 1600 times)
    uint8_t c_n = (x1[0] + x2[0]) % 2;

    // Prepare state for next call
    advance();

    return c_n;
  }
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: SCRAMBLING LOGIC
// ═══════════════════════════════════════════════════════════════════════════

struct ScrambleConfig {
  uint16_t rnti;    // C-RNTI (User ID)
  uint16_t cell_id; // Physical Cell ID (0..1007)
  uint8_t q;        // Codeword index (usually 0)
};

/**
 * @brief Performs the scrambling operation.
 */
BitVector perform_scrambling(const BitVector &input_bits,
                             const ScrambleConfig &cfg) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE: SCRAMBLING (RNTI + CELL ID)                           ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";

  BitVector output_bits;
  output_bits.reserve(input_bits.size());

  // 1. Calculate Initialization Seed (c_init)
  // Formula: c_init = (RNTI * 2^15) + (q * 2^14) + CellID
  uint32_t c_init =
      ((uint32_t)cfg.rnti << 15) + ((uint32_t)cfg.q << 14) + cfg.cell_id;

  std::cout << "[INFO] Parameters:\n";
  std::cout << "  • RNTI:    " << cfg.rnti << " (0x" << std::hex << cfg.rnti
            << std::dec << ")\n";
  std::cout << "  • Cell ID: " << cfg.cell_id << "\n";
  std::cout << "  • c_init:  " << c_init
            << " (binary: " << std::bitset<31>(c_init) << ")\n\n";

  // 2. Initialize Generator
  GoldSequenceGenerator gold_gen(c_init);

  // 3. Process Bits
  std::cout << "[INFO] Processing " << input_bits.size() << " bits...\n";

  for (size_t i = 0; i < input_bits.size(); i++) {
    uint8_t scramble_bit = gold_gen.get_next_bit();
    uint8_t in_bit = input_bits[i];

    // XOR Operation
    uint8_t out_bit = (in_bit + scramble_bit) % 2;

    output_bits.push_back(out_bit);
  }

  return output_bits;
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN (TEST BENCH)
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  // 1. Define Configuration
  ScrambleConfig cfg;
  cfg.rnti = 0x1234; // Example RNTI
  cfg.cell_id = 1;   // Example Cell ID
  cfg.q = 0;         // Codeword 0

  // 2. Mock Input Data (e.g., All Zeros)
  // If input is all zeros, the output reveals the raw Scrambling Sequence.
  BitVector input_data(20, 0);

  // 3. Perform Scrambling
  BitVector scrambled = perform_scrambling(input_data, cfg);

  // 4. Verify/Visualize
  std::cout << "\n[VISUALIZATION]\n";
  std::cout << "Idx | Input | Seq(c) | Output (In XOR c)\n";
  std::cout << "----|-------|--------|------------------\n";

  // Note: Since input is 0, Output must equal Sequence
  for (size_t i = 0; i < 10; i++) { // Show first 10
    std::cout << std::setw(3) << i << " |   " << (int)input_data[i] << "   |   "
              << (int)scrambled[i]
              << "    |        " // Output is same as Seq here
              << (int)scrambled[i] << "\n";
  }

  // 5. Test Inverse Property (Descrambling)
  // Scrambling is symmetric: (A XOR B) XOR B = A
  std::cout << "\n[TEST] Verifying Descrambling...\n";
  BitVector descrambled = perform_scrambling(scrambled, cfg);

  bool integrity = true;
  for (size_t i = 0; i < input_data.size(); i++) {
    if (descrambled[i] != input_data[i])
      integrity = false;
  }

  if (integrity)
    std::cout << "SUCCESS: Descrambled data matches original Input.\n";
  else
    std::cerr << "FAILURE: Descrambling failed.\n";

  return 0;
}
