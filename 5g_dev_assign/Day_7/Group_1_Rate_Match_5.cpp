// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  TITLE:       5G NR PBCH: RATE MATCHING (POLAR CODE)                    ║
 * ║  AUTHOR:      AI Technical Documentation Generator                      ║
 * ║  DATE:        2023-10-27                                                ║
 * ║  STANDARD:    3GPP TS 38.212 Section 5.4.1                              ║
 * ║                                                                         ║
 * ║  DESCRIPTION:                                                           ║
 * ║  Implements Stage 5 of the PBCH chain: Rate Matching.                   ║
 * ║  Adapts the mother code length (N=512) to the physical resource         ║
 * ║  capacity (E=864) via Sub-block Interleaving and Circular Buffer        ║
 * ║  repetition.                                                            ║
 * ║                                                                         ║
 * ║  ALGORITHM STEPS:                                                       ║
 * ║  1. Sub-block Interleaving: Permute bits using Bit-Reversal pattern.    ║
 * ║     y[bit_reverse(n)] = d[n]                                            ║
 * ║  2. Circular Buffer Loading: buffer = y                                 ║
 * ║  3. Bit Selection: Extract E=864 bits from buffer with wrap-around.     ║
 * ║                                                                         ║
 * ║  INPUT SCENARIO:                                                        ║
 * ║  - Input: 512 Polar coded bits (d)                                      ║
 * ║  - Output: 864 Rate matched bits (f)                                    ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════
#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

// ─── Constants ───
const int RM_N_MOTHER = 512; // Mother Code Length (N)
const int RM_E_OUTPUT = 864; // Rate Matched Output Length (E)
const int RM_LOG2_N = 9;     // log2(512) for bit reversal

using BitVector = std::vector<uint8_t>;

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Reverses the lower 'bit_count' bits of an integer.
 *
 * Example: if bit_count=9, n=1 (000000001) -> returns 256 (100000000)
 *
 * @param n Integer to reverse
 * @param bit_count Number of bits to consider
 * @return uint16_t Bit-reversed value
 */
uint16_t reverse_bits(uint16_t n, int bit_count) {
  uint16_t result = 0;
  for (int i = 0; i < bit_count; i++) {
    if ((n >> i) & 1) {
      result |= (1 << (bit_count - 1 - i));
    }
  }
  return result;
}

/**
 * @brief Prints a sequence of bits with formatting.
 */
void print_bits(const std::string &label, const BitVector &bits,
                size_t limit = 64) {
  std::cout << "  " << std::left << std::setw(20) << label << " │ ";
  for (size_t i = 0; i < bits.size(); ++i) {
    if (i >= limit) {
      std::cout << "... (" << bits.size() - i << " more)";
      break;
    }
    std::cout << (int)bits[i];
    if ((i + 1) % 8 == 0 && i != bits.size() - 1)
      std::cout << " ";
  }
  std::cout << "\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// STAGE 5: RATE MATCHING
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Executes Stage 5: Rate Matching (Interleaving + Repetition)
 *
 * @param d_bits Input 512 encoded bits
 * @return BitVector Output 864 rate-matched bits
 */
BitVector stage5_rate_matching(const BitVector &d_bits) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout << "║  STAGE 5: RATE MATCHING (E=" << RM_E_OUTPUT
            << ")                             ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Standard Reference: TS 38.212 Section 5.4.1\n\n";

  if (d_bits.size() != RM_N_MOTHER) {
    std::cerr << "Error: Input size " << d_bits.size()
              << " != N=" << RM_N_MOTHER << "\n";
    return BitVector();
  }

  // ─── Step 5.1: Sub-block Interleaving ───
  // Logic: y[reverse(n)] = d[n]

  BitVector y_bits(RM_N_MOTHER, 0);

  std::cout << "[STEP 5.1] Sub-block Interleaving (Bit Reversal)\n";
  std::cout
      << "  Permutation Formula: y(J(n)) = d(n) where J(n) = bit_rev(n)\n";
  std::cout << "  Mapping Examples:\n";
  std::cout << "  n (Input Idx) │ J(n) (Output Idx) │ Action\n";
  std::cout
      << "  ──────────────┼───────────────────┼──────────────────────────\n";

  for (int n = 0; n < RM_N_MOTHER; n++) {
    uint16_t J_n = reverse_bits(n, RM_LOG2_N);

    y_bits[J_n] = d_bits[n];

    // Visualize first few and specific interesting cases
    if (n < 4 || n == 255) {
      std::cout << "  " << std::setw(13) << n << " │ " << std::setw(17) << J_n
                << " │ "
                << "y[" << J_n << "] = d[" << n << "] = " << (int)d_bits[n]
                << "\n";
    }
    if (n == 4)
      std::cout << "  ...           │ ...               │ ...\n";
  }
  std::cout
      << "  ──────────────┴───────────────────┴──────────────────────────\n";
  print_bits("Interleaved (y)", y_bits, 32);

  // ─── Step 5.2: Bit Selection (Circular Buffer) ───
  // Logic: f[k] = y[k mod N]

  BitVector f_bits;
  f_bits.reserve(RM_E_OUTPUT);

  std::cout << "\n[STEP 5.2] Bit Selection (Circular Buffer Repetition)\n";
  std::cout << "  Target E = " << RM_E_OUTPUT << ", Source N = " << RM_N_MOTHER
            << "\n";
  std::cout << "  Repetition Factor = " << (double)RM_E_OUTPUT / RM_N_MOTHER
            << "x\n";
  std::cout << "  Logic: f(k) = buffer[k mod " << RM_N_MOTHER << "]\n\n";

  // Perform selection
  for (int k = 0; k < RM_E_OUTPUT; k++) {
    int buffer_idx = k % RM_N_MOTHER;
    f_bits.push_back(y_bits[buffer_idx]);
  }

  // Visualization of the Wrap-Around
  std::cout << "  Transition Analysis (Wrap-around point):\n";
  std::cout << "  k (Out Idx) │ Modulo Idx │ Value Source\n";
  std::cout << "  ────────────┼────────────┼──────────────────────────────\n";
  for (int k = 509; k <= 514; k++) {
    std::cout << "  " << std::setw(11) << k << " │ " << std::setw(10)
              << (k % RM_N_MOTHER) << " │ "
              << "f[" << k << "] = y[" << (k % RM_N_MOTHER) << "]";
    if (k == 511)
      std::cout << "  <-- End of 1st pass";
    if (k == 512)
      std::cout << "  <-- Start of 2nd pass (Wrap)";
    std::cout << "\n";
  }

  std::cout << "\n✓ STAGE 5 COMPLETE: " << f_bits.size()
            << " rate-matched bits generated.\n";
  return f_bits;
}

// ═══════════════════════════════════════════════════════════════════════════
// DEMONSTRATION SCENARIO
// ═══════════════════════════════════════════════════════════════════════════

void run_demonstration() {
  // 1. Generate Simulated Input (d)
  // Create a 512-bit pattern where index is encoded in value or pattern for
  // tracing Let's use a simple pattern: 1 at index 1, 0 elsewhere, to trace the
  // bit reversal of 1->256
  BitVector input_d(RM_N_MOTHER, 0);

  // Set specific bits to 1 to verify interleaving
  input_d[0] = 1;   // Should map to y[0]
  input_d[1] = 1;   // Should map to y[256]
  input_d[511] = 1; // 511 (111111111) rev -> 511. Should map to y[511]

  // Add some random noise for visual density
  for (int i = 10; i < 50; i++)
    input_d[i] = (i % 2);

  std::cout
      << "═══════════════════════════════════════════════════════════════\n";
  std::cout
      << "      5G NR PBCH RATE MATCHING DEMONSTRATION                   \n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  std::cout << "Input Configuration:\n";
  std::cout << "  d[1] = 1 (Expect y[256] = 1)\n";
  std::cout << "  d[511] = 1 (Expect y[511] = 1)\n";

  // Run Stage 5
  BitVector output_f = stage5_rate_matching(input_d);

  // Final Verification
  std::cout
      << "\n═══════════════════════════════════════════════════════════════\n";
  std::cout << "FINAL OUTPUT VERIFICATION\n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  std::cout << "1. Total Output Size: " << output_f.size() << " (Expected "
            << RM_E_OUTPUT << ")\n";

  // Verify specific mappings
  // d[1] -> y[256].
  // f[256] should be y[256] = 1.
  // f[256 + 512] = f[768] should also be 1 (repetition).

  int val_f256 = output_f[256];
  int val_f768 = output_f[768];

  std::cout << "2. Bit Tracing (d[1] -> y[256]):\n";
  std::cout << "   f[256] = " << val_f256 << " "
            << (val_f256 == 1 ? "(Correct)" : "(Error)") << "\n";
  std::cout << "   f[768] = " << val_f768 << " "
            << (val_f768 == 1 ? "(Correct Repetition)" : "(Error)") << "\n";

  std::cout << "3. Repetition Count:\n";
  std::cout << "   " << (RM_E_OUTPUT - RM_N_MOTHER)
            << " bits are repeated from the start of the buffer.\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  try {
    run_demonstration();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
