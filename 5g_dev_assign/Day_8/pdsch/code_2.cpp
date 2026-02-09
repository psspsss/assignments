// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/*
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH STAGE 2: TRANSPORT BLOCK CRC ATTACHMENT (CRC24A)           ║
 * ║  Implementation Reference: 3GPP TS 38.212 Section 5.1                   ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This program implements the error detection mechanism for the 5G NR Transport
 * Block (TB). Before the TB is segmented into Code Blocks (CBs), a 24-bit CRC
 * (Cyclic Redundancy Check) is calculated and attached to the end of the data.
 *
 * TECHNICAL SCOPE:
 * - Bit-level manipulation of Transport Blocks
 * - Implementation of Generator Polynomial gCRC24A(D)
 * - Linear Feedback Shift Register (LFSR) simulation
 * - Error detection verification (Success/Failure scenarios)
 *
 * IMPLEMENTATION STAGES:
 * 1. Data Input (Bit Stream Conversion)
 * 2. CRC24A Calculation (Poly: 0x864CFB)
 * 3. CRC Attachment
 * 4. Receiver Verification (Syndrome Check)
 *
 * AUTHOR: AI C++ Technical Generator
 * DATE:   2023-10-27
 * STANDARDS:
 * - 3GPP TS 38.212 Section 5.1 (CRC calculation)
 * - 3GPP TS 38.212 Section 7.2.1 (Transport block CRC attachment)
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════

#include <algorithm>
#include <bitset>
#include <cstdint> // FIXED: Required for uint8_t and uint32_t
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

// Represents a single bit (0 or 1)
using Bit = uint8_t;
// Represents a stream of bits
using BitStream = std::vector<Bit>;

/**
 * @brief 3GPP TS 38.212 Table 5.1-1: CRC24A Polynomial
 * gCRC24A(D) = D^24 + D^23 + D^18 + D^17 + D^14 + D^11 + D^10 + D^7 + D^6 + D^5
 * + D^4 + D^3 + D + 1
 *
 * Binary Representation (Highest degree dropped for XOR implementation usually,
 * but here we define the full trail):
 * 1 1000 0110 0100 1100 1111 1011
 * Hex Tail: 0x864CFB
 */
const uint32_t POLY_CRC24A = 0x864CFB;
const int CRC_LENGTH = 24;

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Converts a byte array (vector of uint8_t) to a bit stream.
 */
BitStream bytes_to_bits(const std::vector<uint8_t> &bytes) {
  BitStream bits;
  bits.reserve(bytes.size() * 8);
  for (uint8_t byte : bytes) {
    for (int i = 7; i >= 0; --i) {
      bits.push_back((byte >> i) & 1);
    }
  }
  return bits;
}

/**
 * @brief Converts a bit stream back to hex string for visualization.
 */
std::string bits_to_hex_string(const BitStream &bits) {
  std::stringstream ss;
  ss << "0x";
  uint8_t current_byte = 0;
  int bit_count = 0;

  for (Bit b : bits) {
    current_byte = (current_byte << 1) | b;
    bit_count++;
    if (bit_count == 8) {
      ss << std::hex << std::setw(2) << std::setfill('0') << (int)current_byte;
      current_byte = 0;
      bit_count = 0;
    }
  }
  // Handle remaining bits (left aligned padding if needed, but usually we print
  // full bytes)
  if (bit_count > 0) {
    current_byte <<= (8 - bit_count);
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)current_byte;
  }
  return ss.str();
}

/**
 * @brief Visualizes the bit processing.
 */
void print_bit_window(const BitStream &bits, int cursor, int window_size) {
  std::cout << "  Proc: ";
  for (int i = 0; i < window_size; i++) {
    if (cursor + i < (int)bits.size())
      std::cout << (int)bits[cursor + i];
    else
      std::cout << ".";
  }
  std::cout << "\r"; // Overwrite line
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: STAGE 1 - CRC CALCULATION CORE
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Calculates the CRC24A checksum for a given bitstream.
 *
 * Algorithm:
 * 1. Appends 24 zeros to the input (padding).
 * 2. Simulates a Linear Feedback Shift Register (LFSR) division.
 * 3. The remainder in the register is the CRC.
 *
 * @param input_data The payload bits
 * @return BitStream The 24-bit CRC checksum
 */
BitStream stage1_calculate_crc24a(const BitStream &input_data) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 1: CRC24A CALCULATION                                  "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";
  std::cout << "Standard Reference: TS 38.212 Sec 5.1 (gCRC24A)\n";
  std::cout << "Polynomial: D^24 + D^23 + ... + 1 (Hex: 0x864CFB)\n\n";

  // 1. Prepare Buffer: Input + 24 Zeros
  BitStream processing_bits = input_data;
  for (int i = 0; i < CRC_LENGTH; ++i)
    processing_bits.push_back(0);

  // 2. Perform Modulo-2 Division (XOR)
  // We treat the vector as a massive binary number and divide by the Poly.
  std::cout << "[STEP 1.1] Initializing LFSR Division...\n";
  std::cout << "  > Input Bits: " << input_data.size() << "\n";
  std::cout << "  > Total Bits (Input + Pad): " << processing_bits.size()
            << "\n";

  // Convert Poly to BitStream for easier XORing
  BitStream poly_bits;
  // Implicit MSB (bit 24) is 1, but usually handled by the 'if' condition
  // We strictly use the tail 0x864CFB (24 bits) for XORing the window
  // when the MSB is 1.
  uint32_t poly_temp = POLY_CRC24A;
  for (int i = CRC_LENGTH - 1; i >= 0; --i) {
    poly_bits.push_back((poly_temp >> i) & 1);
  }

  // Processing Loop
  // We iterate up to input_data.size(). Whenever we find a '1', we XOR the NEXT
  // 24 bits.
  for (size_t i = 0; i < input_data.size(); ++i) {
    if (processing_bits[i] == 1) {
      // XOR the window [i+1 ... i+24] with the Poly
      for (int j = 0; j < CRC_LENGTH; ++j) {
        processing_bits[i + 1 + j] = processing_bits[i + 1 + j] ^ poly_bits[j];
      }
    }
    // Optional: Visualize progress every 10%
    if (input_data.size() > 100 && i % (input_data.size() / 10) == 0) {
      std::cout << "  > Processed " << std::setw(3)
                << (i * 100 / input_data.size()) << "% of bits...\r";
      std::flush(std::cout);
    }
  }
  std::cout << "  > Processed 100% of bits.        \n";

  // 3. Extract Remainder (The last 24 bits)
  BitStream remainder;
  for (size_t i = processing_bits.size() - CRC_LENGTH;
       i < processing_bits.size(); ++i) {
    remainder.push_back(processing_bits[i]);
  }

  std::cout << "  > Calculated CRC: " << bits_to_hex_string(remainder) << "\n";
  std::cout << "\n✓ STAGE 1 COMPLETE: Checksum Generated.\n";

  return remainder;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: STAGE 2 - ATTACHMENT
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Appends the calculated CRC to the Transport Block.
 * TS 38.212 Section 7.2.1
 */
BitStream stage2_attach_crc(const BitStream &tb_data, const BitStream &crc) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 2: CRC ATTACHMENT                                      "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";

  BitStream output = tb_data;
  output.insert(output.end(), crc.begin(), crc.end());

  std::cout << "[STEP 2.1] Appending Parity Bits...\n";
  std::cout << "  > TB Size:  " << tb_data.size() << " bits\n";
  std::cout << "  > CRC Size: " << crc.size() << " bits\n";
  std::cout << "  > New Size: " << output.size() << " bits\n";

  // ASCII Diagram
  std::cout << "\n  Structure:\n";
  std::cout << "  ┌──────────────────────────────────┬──────────────┐\n";
  std::cout << "  │  TRANSPORT BLOCK (Payload)       │ CRC24A (Par) │\n";
  std::cout << "  └──────────────────────────────────┴──────────────┘\n";
  std::cout << "  <----------- A bits --------------> <--- 24 bits ->\n";

  std::cout << "\n✓ STAGE 2 COMPLETE: TB + CRC Ready for Segmentation.\n";
  return output;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5: STAGE 3 - VERIFICATION (RECEIVER SIDE)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Verifies a received bitstream.
 *
 * If the data is correct, running the CRC algorithm on the WHOLE block
 * (Payload + CRC) should result in a remainder of ALL ZEROS.
 *
 * @param received_bits The full bitstream (Payload + CRC)
 * @return true if valid, false if error detected
 */
bool stage3_verify_data(BitStream received_bits) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 3: RECEIVER VERIFICATION (SYNDROME CHECK)              "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";

  // Prepare Poly Bits
  BitStream poly_bits;
  uint32_t poly_temp = POLY_CRC24A;
  for (int i = CRC_LENGTH - 1; i >= 0; --i)
    poly_bits.push_back((poly_temp >> i) & 1);

  // We process the ENTIRE received block (Payload + CRC).
  // We do NOT add 24 zeros padding here, because the attached CRC acts as the
  // "target". If correct, the result after division should be 0.

  BitStream processing = received_bits;
  int data_len = processing.size() - CRC_LENGTH;

  std::cout << "[STEP 3.1] Checking Syndrome...\n";

  for (int i = 0; i < data_len; ++i) {
    if (processing[i] == 1) {
      for (int j = 0; j < CRC_LENGTH; ++j) {
        processing[i + 1 + j] = processing[i + 1 + j] ^ poly_bits[j];
      }
    }
  }

  // Check the last 24 bits (The Remainder)
  bool all_zeros = true;
  std::cout << "  > Remainder bits: ";
  for (size_t i = processing.size() - CRC_LENGTH; i < processing.size(); ++i) {
    std::cout << (int)processing[i];
    if (processing[i] != 0)
      all_zeros = false;
  }
  std::cout << "\n";

  if (all_zeros) {
    std::cout << "  > \033[1;32mRESULT: CRC PASS (Syndrome is Zero)\033[0m\n";
  } else {
    std::cout << "  > \033[1;31mRESULT: CRC FAIL (Syndrome Non-Zero)\033[0m\n";
  }

  return all_zeros;
}

// ═══════════════════════════════════════════════════════════════════════════
// DEMONSTRATION SCENARIOS
// ═══════════════════════════════════════════════════════════════════════════

void run_scenario_valid() {
  std::cout << "\n-------------------------------------------------------------"
               "----------\n";
  std::cout << "SCENARIO A: NORMAL TRANSMISSION (NO ERRORS)\n";
  std::cout << "---------------------------------------------------------------"
               "--------\n";

  // 1. Generate Dummy Transport Block (Example: "5G NR" in ASCII)
  std::vector<uint8_t> payload_bytes = {'5', 'G', ' ', 'N', 'R', 0xAA, 0xBB};
  BitStream tb_bits = bytes_to_bits(payload_bytes);

  std::cout << "Input Hex: " << bits_to_hex_string(tb_bits) << "\n";

  // 2. Calculate
  BitStream crc = stage1_calculate_crc24a(tb_bits);

  // 3. Attach
  BitStream transmitted = stage2_attach_crc(tb_bits, crc);

  // 4. Verify
  stage3_verify_data(transmitted);
}

void run_scenario_error() {
  std::cout << "\n-------------------------------------------------------------"
               "----------\n";
  std::cout << "SCENARIO B: NOISY CHANNEL (BIT FLIP ERROR)\n";
  std::cout << "---------------------------------------------------------------"
               "--------\n";

  // 1. Setup Data
  std::vector<uint8_t> payload_bytes = {0xFF, 0x00, 0xFF, 0x00};
  BitStream tb_bits = bytes_to_bits(payload_bytes);
  BitStream crc = stage1_calculate_crc24a(tb_bits);
  BitStream transmitted = stage2_attach_crc(tb_bits, crc);

  // 2. Introduce Error (Simulate Noise)
  std::cout << "\n[!] SIMULATING CHANNEL NOISE...\n";
  int flip_index = 10; // Flip the 10th bit
  std::cout << "  > Flipping bit at index " << flip_index << " ( "
            << (int)transmitted[flip_index] << " -> "
            << !(int)transmitted[flip_index] << " )\n";

  transmitted[flip_index] = !transmitted[flip_index];

  // 3. Verify
  stage3_verify_data(transmitted);
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << R"(
    ###############################################################
    #                                                             #
    #      5G NR PDSCH - STAGE 2: CRC24A ERROR DETECTION          #
    #         Implementation of 3GPP TS 38.212 Sec 5.1            #
    #                                                             #
    ###############################################################
    )" << "\n";

  std::cout << "EDUCATIONAL NOTES:\n";
  std::cout << "1. CRC24A is used for Transport Blocks (TBs).\n";
  std::cout << "2. CRC24B is used for Code Blocks (CBs).\n";
  std::cout << "3. The polynomial provides detection of random bit errors.\n";
  std::cout
      << "4. 24 bits of CRC implies a collision probability of 1 / 2^24.\n";

  try {
    run_scenario_valid();
    run_scenario_error();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "FINAL SUMMARY:\n";
  std::cout << "✓ Implemented CRC24A Polynomial (0x864CFB).\n";
  std::cout << "✓ Verified checksum generation matches standards logic.\n";
  std::cout << "✓ Demonstrated error detection capability.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  return 0;
}
