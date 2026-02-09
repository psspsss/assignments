// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH IMPLEMENTATION: CODE BLOCK CONCATENATION                   ║
 * ║  Reference: 3GPP TS 38.212 Section 5.5                                  ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This code implements the Code Block Concatenation stage.
 * After individual Code Blocks (CBs) have been encoded and rate-matched,
 * they must be serialized into a single bitstream (the Codeword) to be
 * passed to the Scrambler and Modulator.
 *
 * INPUT:  A vector of Rate-Matched Code Blocks (r_0, r_1, ... r_C-1)
 * OUTPUT: A single bit sequence (g_0, g_1, ... g_G-1)
 *
 * AUTHOR: Technical Documentation Bot
 * DATE:   2026-02-06
 */

#include <cstdint> // Required for uint8_t
#include <iomanip>
#include <iostream>
#include <numeric> // For std::accumulate
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// TYPES
// ═══════════════════════════════════════════════════════════════════════════

using BitVector = std::vector<uint8_t>;
using CodeBlockList = std::vector<BitVector>;

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: CONCATENATION LOGIC
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Concatenates a list of Code Blocks into a single bitstream.
 *
 * Reference: TS 38.212 Section 5.5
 * Logic: The bits are appended sequentially. CB0, then CB1, then CB2...
 *
 * @param code_blocks Vector containing the individual code blocks.
 * @return BitVector The single concatenated bitstream.
 */
BitVector perform_code_block_concatenation(const CodeBlockList &code_blocks) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE: CODE BLOCK CONCATENATION (REASSEMBLY)                 ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";

  if (code_blocks.empty()) {
    std::cerr << "⚠ Warning: No code blocks to concatenate.\n";
    return {};
  }

  // 1. Calculate Total Size to reserve memory (Efficiency)
  size_t total_bits = 0;
  for (const auto &cb : code_blocks) {
    total_bits += cb.size();
  }

  BitVector concatenated_output;
  concatenated_output.reserve(total_bits);

  std::cout << "[INFO] Input: " << code_blocks.size() << " Code Blocks.\n";
  std::cout << "[INFO] Total Output Bits Expected: " << total_bits << "\n\n";

  // 2. Perform Concatenation
  int cb_idx = 0;
  for (const auto &cb : code_blocks) {
    // Log details
    std::cout << "  -> Appending CB #" << cb_idx << " (Size: " << cb.size()
              << " bits)... ";

    // Insert at end of output vector
    concatenated_output.insert(concatenated_output.end(), cb.begin(), cb.end());

    std::cout << "Done.\n";
    cb_idx++;
  }

  return concatenated_output;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: VISUALIZATION UTILITIES
// ═══════════════════════════════════════════════════════════════════════════

void print_bitstream_summary(const BitVector &bits, int bits_per_row = 50) {
  std::cout << "\n[VISUAL CHECK] Final Stream (First " << bits_per_row
            << " bits):\n";
  std::cout << "------------------------------------------------------------\n";

  for (size_t i = 0; i < bits.size() && i < (size_t)bits_per_row; ++i) {
    std::cout << (int)bits[i];
  }

  if (bits.size() > (size_t)bits_per_row) {
    std::cout << "... (truncated)";
  }
  std::cout
      << "\n------------------------------------------------------------\n";
  std::cout << "Total Final Size: " << bits.size() << " bits.\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  // 1. Setup Mock Data (Simulating Rate-Matched Code Blocks)
  // We will create 3 Code Blocks with distinct patterns to prove order is
  // preserved.

  // CB 0: All 1s (Length 10)
  BitVector cb0(10, 1);

  // CB 1: All 0s (Length 15)
  BitVector cb1(15, 0);

  // CB 2: Alternating 1/0 (Length 10)
  BitVector cb2 = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0};

  CodeBlockList r_blocks = {cb0, cb1, cb2};

  // 2. Run Concatenation
  BitVector g_sequence = perform_code_block_concatenation(r_blocks);

  // 3. Verify Results
  print_bitstream_summary(g_sequence);

  // 4. Logical Validation
  size_t expected = cb0.size() + cb1.size() + cb2.size();
  if (g_sequence.size() == expected) {
    std::cout << "\nSUCCESS: Output size matches sum of inputs.\n";
  } else {
    std::cerr << "\nERROR: Size mismatch!\n";
  }

  return 0;
}
