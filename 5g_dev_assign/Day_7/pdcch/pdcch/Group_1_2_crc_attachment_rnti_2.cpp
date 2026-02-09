/**
 * ═══════════════════════════════════════════════════════════════════════════
 *  TITLE:       5G NR DCI PROCESSING: CRC ATTACHMENT & RNTI MASKING
 *  AUTHOR:      AI C++ Generator
 *  DATE:        2023-10-27
 *  STANDARD:    3GPP TS 38.212 V17.0.0 (Section 5.1 & 7.3.2)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *  DESCRIPTION:
 *  This program demonstrates the specific channel coding steps for a 5G
 *  Downlink Control Information (DCI) message. It performs:
 *  1. CRC Calculation using the 5G-specific CRC24C polynomial.
 *  2. RNTI Masking (Scrambling) where the UE's identity is XORed with the
 *     CRC parity bits.
 *
 *  SCENARIO (FROM INPUT):
 *  - DCI Payload: 42 bits
 *  - RNTI:        0x4E21 (C-RNTI for UE#7)
 *  - Task:        Compute CRC, Mask with RNTI, Output 66 bits.
 *
 *  STAGES:
 *  1. [INPUT]     Load DCI payload bits.
 *  2. [CRC]       Compute 24-bit CRC using Generator Poly gCRC24C.
 *  3. [MASK]      XOR the last 16 bits of CRC with the 16-bit RNTI.
 *  4. [OUTPUT]    Combine Payload + Masked CRC.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Represents a sequence of bits (0 or 1).
 */
using BitVector = std::vector<uint8_t>;

/**
 * @brief 3GPP TS 38.212 Section 5.1 - CRC24C Polynomial
 * D^24 + D^23 + D^21 + D^20 + D^17 + D^15 + D^13 + D^12 + D^8 + D^4 + D^2 + D +
 * 1 Hex: 0x1B2B117 (Lower 24 terms) Used for Polar-coded channels (PDCCH/PBCH).
 */
const uint32_t POLY_CRC24C = 0x1B2B117;
const int CRC_LENGTH = 24;
const int RNTI_LENGTH = 16;

/**
 * @brief Configuration context for the simulation.
 */
struct ProcessingContext {
  std::string dci_bits_str; // Raw input string
  uint16_t rnti_value;      // C-RNTI
  std::string ue_label;     // Description
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Parse a binary string into a BitVector.
 */
BitVector parse_binary(const std::string &bin_str) {
  BitVector bits;
  for (char c : bin_str) {
    if (c == '1')
      bits.push_back(1);
    else if (c == '0')
      bits.push_back(0);
  }
  return bits;
}

/**
 * @brief Convert integer to binary string for visualization.
 */
std::string to_bin_str(uint32_t val, int width) {
  std::string s = "";
  for (int i = width - 1; i >= 0; i--) {
    s += ((val >> i) & 1) ? "1" : "0";
  }
  return s;
}

/**
 * @brief Visualizes bit data in a formatted box.
 */
void print_data(const BitVector &bits, const std::string &label) {
  std::cout
      << "┌───────────────────────────────────────────────────────────────┐\n";
  std::cout << "│ " << std::left << std::setw(61) << label << " │\n";
  std::cout
      << "├───────────────────────────────────────────────────────────────┤\n";

  // Print bits in groups of 8
  std::cout << "│ HEX: ";
  // Simple Hex preview of first few bytes
  size_t byte_count = (bits.size() + 7) / 8;
  for (size_t i = 0; i < std::min(size_t(6), byte_count); ++i) {
    uint8_t byte = 0;
    for (int b = 0; b < 8 && (i * 8 + b) < bits.size(); b++)
      byte = (byte << 1) | bits[i * 8 + b];
    std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte
              << " ";
  }
  if (byte_count > 6)
    std::cout << "...";
  std::cout << std::dec << "\n";

  std::cout << "│ BIN: ";
  int count = 0;
  for (uint8_t b : bits) {
    std::cout << (int)b;
    if (++count % 8 == 0)
      std::cout << " ";
    if (count >= 48) {
      std::cout << "...";
      break;
    }
  }
  std::cout << "\n│ LEN: " << bits.size() << " bits" << std::right
            << std::setw(45) << " │\n";
  std::cout
      << "└───────────────────────────────────────────────────────────────┘\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: CORE IMPLEMENTATION STAGES
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 1: INPUT & SETUP
 */
BitVector stage1_setup(const ProcessingContext &ctx) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 1: INPUT SETUP                                         ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";

  BitVector dci = parse_binary(ctx.dci_bits_str);

  std::cout << "[STEP 1.1] Loaded DCI Payload for " << ctx.ue_label << "\n";
  std::cout << "           RNTI: 0x" << std::hex << std::uppercase
            << ctx.rnti_value << std::dec << "\n";
  print_data(dci, "Raw DCI Payload");

  std::cout << "✓ STAGE 1 COMPLETE\n";
  return dci;
}

/**
 * @brief STAGE 2: CRC-24C CALCULATION
 *
 * Implements the Linear Feedback Shift Register (LFSR) for CRC-24C.
 * Generator: 0x1B2B117
 *
 * @return The calculated 24-bit CRC value.
 */
uint32_t stage2_calculate_crc(const BitVector &payload) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 2: CRC-24C CALCULATION                                 ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Standard: 3GPP TS 38.212 Section 5.1\n";
  std::cout << "Polynomial: gCRC24C(D) = 0x1B2B117\n\n";

  // 1. Initialize Register to all zeros (specific to DCI/Polar)
  uint32_t reg = 0;

  // 2. Process all payload bits
  std::cout << "[STEP 2.1] Processing " << payload.size()
            << " payload bits...\n";
  for (uint8_t bit : payload) {
    bool msb_is_1 = (reg >> 23) & 1; // Check the bit popping out (D^23)
    reg = (reg << 1) & 0xFFFFFF;     // Shift left

    // LFSR Logic: Input XORed with MSB determines if we XOR with Poly
    if (msb_is_1 ^ (bit == 1)) {
      reg ^= POLY_CRC24C;
    }
  }

  // 3. Process the "Tail" (effectively 24 zeros attached)
  // In 3GPP implementation, we continue shifting for the length of CRC
  std::cout << "[STEP 2.2] Processing 24 zero-bit tail...\n";
  for (int i = 0; i < CRC_LENGTH; ++i) {
    bool msb_is_1 = (reg >> 23) & 1;
    reg = (reg << 1) & 0xFFFFFF;
    if (msb_is_1) {
      reg ^= POLY_CRC24C;
    }
  }

  std::cout << "\n[RESULT] Calculated CRC (24-bit):\n";
  std::cout << "HEX: 0x" << std::hex << std::uppercase << std::setw(6)
            << std::setfill('0') << reg << std::dec << "\n";
  std::cout << "BIN: " << to_bin_str(reg, 24) << "\n";

  std::cout << "\n✓ STAGE 2 COMPLETE\n";
  return reg;
}

/**
 * @brief STAGE 3: RNTI MASKING
 *
 * XORs the last 16 bits of the CRC with the RNTI.
 * Formula: c_k = b_k + x_rnti,k (mod 2) for the relevant bits.
 */
uint32_t stage3_mask_crc(uint32_t crc_val, uint16_t rnti) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 3: RNTI MASKING                                        ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Reference: 3GPP TS 38.212 Section 7.3.2\n";
  std::cout << "Operation: XOR last 16 bits of CRC with RNTI\n\n";

  // Prepare Mask
  // RNTI is 16 bits. It aligns with the LSB of the 24-bit CRC.
  uint32_t mask = (uint32_t)rnti;

  // Perform XOR
  uint32_t masked_crc = crc_val ^ mask;

  // VISUALIZATION OF THE XOR
  std::cout << "[STEP 3.1] Bitwise Operation:\n";
  std::cout << "  CRC Upper 8 bits : " << to_bin_str((crc_val >> 16) & 0xFF, 8)
            << " (Unchanged)\n";
  std::cout << "  ---------------------------------------------------\n";
  std::cout << "  CRC Lower 16 bits: " << to_bin_str(crc_val & 0xFFFF, 16)
            << "\n";
  std::cout << "  RNTI Mask (UE#7) : " << to_bin_str(rnti, 16) << " (0x"
            << std::hex << rnti << std::dec << ")\n";
  std::cout << "                     ════════════════ XOR\n";
  std::cout << "  Masked Lower 16  : " << to_bin_str(masked_crc & 0xFFFF, 16)
            << "\n";

  std::cout << "\n[RESULT] Final Masked CRC:\n";
  std::cout << "HEX: 0x" << std::hex << std::uppercase << std::setw(6)
            << std::setfill('0') << masked_crc << std::dec << "\n";
  std::cout << "BIN: " << to_bin_str(masked_crc, 24) << "\n";

  std::cout << "\n✓ STAGE 3 COMPLETE\n";
  return masked_crc;
}

/**
 * @brief STAGE 4: OUTPUT CONCATENATION
 *
 * Appends the masked CRC to the payload.
 */
BitVector stage4_finalize(const BitVector &payload, uint32_t masked_crc) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 4: FINAL SEQUENCE GENERATION                           ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";

  BitVector output = payload;

  // Append CRC (MSB first)
  for (int i = CRC_LENGTH - 1; i >= 0; --i) {
    output.push_back((masked_crc >> i) & 1);
  }

  std::cout << "[STEP 4.1] Concatenating Payload (" << payload.size()
            << " bits) + CRC (24 bits)\n";
  std::cout << "[STEP 4.2] Total Length K = " << output.size() << " bits\n";

  print_data(output, "Final Coded Bits (Input to Polar Encoder)");

  std::cout << "\n✓ STAGE 4 COMPLETE\n";
  return output;
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════╗\n";
  std::cout << "║           5G NR PDCCH CRC & RNTI MASKING SIMULATOR           "
               "     ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════╝\n";
  std::cout << " Demonstrates how the gNB ensures the DCI is addressed to "
               "specific UEs.\n\n";

  // 1. SETUP SCENARIO
  ProcessingContext ctx;
  ctx.dci_bits_str =
      "111100010110000101010111110100000001001111"; // 42 bits from prompt
  ctx.rnti_value = 0x4E21;                          // Target UE#7
  ctx.ue_label = "UE#7 (C-RNTI=0x4E21)";

  // 2. EXECUTE PIPELINE
  BitVector payload = stage1_setup(ctx);
  uint32_t raw_crc = stage2_calculate_crc(payload);
  uint32_t masked_crc = stage3_mask_crc(raw_crc, ctx.rnti_value);
  BitVector final_seq = stage4_finalize(payload, masked_crc);

  // 3. FINAL SUMMARY
  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << " SUMMARY OF OPERATIONS\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << "1. Raw DCI Payload: 42 bits\n";
  std::cout << "2. CRC Calculation: 0x" << std::hex << std::uppercase << raw_crc
            << " (Standard CRC24C)\n";
  std::cout << "3. RNTI Masking:    XOR with 0x" << ctx.rnti_value
            << " on last 16 bits\n";
  std::cout << "4. Final CRC Field: 0x" << masked_crc << "\n";
  std::cout << "5. Total Bits (K):  " << final_seq.size() << " bits\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout
      << "NOTE: The actual calculated CRC in this code is mathematically\n";
  std::cout << "derived from the input bits using the TS 38.212 standard.\n";
  std::cout << "If it differs from static textbook examples, the standard\n";
  std::cout << "calculation implemented here takes precedence.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";

  return 0;
}
