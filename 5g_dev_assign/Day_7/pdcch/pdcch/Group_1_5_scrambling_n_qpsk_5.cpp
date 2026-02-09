/**
 * ═══════════════════════════════════════════════════════════════════════════
 *  TITLE:       5G NR SCRAMBLING & QPSK MODULATION (PDCCH)
 *  AUTHOR:      AI C++ Generator
 *  DATE:        2023-10-27
 *  STANDARD:    3GPP TS 38.211 V17.0.0 (Section 7.3.2.3 & 5.1.3)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *  DESCRIPTION:
 *  This program implements the final physical layer processing steps before
 *  resource mapping:
 *  1. [SCRAMBLING]: XORs the input bits with a cell-specific Gold Sequence.
 *     This ensures interference randomization between neighboring cells.
 *     Init Seed: c_init = (RNTI * 2^16) + n_ID.
 *
 *  2. [MODULATION]: Maps pairs of scrambled bits to Complex QPSK symbols.
 *     Mapping: b(2i),b(2i+1) -> Complex Constellation Point.
 *
 *  SCENARIO (FROM INPUT):
 *  - Input:       432 Rate-Matched bits (E=432)
 *  - RNTI:        0x4E21 (UE#7)
 *  - Cell ID:     0 (Assumed n_ID)
 *  - Output:      216 Complex QPSK Symbols
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <bitset>
#include <cmath>
#include <complex>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

using Bit = uint8_t;
using BitVector = std::vector<Bit>;
using ComplexSym = std::complex<double>;

// 3GPP Gold Sequence Constant (Nc)
const int NC_OFFSET = 1600;

// Scaling factor for QPSK (1/sqrt(2)) to normalize power to 1
const double QPSK_SCALE = 0.70710678118; // 1/sqrt(2)

/**
 * @brief Context for the Scrambling/Modulation stage
 */
struct PhyLayerContext {
  uint16_t rnti;                   // UE Identity
  uint16_t n_id;                   // Physical Cell ID (or scrambling ID)
  BitVector input_bits;            // Rate Matched Bits (E)
  BitVector scrambled_bits;        // b_tilde
  std::vector<ComplexSym> symbols; // d(i)
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Generate dummy input bits (432 bits)
 * Matches the scenario where we have E=432 bits coming from Rate Matching.
 */
BitVector generate_input_bits(int length) {
  BitVector bits(length);
  // Simple deterministic pattern 111101...
  for (int i = 0; i < length; ++i) {
    bits[i] = (i % 5 == 0 || i % 2 == 0) ? 1 : 0;
  }
  return bits;
}

/**
 * @brief Visualizes bit buffers
 */
void print_bits(const BitVector &bits, const std::string &label,
                int limit = 64) {
  std::cout
      << "┌───────────────────────────────────────────────────────────────┐\n";
  std::cout << "│ " << std::left << std::setw(61) << label << " │\n";
  std::cout
      << "├───────────────────────────────────────────────────────────────┤\n";
  std::cout << "│ ";
  for (int i = 0; i < limit && i < (int)bits.size(); ++i) {
    std::cout << (int)bits[i];
  }
  if ((int)bits.size() > limit)
    std::cout << "...";
  std::cout << "\n└────────────────────────────────────────────────────────────"
               "───┘\n";
}

/**
 * @brief Visualizes Complex Symbols
 */
void print_symbols(const std::vector<ComplexSym> &syms, int limit = 8) {
  std::cout
      << "┌───────────────────────────────────────────────────────────────┐\n";
  std::cout
      << "│ QPSK SYMBOLS (I/Q Plane)                                      │\n";
  std::cout
      << "├───────────────────────────────────────────────────────────────┤\n";
  std::cout
      << "│ idx |  Real (I) |  Imag (Q) | Interpretation                  │\n";
  std::cout
      << "├─────┼───────────┼───────────┼─────────────────────────────────┤\n";

  for (int i = 0; i < limit && i < (int)syms.size(); ++i) {
    double re = syms[i].real();
    double im = syms[i].imag();

    std::string interpretation;
    if (re > 0 && im > 0)
      interpretation = "00 (Top Right)";
    else if (re > 0 && im < 0)
      interpretation = "01 (Bot Right)";
    else if (re < 0 && im > 0)
      interpretation = "10 (Top Left)";
    else
      interpretation = "11 (Bot Left)";

    std::cout << "│ " << std::setw(3) << i << " | " << std::fixed
              << std::setprecision(4) << std::setw(9) << re << " | "
              << std::setw(9) << im << " | " << std::left << std::setw(31)
              << interpretation << " │\n";
  }
  std::cout
      << "└─────┴───────────┴───────────┴─────────────────────────────────┘\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: CORE IMPLEMENTATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 1: PSEUDO-RANDOM SEQUENCE GENERATION (GOLD SEQUENCE)
 *
 * Implements TS 38.211 Section 5.2.1.
 * Generates c(n) sequence based on c_init.
 */
class GoldSequenceGenerator {
private:
  uint32_t x1[1600 + 432 + 31]; // Buffer for x1 sequence
  uint32_t x2[1600 + 432 + 31]; // Buffer for x2 sequence

public:
  std::vector<uint8_t> generate(uint32_t c_init, int length) {
    std::vector<uint8_t> c(length);

    // 1. Initialization
    // x1(0)=1, x1(n)=0 for n=1..30
    for (int i = 0; i < 31; i++)
      x1[i] = (i == 0) ? 1 : 0;

    // x2 initialized by c_init
    // x2(i) = (c_init >> i) & 1
    for (int i = 0; i < 31; i++)
      x2[i] = (c_init >> i) & 1;

    // 2. Evolution (Linear Feedback Shift Register)
    // We need to evolve enough steps for Nc + length
    int total_steps = NC_OFFSET + length;

    for (int n = 0; n < total_steps; ++n) {
      // x1(n+31) = (x1(n+3) + x1(n)) mod 2
      x1[n + 31] = (x1[n + 3] + x1[n]) % 2;

      // x2(n+31) = (x2(n+3) + x2(n+2) + x2(n+1) + x2(n)) mod 2
      x2[n + 31] = (x2[n + 3] + x2[n + 2] + x2[n + 1] + x2[n]) % 2;
    }

    // 3. Output Generation
    // c(n) = (x1(n+Nc) + x2(n+Nc)) mod 2
    for (int n = 0; n < length; ++n) {
      c[n] = (x1[n + NC_OFFSET] + x2[n + NC_OFFSET]) % 2;
    }

    return c;
  }
};

/**
 * @brief STAGE 2: SCRAMBLING
 *
 * XORs input bits with the Gold Sequence.
 * b_tilde(i) = (b(i) + c(i)) mod 2
 */
void stage2_scrambling(PhyLayerContext &ctx) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 1: SCRAMBLING (INTERFERENCE RANDOMIZATION)             ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Reference: TS 38.211 Section 7.3.2.3\n";

  // 1. Calculate Initialization Seed (c_init)
  // For PDCCH: c_init = (n_RNTI * 2^16) + n_ID
  uint32_t c_init = ((uint32_t)ctx.rnti << 16) + ctx.n_id;

  std::cout << "[STEP 1.1] Calculating Seed c_init:\n";
  std::cout << "  RNTI: 0x" << std::hex << ctx.rnti << "\n";
  std::cout << "  n_ID: " << std::dec << ctx.n_id << "\n";
  std::cout << "  c_init = (" << std::hex << ctx.rnti << " << 16) + "
            << std::dec << ctx.n_id << " = " << std::dec << c_init << " (0x"
            << std::hex << c_init << ")\n"
            << std::dec;

  // 2. Generate Gold Sequence
  GoldSequenceGenerator gen;
  std::vector<uint8_t> c_seq = gen.generate(c_init, ctx.input_bits.size());

  // 3. Perform XOR
  ctx.scrambled_bits.resize(ctx.input_bits.size());
  for (size_t i = 0; i < ctx.input_bits.size(); ++i) {
    ctx.scrambled_bits[i] = (ctx.input_bits[i] + c_seq[i]) % 2;
  }

  std::cout << "[STEP 1.2] Scrambling " << ctx.input_bits.size()
            << " bits...\n";
  print_bits(ctx.input_bits, "Input Bits (Rate Matched)");
  print_bits(c_seq, "Gold Sequence c(n)");
  print_bits(ctx.scrambled_bits, "Scrambled Output (b_tilde)");

  std::cout << "✓ STAGE 1 COMPLETE\n";
}

/**
 * @brief STAGE 3: QPSK MODULATION
 *
 * Maps bit pairs to complex symbols.
 * 00 -> (+1+j)/sqrt(2)
 * 01 -> (+1-j)/sqrt(2)
 * 10 -> (-1+j)/sqrt(2)
 * 11 -> (-1-j)/sqrt(2)
 */
void stage3_modulation(PhyLayerContext &ctx) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 2: QPSK MODULATION                                     ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Reference: TS 38.211 Section 5.1.3\n";

  size_t num_symbols = ctx.scrambled_bits.size() / 2;
  ctx.symbols.resize(num_symbols);

  std::cout << "[STEP 2.1] Mapping " << ctx.scrambled_bits.size() << " bits to "
            << num_symbols << " symbols.\n";

  for (size_t i = 0; i < num_symbols; ++i) {
    // Take 2 bits
    Bit b0 = ctx.scrambled_bits[2 * i];
    Bit b1 = ctx.scrambled_bits[2 * i + 1];

    double re, im;

    // Standard TS 38.211 QPSK Mapping
    // 0 -> +1/sqrt(2), 1 -> -1/sqrt(2)
    // Real part depends on even bit (b0), Imaginary on odd bit (b1)

    // b(2i)   -> I: 0 -> >0, 1 -> <0
    // b(2i+1) -> Q: 0 -> >0, 1 -> <0

    re = (b0 == 0) ? QPSK_SCALE : -QPSK_SCALE;
    im = (b1 == 0) ? QPSK_SCALE : -QPSK_SCALE;

    ctx.symbols[i] = ComplexSym(re, im);
  }

  print_symbols(ctx.symbols, 10);

  std::cout << "✓ STAGE 2 COMPLETE\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════╗\n";
  std::cout << "║           5G NR SCRAMBLING & QPSK MODULATION                 "
               "     ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════╝\n";
  std::cout << " Preparing Rate-Matched Bits for Physical Transmission\n\n";

  // 1. Setup Context
  PhyLayerContext ctx;
  ctx.rnti = 0x4E21;                         // From Prompt
  ctx.n_id = 0;                              // Assumed Cell ID
  ctx.input_bits = generate_input_bits(432); // E=432

  // 2. Run Scrambling
  // (Randomizes data so long strings of 0s or 1s don't cause spectral spikes)
  stage2_scrambling(ctx);

  // 3. Run Modulation
  // (Converts bits to phase/amplitude changes on the carrier)
  stage3_modulation(ctx);

  // 4. Final Summary
  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << " KEY TAKEAWAYS\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << "1. Scrambling Identity: The UE only successfully descrambles "
               "if it uses\n";
  std::cout << "   the same RNTI (" << std::hex << ctx.rnti
            << ") and Cell ID.\n"
            << std::dec;
  std::cout << "2. Constellation: 5G PDCCH uses QPSK for robustness (low order "
               "modulation).\n";
  std::cout << "   Each symbol carries 2 coded bits.\n";
  std::cout << "3. Next Step: These 216 symbols are mapped to Resource "
               "Elements (REs)\n";
  std::cout << "   on the time-frequency grid (CCE-to-REG mapping).\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";

  return 0;
}
