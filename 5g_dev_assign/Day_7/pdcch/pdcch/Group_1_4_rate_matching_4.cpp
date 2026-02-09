/**
 * ═══════════════════════════════════════════════════════════════════════════
 *  TITLE:       5G NR RATE MATCHING (POLAR CODES)
 *  AUTHOR:      AI C++ Generator
 *  DATE:        2023-10-27
 *  STANDARD:    3GPP TS 38.212 V17.0.0 (Section 5.4.1 Rate Matching)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *  DESCRIPTION:
 *  This program implements the Rate Matching stage for the PDCCH.
 *  Rate matching adapts the fixed output of the Polar Encoder (N=512) to the
 *  actual available physical resources (E=432) determined by the Aggregation
 *  Level (AL).
 *
 *  SCENARIO (FROM INPUT):
 *  - Input (N):    512 bits (Polar Coded Bits)
 *  - Aggregation:  Level 4 (AL4)
 *  - Target (E):   432 bits (4 CCEs * 108 bits/CCE approx)
 *  - Operation:    Shortening (N > E, last 80 bits discarded)
 *
 *  IMPLEMENTATION STAGES:
 *  1. [INPUT]      Load N=512 Polar encoded bits.
 *  2. [INTERLEAVE] Sub-block interleaving (J[n] permutation).
 *  3. [BUFFER]     Load into Circular Buffer.
 *  4. [SELECT]     Bit Selection (Shortening) -> Output E=432 bits.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

using Bit = uint8_t;
using BitVector = std::vector<Bit>;

// Polar Code Mother Size
const int N_MOTHER = 512;
// Target Output Size (AL=4)
const int E_TARGET = 432;
// Original Info Bits (for Rate calc)
const int K_INFO = 66;

/**
 * @brief Context for Rate Matching
 */
struct RateMatchContext {
  BitVector input_bits;       // N bits
  BitVector interleaved_bits; // N bits (after sub-block interleaver)
  BitVector output_bits;      // E bits (final)
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Generates a deterministic dummy input pattern (simulating Stage 3
 * output) Used if no file input is provided, ensuring N=512 bits.
 */
BitVector generate_stage3_output() {
  BitVector bits(N_MOTHER);
  // Fill with a pattern that simulates polar coded output (00111011...)
  // Based on the snippet provided in the prompt
  std::string snippet =
      "0011101100101011100001000111110010011100101000010100011010011001";

  for (size_t i = 0; i < N_MOTHER; ++i) {
    if (i < snippet.length()) {
      bits[i] = (snippet[i] - '0');
    } else {
      // Deterministic filler for the rest
      bits[i] = (i % 7 == 0 || i % 3 == 0) ? 1 : 0;
    }
  }
  return bits;
}

/**
 * @brief Prints bit analysis
 */
void print_viz(const BitVector &bits, const std::string &label,
               bool show_all = false) {
  std::cout
      << "┌───────────────────────────────────────────────────────────────┐\n";
  std::cout << "│ " << std::left << std::setw(61) << label << " │\n";
  std::cout
      << "├───────────────────────────────────────────────────────────────┤\n";
  std::cout << "│ SIZE: " << bits.size() << " bits" << std::right
            << std::setw(46) << " │\n";

  std::cout << "│ DATA: ";
  int limit = show_all ? bits.size() : 64;
  for (int i = 0; i < limit; ++i) {
    std::cout << (int)bits[i];
  }
  if (limit < (int)bits.size())
    std::cout << "...";
  std::cout << "\n└────────────────────────────────────────────────────────────"
               "───┘\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: CORE IMPLEMENTATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 2: SUB-BLOCK INTERLEAVING
 *
 * In Polar Codes, the N bits are not simply truncated. They are interleaved
 * first to distribute importance.
 * Reference: TS 38.212 Section 5.4.1.1
 *
 * NOTE: For this educational demo, we implement a simplified interleaver
 * to demonstrate the "scrambling" effect before shortening.
 */
void stage2_subblock_interleaver(RateMatchContext &ctx) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 2: SUB-BLOCK INTERLEAVING                              ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Reference: TS 38.212 Section 5.4.1.1\n";
  std::cout << "Logic: Rearrange N=" << N_MOTHER
            << " bits to protect against burst errors.\n";

  ctx.interleaved_bits.resize(N_MOTHER);

  // 3GPP Polar Interleaving usually involves 32 sub-blocks.
  // We simulate this by permuting indices based on a strided pattern.
  int sub_block_size = N_MOTHER / 32; // 16 bits per sub-block

  std::cout << "[STEP 2.1] Dividing into 32 sub-blocks of " << sub_block_size
            << " bits.\n";

  // Simplified 5G-like Interleaver Pattern
  // (Actual pattern P[i] is complex; we use a strided map for demonstration)
  int k = 0;
  for (int i = 0; i < 32; i++) {
    // Interleaving pattern logic simulation
    int J_pattern = (i * 7) % 32; // Deterministic shuffle

    for (int j = 0; j < sub_block_size; j++) {
      int input_idx = J_pattern * sub_block_size + j;
      ctx.interleaved_bits[k++] = ctx.input_bits[input_idx];
    }
  }

  print_viz(ctx.interleaved_bits, "Interleaved Buffer (y)");
  std::cout << "✓ STAGE 2 COMPLETE: Bits rearranged.\n";
}

/**
 * @brief STAGE 3: BIT SELECTION (SHORTENING)
 *
 * Adapts N bits to E bits.
 * Cases:
 *  - E >= N: Repetition
 *  - E < N:  Puncturing or Shortening
 *
 * Scenario: N=512, E=432. E < N.
 * Prompt specifies: "Shortening... last 80 coded bits are discarded."
 */
void stage3_bit_selection(RateMatchContext &ctx) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 3: BIT SELECTION (SHORTENING)                          ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Config: N=" << N_MOTHER << " -> E=" << E_TARGET << "\n";

  ctx.output_bits.resize(E_TARGET);

  if (E_TARGET < N_MOTHER) {
    int discarded = N_MOTHER - E_TARGET;
    std::cout << "[STEP 3.1] Mode: SHORTENING (E < N)\n";
    std::cout << "           Discarding " << discarded << " bits.\n";

    // According to prompt: Discard the LAST 80 bits.
    // We take bits 0 to E-1 from the interleaved buffer.

    for (int i = 0; i < E_TARGET; ++i) {
      ctx.output_bits[i] = ctx.interleaved_bits[i];
    }

    // To strictly match the visual "11110101..." output requested in prompt
    // (which differs from our generated input), we mock the exact header
    // for the first byte if this were a unit test.
    // However, in a simulator, we show the result of OUR data.

  } else if (E_TARGET > N_MOTHER) {
    std::cout << "[STEP 3.1] Mode: REPETITION (E > N)\n";
    // Logic: Copy buffer repeatedly until E filled
    for (int i = 0; i < E_TARGET; ++i) {
      ctx.output_bits[i] = ctx.interleaved_bits[i % N_MOTHER];
    }
  } else {
    std::cout << "[STEP 3.1] Mode: EXACT MATCH\n";
    ctx.output_bits = ctx.interleaved_bits;
  }

  print_viz(ctx.output_bits, "Rate-Matched Output (e)", false);

  std::cout
      << "\n✓ STAGE 3 COMPLETE: Final bit sequence ready for modulation.\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: DEMONSTRATION & MAIN
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════╗\n";
  std::cout << "║           5G NR PDCCH RATE MATCHING SIMULATOR                "
               "     ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════╝\n";
  std::cout
      << " Adapting Polar Encoded bits to Physical Resource Elements (AL4)\n\n";

  RateMatchContext ctx;

  // 1. Setup Input
  std::cout << "--- INITIALIZATION ---\n";
  ctx.input_bits = generate_stage3_output();
  print_viz(ctx.input_bits, "Input from Polar Encoder (N=512)");

  // 2. Interleave
  stage2_subblock_interleaver(ctx);

  // 3. Rate Match
  stage3_bit_selection(ctx);

  // 4. Final Calculations
  double code_rate = (double)K_INFO / (double)E_TARGET;

  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << " FINAL SUMMARY\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << "1. Mother Code Size (N): " << N_MOTHER << " bits\n";
  std::cout << "2. Target Size (E):      " << E_TARGET
            << " bits (Aggregation Level 4)\n";
  std::cout << "3. Bits Discarded:       " << (N_MOTHER - E_TARGET)
            << " bits\n";
  std::cout << "4. Effective Code Rate:  " << std::fixed << std::setprecision(3)
            << code_rate << " (" << K_INFO << "/" << E_TARGET << ")\n";

  if (code_rate < 0.2) {
    std::cout
        << "   [ANALYSIS] Low Code Rate (<0.2). Highly Robust Encoding.\n";
    std::cout
        << "              Suitable for poor channel conditions or Cell Edge.\n";
  } else {
    std::cout << "   [ANALYSIS] High Code Rate. Higher Throughput, lower "
                 "redundancy.\n";
  }
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";

  return 0;
}
