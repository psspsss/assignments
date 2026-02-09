// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  TITLE:       5G NR PBCH: QPSK MODULATION                               ║
 * ║  AUTHOR:      AI Technical Documentation Generator                      ║
 * ║  DATE:        2023-10-27                                                ║
 * ║  STANDARD:    3GPP TS 38.211 Section 5.1.3 (Table 5.1.3-1)              ║
 * ║                                                                         ║
 * ║  DESCRIPTION:                                                           ║
 * ║  Implements Stage 8 of the PBCH chain: QPSK Modulation.                 ║
 * ║  Maps pairs of scrambled bits to complex-valued modulation symbols.      ║
 * ║                                                                         ║
 * ║  MATHEMATICAL FORMULA:                                                  ║
 * ║  d(i) = (1/√2) * [(1 - 2*b(2i)) + j*(1 - 2*b(2i+1))]                    ║
 * ║                                                                         ║
 * ║  MAPPING SUMMARY:                                                       ║
 * ║  b(2i) b(2i+1) │  I (Real)   │  Q (Imag)   │ Quadrant                   ║
 * ║  ──────────────┼─────────────┼─────────────┼──────────                  ║
 * ║     0     0    │   +0.707    │   +0.707    │    I                       ║
 * ║     0     1    │   +0.707    │   -0.707    │    IV                      ║
 * ║     1     0    │   -0.707    │   +0.707    │    II                      ║
 * ║     1     1    │   -0.707    │   -0.707    │    III                     ║
 * ║                                                                         ║
 * ║  INPUT SCENARIO:                                                        ║
 * ║  - Input: 864 scrambled bits                                            ║
 * ║  - Output: 432 Complex Symbols                                          ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint> // [FIX] Required for uint8_t
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

// ─── Constants ───
const int MOD_M_BIT = 864;               // Total Bits
const int MOD_M_SYMB = MOD_M_BIT / 2;    // Total Symbols (432)
const double INV_SQRT_2 = 0.70710678118; // 1 / sqrt(2)

// ─── Type Definitions ───
using BitVector = std::vector<uint8_t>;
using ComplexSym = std::complex<double>; // Standard complex type (real, imag)

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Prints an ASCII representation of the QPSK Constellation.
 */
void print_ascii_constellation() {
  std::cout << "  Visualizing QPSK Constellation Map:\n\n";
  std::cout << "          Q (Imaginary)\n";
  std::cout << "               ▲\n";
  std::cout << "      10 (II)  │  00 (I)\n";
  std::cout << "    (-.7, +.7) │ (+.7, +.7)\n";
  std::cout << "        ●      │      ●\n";
  std::cout << "               │\n";
  std::cout << "  I ◄──────────┼──────────► (Real)\n";
  std::cout << "               │\n";
  std::cout << "        ●      │      ●\n";
  std::cout << "    (-.7, -.7) │ (+.7, -.7)\n";
  std::cout << "      11 (III) │  01 (IV)\n";
  std::cout << "               ▼\n\n";
}

/**
 * @brief Helper to format complex numbers nicely.
 */
std::string format_complex(const ComplexSym &c) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(4);

  // Real part
  if (c.real() >= 0)
    ss << "+";
  ss << c.real();

  // Imag part
  if (c.imag() >= 0)
    ss << " + j";
  else
    ss << " - j";
  ss << std::abs(c.imag());

  return ss.str();
}

/**
 * @brief Determines the Quadrant based on bit values.
 */
std::string get_quadrant_name(uint8_t b0, uint8_t b1) {
  if (b0 == 0 && b1 == 0)
    return "I   (Top-Right)";
  if (b0 == 0 && b1 == 1)
    return "IV  (Bot-Right)";
  if (b0 == 1 && b1 == 0)
    return "II  (Top-Left) ";
  if (b0 == 1 && b1 == 1)
    return "III (Bot-Left) ";
  return "UNKNOWN";
}

// ═══════════════════════════════════════════════════════════════════════════
// STAGE 8: QPSK MODULATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Executes Stage 8: QPSK Modulation.
 *
 * Maps every 2 bits to 1 complex symbol.
 *
 * @param bits Input scrambled bits (Length 864)
 * @return std::vector<ComplexSym> Output symbols (Length 432)
 */
std::vector<ComplexSym> stage8_qpsk_modulation(const BitVector &bits) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 8: QPSK MODULATION                                    ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Standard Reference: TS 38.211 Table 5.1.3-1\n";
  std::cout << "Input Bits: " << bits.size()
            << " → Output Symbols: " << (bits.size() / 2) << "\n\n";

  if (bits.size() % 2 != 0) {
    std::cerr << "Error: Input bit count must be even for QPSK.\n";
    return {};
  }

  std::vector<ComplexSym> symbols;
  symbols.reserve(bits.size() / 2);

  // ─── Step 8.1: Display Mapping Table ───
  std::cout << "[STEP 8.1] Signal Constellation Mapping\n";
  print_ascii_constellation();

  // ─── Step 8.2: Perform Modulation ───
  std::cout << "[STEP 8.2] Processing Bits into Symbols\n";
  std::cout << "  Formula: d(i) = 1/√2 * [(1-2b0) + j(1-2b1)]\n\n";

  std::cout
      << "  Idx │ Bits  │ I (Real) │ Q (Imag) │ Symbol Value      │ Quadrant\n";
  std::cout << "  "
               "────┼───────┼──────────┼──────────┼───────────────────┼────────"
               "──────\n";

  for (size_t i = 0; i < bits.size(); i += 2) {
    // Extract pair
    uint8_t b0 = bits[i];
    uint8_t b1 = bits[i + 1];

    // Apply Formula
    // 1 - 2*b: maps 0->1, 1->-1
    double re = INV_SQRT_2 * (1.0 - 2.0 * b0);
    double im = INV_SQRT_2 * (1.0 - 2.0 * b1);

    ComplexSym sym(re, im);
    symbols.push_back(sym);

    // Detailed logging for first 8 symbols (as per prompt req)
    if ((i / 2) < 8) {
      std::cout << "  " << std::setw(3) << (i / 2) << " │ " << (int)b0 << ","
                << (int)b1 << "   │ " << std::fixed << std::setprecision(4)
                << (re > 0 ? "+" : "") << re << " │ " << (im > 0 ? "+" : "")
                << im << " │ " << format_complex(sym) << " │ "
                << get_quadrant_name(b0, b1) << "\n";
    }
  }
  std::cout
      << "  ... │ ...   │ ...      │ ...      │ ...               │ ...\n";

  std::cout << "\n✓ STAGE 8 COMPLETE: Generated " << symbols.size()
            << " symbols.\n";
  return symbols;
}

// ═══════════════════════════════════════════════════════════════════════════
// DEMONSTRATION SCENARIO
// ═══════════════════════════════════════════════════════════════════════════

void run_demonstration() {
  // 1. Setup Input: Use the exact sequence from the prompt explanation
  // b_tilde = [1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, ...]
  std::vector<int> prefix_ints = {1, 0, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 0, 1, 0, 0, 1};

  BitVector input_bits;
  input_bits.reserve(MOD_M_BIT);

  // Fill prefix
  for (int b : prefix_ints)
    input_bits.push_back((uint8_t)b);

  // Fill rest with zeros (for valid size)
  while (input_bits.size() < MOD_M_BIT)
    input_bits.push_back(0);

  std::cout
      << "═══════════════════════════════════════════════════════════════\n";
  std::cout
      << "      5G NR PBCH QPSK MODULATION DEMO                          \n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  // Run Stage 8
  std::vector<ComplexSym> output_syms = stage8_qpsk_modulation(input_bits);

  // ─── Verification ───
  std::cout
      << "\n═══════════════════════════════════════════════════════════════\n";
  std::cout << "VERIFICATION AGAINST PROMPT VALUES\n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  // Data from prompt for verification
  struct VerifyData {
    int idx;
    std::string expected_bits;
    double expected_re;
    double expected_im;
  };

  std::vector<VerifyData> checks = {
      {0, "10", -0.7071, 0.7071},  // Quadrant II
      {1, "11", -0.7071, -0.7071}, // Quadrant III
      {7, "01", 0.7071, -0.7071}   // Quadrant IV
  };

  for (const auto &check : checks) {
    if (check.idx >= output_syms.size())
      continue;

    ComplexSym s = output_syms[check.idx];

    std::cout << "Symbol d(" << check.idx << ") [" << check.expected_bits
              << "]:\n";
    std::cout << "  Calculated: " << format_complex(s) << "\n";
    std::cout << "  Expected:   " << (check.expected_re > 0 ? "+" : "")
              << check.expected_re << " " << (check.expected_im > 0 ? "+" : "")
              << check.expected_im << "j\n";

    // Simple tolerance check
    bool re_ok = std::abs(s.real() - check.expected_re) < 0.001;
    bool im_ok = std::abs(s.imag() - check.expected_im) < 0.001;

    if (re_ok && im_ok)
      std::cout << "  STATUS: MATCH ✓\n\n";
    else
      std::cout << "  STATUS: MISMATCH ✗\n\n";
  }

  // ─── Power Check ───
  // QPSK symbols should have unit power (magnitude squared = 1)
  if (!output_syms.empty()) {
    double mag_sq = std::norm(output_syms[0]);
    std::cout << "Unit Power Check (Symbol 0):\n";
    std::cout << "  |d(0)|^2 = (" << output_syms[0].real() << ")^2 + ("
              << output_syms[0].imag() << ")^2\n";
    std::cout << "           = 0.5 + 0.5 = " << mag_sq << "\n";
  }
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
