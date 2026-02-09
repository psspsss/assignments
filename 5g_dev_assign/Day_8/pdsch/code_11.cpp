// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH IMPLEMENTATION: MODULATION (MAPPER)                        ║
 * ║  Reference: 3GPP TS 38.211 Section 5.1                                  ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This code implements the Modulation Mapper. It takes binary bits as input
 * and produces Complex Symbols (I + jQ) as output.
 *
 * SUPPORTED SCHEMES:
 * - QPSK   (2 bits/symbol)
 * - 16QAM  (4 bits/symbol)
 * - 64QAM  (6 bits/symbol)
 *
 * ALGORITHM:
 * Uses the specific bit-to-symbol formulas defined in TS 38.211 to ensure
 * Gray Mapping (adjacent symbols in the constellation differ by only 1 bit).
 *
 * AUTHOR: Technical Documentation Bot
 * DATE:   2026-02-06
 */

#include <cmath>
#include <complex>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// TYPES & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

using BitVector = std::vector<uint8_t>;
using ComplexSymbol = std::complex<double>;
using SymbolVector = std::vector<ComplexSymbol>;

enum class ModulationScheme {
  QPSK,
  QAM16,
  QAM64,
  QAM256 // Not implemented in demo to save space, but follows same logic
};

// Power Normalization Factors (1 / sqrt(average_power))
const double SCALE_QPSK = 1.0 / std::sqrt(2.0);
const double SCALE_16QAM = 1.0 / std::sqrt(10.0);
const double SCALE_64QAM = 1.0 / std::sqrt(42.0);

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: MAPPING LOGIC
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Maps 2 bits to QPSK Symbol
 * Ref: TS 38.211 5.1.2
 */
ComplexSymbol map_qpsk(uint8_t b0, uint8_t b1) {
  // Formula: x = 1/sqrt(2) * [(1-2b0) + j(1-2b1)]
  // b=0 -> +1, b=1 -> -1
  double I = (1.0 - 2.0 * b0) * SCALE_QPSK;
  double Q = (1.0 - 2.0 * b1) * SCALE_QPSK;
  return ComplexSymbol(I, Q);
}

/**
 * @brief Maps 4 bits to 16QAM Symbol
 * Ref: TS 38.211 5.1.3
 */
ComplexSymbol map_16qam(const uint8_t *b) {
  // b[0], b[1], b[2], b[3]

  // Real part depends on even bits (b0, b2)
  // Imag part depends on odd bits (b1, b3)

  // Formula: (1-2b0)[2-(1-2b2)]
  double I = (1.0 - 2.0 * b[0]) * (2.0 - (1.0 - 2.0 * b[2]));
  double Q = (1.0 - 2.0 * b[1]) * (2.0 - (1.0 - 2.0 * b[3]));

  return ComplexSymbol(I * SCALE_16QAM, Q * SCALE_16QAM);
}

/**
 * @brief Maps 6 bits to 64QAM Symbol
 * Ref: TS 38.211 5.1.4
 */
ComplexSymbol map_64qam(const uint8_t *b) {
  // Real part: b0, b2, b4
  // Imag part: b1, b3, b5

  // Formula: (1-2b0) * [4 - (1-2b2)[2 - (1-2b4)]]
  double term_I = (2.0 - (1.0 - 2.0 * b[4])); // Innermost
  term_I = 4.0 - (1.0 - 2.0 * b[2]) * term_I; // Middle
  double I = (1.0 - 2.0 * b[0]) * term_I;     // Sign

  double term_Q = (2.0 - (1.0 - 2.0 * b[5]));
  term_Q = 4.0 - (1.0 - 2.0 * b[3]) * term_Q;
  double Q = (1.0 - 2.0 * b[1]) * term_Q;

  return ComplexSymbol(I * SCALE_64QAM, Q * SCALE_64QAM);
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: MODULATOR ENGINE
// ═══════════════════════════════════════════════════════════════════════════

SymbolVector perform_modulation(const BitVector &bits,
                                ModulationScheme scheme) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE: MODULATION (IQ MAPPING)                               ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";

  SymbolVector symbols;
  int bits_per_symbol = 0;
  std::string name;

  switch (scheme) {
  case ModulationScheme::QPSK:
    bits_per_symbol = 2;
    name = "QPSK";
    break;
  case ModulationScheme::QAM16:
    bits_per_symbol = 4;
    name = "16QAM";
    break;
  case ModulationScheme::QAM64:
    bits_per_symbol = 6;
    name = "64QAM";
    break;
  default:
    return {};
  }

  std::cout << "[INFO] Scheme: " << name << " (" << bits_per_symbol
            << " bits/symbol)\n";
  std::cout << "[INFO] Input:  " << bits.size() << " bits\n";

  if (bits.size() % bits_per_symbol != 0) {
    std::cerr
        << "⚠ ERROR: Input bit count is not a multiple of modulation order!\n";
    return {};
  }

  // Processing Loop
  for (size_t i = 0; i < bits.size(); i += bits_per_symbol) {
    ComplexSymbol sym;
    const uint8_t *b_ptr = &bits[i];

    if (scheme == ModulationScheme::QPSK) {
      sym = map_qpsk(b_ptr[0], b_ptr[1]);
    } else if (scheme == ModulationScheme::QAM16) {
      sym = map_16qam(b_ptr);
    } else if (scheme == ModulationScheme::QAM64) {
      sym = map_64qam(b_ptr);
    }

    symbols.push_back(sym);
  }

  std::cout << "[INFO] Output: " << symbols.size()
            << " Complex Symbols generated.\n";
  return symbols;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: VISUALIZATION
// ═══════════════════════════════════════════════════════════════════════════

void print_symbols(const SymbolVector &syms) {
  std::cout << "\n[SYMBOL PREVIEW]\n";
  std::cout << "Idx |  Real (I)  |  Imag (Q)  | Approx Coord\n";
  std::cout << "----|------------|------------|-------------\n";

  for (size_t i = 0; i < syms.size(); ++i) {
    std::cout << std::setw(3) << i << " | " << std::fixed
              << std::setprecision(4) << std::showpos << syms[i].real() << " | "
              << syms[i].imag() << " | ";

    // Simple quadrant visualization
    if (syms[i].real() > 0 && syms[i].imag() > 0)
      std::cout << "(↗ Top-Right)";
    if (syms[i].real() < 0 && syms[i].imag() > 0)
      std::cout << "(↖ Top-Left) ";
    if (syms[i].real() < 0 && syms[i].imag() < 0)
      std::cout << "(↙ Bot-Left) ";
    if (syms[i].real() > 0 && syms[i].imag() < 0)
      std::cout << "(↘ Bot-Right)";

    std::cout << std::noshowpos << "\n";
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  // 1. Test QPSK (2 bits per symbol)
  // Input: 00 (Top-Right), 01 (Bot-Right), 10 (Top-Left), 11 (Bot-Left)
  // Note: Standard QPSK often maps 0->Positive, 1->Negative.
  // 00 -> (+,+), 11 -> (-,-)
  BitVector bits_qpsk = {0, 0, 0, 1, 1, 0, 1, 1};

  SymbolVector syms_qpsk =
      perform_modulation(bits_qpsk, ModulationScheme::QPSK);
  print_symbols(syms_qpsk);

  // 2. Test 16QAM (4 bits per symbol)
  // We will test 2 symbols:
  // A: 0,0,0,0 -> (+1, +1) unscaled -> (+1/sqrt10, +1/sqrt10) scaled
  // B: 1,1,0,0 -> (-3, -3) unscaled -> (-3/sqrt10, -3/sqrt10) scaled
  // Wait, let's trace:
  // 1,1,0,0: b0=1(Neg), b1=1(Neg), b2=0(High), b3=0(High) -> (-3, -3)
  BitVector bits_16qam = {0, 0, 0, 0, 1, 1, 0, 0};

  SymbolVector syms_16qam =
      perform_modulation(bits_16qam, ModulationScheme::QAM16);
  print_symbols(syms_16qam);

  // 3. Physical Property Check
  // Calculate Average Power of 16QAM symbols (Should be approx 1.0)
  double total_power = 0;
  for (auto &s : syms_16qam) {
    total_power += std::norm(s); // norm() returns mag^2
  }
  std::cout << "\n[VALIDATION] Avg Power: " << (total_power / syms_16qam.size())
            << " (Expected ~1.0)\n";

  return 0;
}
