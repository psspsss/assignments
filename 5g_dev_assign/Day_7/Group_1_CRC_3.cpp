// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  TITLE:       5G NR PBCH: CRC24C CALCULATION & ATTACHMENT               ║
 * ║  AUTHOR:      AI Technical Documentation Generator                      ║
 * ║  DATE:        2023-10-27                                                ║
 * ║  STANDARD:    3GPP TS 38.212 Section 7.1.3                              ║
 * ║                                                                         ║
 * ║  DESCRIPTION:                                                           ║
 * ║  Implements Stage 3 of the PBCH chain: Cyclic Redundancy Check (CRC).   ║
 * ║  Uses the CRC24C polynomial specified for 5G NR Polar Codes.            ║
 * ║  Calculates the 24-bit checksum for the 32-bit scrambled payload and    ║
 * ║  appends it to form the 56-bit coded block.                             ║
 * ║                                                                         ║
 * ║  ALGORITHM:                                                             ║
 * ║  1. Generator Polynomial g_CRC24C(D) = [D24...D0]                       ║
 * ║     Hex: 0x1B2B117                                                      ║
 * ║  2. Append L=24 zeros to input sequence.                                ║
 * ║  3. Perform Modulo-2 Division (XOR division).                           ║
 * ║  4. Remainder is the CRC.                                               ║
 * ║                                                                         ║
 * ║  INPUT SCENARIO:                                                        ║
 * ║  - Payload (32 bits): 1 0 1 1 1 1 1 0 ...                               ║
 * ║  - Output Target:     1 0 0 1 1 0 1 1 ... (CRC part)                    ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════
#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

// ─── Constants ───
const int PHY_PAYLOAD_LEN = 32; // A
const int PHY_CRC_LEN = 24;     // L
const int PHY_TOTAL_LEN = 56;   // B = A + L

// ─── CRC24C Polynomial ───
// D^24 + D^23 + D^21 + D^20 + D^17 + D^15 + D^13 + D^12 + D^8 + D^4 + D^2 + D^1
// + 1 Binary: 1 1011 0010 1011 0001 0001 0111 Hex:    1   B    2    B    1    1
// 7
const uint32_t POLY_CRC24C = 0x1B2B117;

using BitVector = std::vector<uint8_t>;

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Prints a sequence of bits with formatting.
 *
 * @param label Label string
 * @param bits Vector of bits (0 or 1)
 * @param highlight_start Index to start highlighting (optional)
 * @param highlight_len Length of highlight
 */
void print_bits(const std::string &label, const BitVector &bits,
                int highlight_start = -1, int highlight_len = 0) {
  std::cout << "  " << std::left << std::setw(20) << label << " │ ";
  for (size_t i = 0; i < bits.size(); ++i) {
    if (highlight_start != -1 && i == (size_t)highlight_start)
      std::cout << "[";

    std::cout << (int)bits[i];

    if (highlight_start != -1 &&
        i == (size_t)(highlight_start + highlight_len - 1))
      std::cout << "]";
    else if ((i + 1) % 8 == 0 && i != bits.size() - 1)
      std::cout << " ";

    // Truncate for display if very long, but show full CRC
    if (bits.size() > 64 && i == 31) {
      std::cout << "... ";
      i = bits.size() - 25; // Skip to CRC
    }
  }
  std::cout << "\n";
}

/**
 * @brief Helper to convert string "1011..." to BitVector
 */
BitVector string_to_bits(const std::string &s) {
  BitVector v;
  for (char c : s) {
    if (c == '0')
      v.push_back(0);
    else if (c == '1')
      v.push_back(1);
  }
  return v;
}

// ═══════════════════════════════════════════════════════════════════════════
// STAGE 3: CRC CALCULATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Computes CRC24C and attaches it to the payload.
 *
 * Algorithm Steps:
 * 1. Define Generator Polynomial vector.
 * 2. Create dividend = [Payload, 24 zeros].
 * 3. Loop through bits. If bit is 1, XOR window with Poly.
 * 4. Remainder is the last 24 bits.
 *
 * @param input_payload 32-bit scrambled MIB
 * @return 56-bit vector [Payload, CRC]
 */
BitVector stage3_crc_calculation(const BitVector &input_payload) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 3: CRC24C CALCULATION                                 ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Standard Reference: TS 38.212 Section 7.1.3\n";
  std::cout << "Polynomial: D^24 + D^23 + ... + 1 (0x1B2B117)\n\n";

  // 1. Setup Polynomial Vector (MSB to LSB)
  BitVector poly_vec;
  for (int i = 24; i >= 0; --i) {
    poly_vec.push_back((POLY_CRC24C >> i) & 1);
  }
  std::cout << "[STEP 3.1] Generator Polynomial g(x):\n";
  print_bits("Poly (25 bits)", poly_vec);

  // 2. Prepare Dividend (Append Zeros)
  BitVector dividend = input_payload;
  for (int i = 0; i < PHY_CRC_LEN; ++i) {
    dividend.push_back(0);
  }
  std::cout << "\n[STEP 3.2] Appending " << PHY_CRC_LEN << " zeros:\n";
  print_bits("Dividend (56 bits)", dividend);

  // 3. Modulo-2 Division (Bitwise XOR)
  // We process only the first A (32) bits.
  // If a bit is 1, we XOR the polynomial starting at that position.
  // The "remainder" will naturally form in the last 24 bits.

  std::cout
      << "\n[STEP 3.3] Performing Modulo-2 Division (Trace first 3 steps):\n";
  BitVector current = dividend; // Working copy

  int trace_count = 0;
  for (size_t i = 0; i < input_payload.size(); ++i) {
    // If the MSB of the current window is 1, we must divide (XOR)
    if (current[i] == 1) {

      // Visualization for first few operations
      if (trace_count < 3) {
        std::cout << "  Bit " << std::setw(2) << i
                  << " is 1 -> XOR Poly at this position\n";
        // Show before state snippet
        std::cout << "    Window: ";
        for (size_t k = 0; k < 25; k++)
          std::cout << (int)current[i + k];
        std::cout << "\n    Poly:   ";
        for (size_t k = 0; k < 25; k++)
          std::cout << (int)poly_vec[k];
        std::cout << "\n    ───────\n";
      }

      // Perform XOR
      for (size_t j = 0; j < poly_vec.size(); ++j) {
        current[i + j] = current[i + j] ^ poly_vec[j];
      }

      if (trace_count < 3) {
        std::cout << "    Result: ";
        for (size_t k = 0; k < 25; k++)
          std::cout << (int)current[i + k];
        std::cout << "\n\n";
        trace_count++;
      }
    }
  }

  // 4. Extract CRC
  BitVector crc;
  crc.reserve(PHY_CRC_LEN);
  for (size_t i = input_payload.size(); i < current.size(); ++i) {
    crc.push_back(current[i]);
  }

  std::cout << "[STEP 3.4] Division Complete. Extracting Remainder:\n";
  print_bits("CRC Remainder", crc);

  // 5. Assemble Output
  BitVector output = input_payload;
  output.insert(output.end(), crc.begin(), crc.end());

  std::cout << "\n✓ STAGE 3 COMPLETE: " << output.size()
            << " bits generated.\n";
  return output;
}

// ═══════════════════════════════════════════════════════════════════════════
// DEMONSTRATION SCENARIO
// ═══════════════════════════════════════════════════════════════════════════

void run_demonstration() {
  // Construct the specific input scrambled payload described in the prompt
  // First 8 bits: 1 0 1 1 1 1 1 0
  // Remaining 24 bits: Arbitrary filler to make 32 bits (using alternating
  // pattern for demo)

  std::string input_str_prefix = "10111110";
  BitVector payload = string_to_bits(input_str_prefix);

  // Fill remaining 24 bits with pattern 00110011... to reach 32 bits
  // Note: The specific output CRC in the prompt ("10011011...") corresponds to
  // a specific full 32-bit input. Since the prompt only gave the first 8 bits
  // of input, we will use the first 8 bits, pad with zeros, and calculate the
  // REAL CRC for that. If exact matching of the prompt's output is required, we
  // would need the full 32-bit input. Assuming padding with zeros for the
  // remaining input bits for this demo.
  while (payload.size() < PHY_PAYLOAD_LEN) {
    payload.push_back(0);
  }

  std::cout
      << "═══════════════════════════════════════════════════════════════\n";
  std::cout
      << "      5G NR PBCH CRC DEMONSTRATION                             \n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  print_bits("Input Payload (ā)", payload);

  // Run Stage 3
  BitVector coded_block = stage3_crc_calculation(payload);

  // Final Verification Display
  std::cout
      << "\n═══════════════════════════════════════════════════════════════\n";
  std::cout << "FINAL OUTPUT STRUCTURE (B = 56 bits)\n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  // Split for display
  BitVector out_payload(coded_block.begin(), coded_block.begin() + 32);
  BitVector out_crc(coded_block.begin() + 32, coded_block.end());

  std::cout << "Section 1: Scrambled Payload (32 bits)\n";
  print_bits("b0...b31", out_payload);

  std::cout << "\nSection 2: CRC24C Parity Bits (24 bits)\n";
  print_bits("b32...b55", out_crc);

  // Check against prompt output snippet "10011011..."
  // Note: This check only passes if the hidden 24 bits of input match the
  // hidden 24 bits that generated the prompt's example. Since we don't have
  // those, we just show the calculation.
  std::cout << "\nVerification Note: The calculated CRC depends on all 32 "
               "input bits.\n";
  std::cout << "This code implements the exact g_CRC24C(D) algorithm.\n";
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
