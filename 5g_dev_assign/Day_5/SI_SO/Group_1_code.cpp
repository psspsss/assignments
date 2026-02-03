/**
 * ═══════════════════════════════════════════════════════════════════════════
 * 5G NR RLC PDU SEGMENTATION - COMPLETE IMPLEMENTATION (Stages 1-4)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Standard Compliance: 3GPP TS 38.322 (RLC)
 *
 * This implementation demonstrates the complete RLC PDU segmentation flow:
 *
 *   STAGE 1: RLC SDU Input & Parameter Setup
 *   STAGE 2: Segmentation Decision & RLC PDU Header Construction (SI Field)
 *   STAGE 3: RLC PDU Data Field Population (Data & SO Field)
 *   STAGE 4: RLC PDU Output & Transmission
 *
 * Author: 5G Telecom Developer
 * Date: February 2026
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <algorithm> // For std::min
#include <bitset>    // For std::bitset - FIXED: Added missing header
#include <cstdint>
#include <cstring> // For memcpy, memset
#include <iomanip> // For std::hex, std::setw, std::setfill
#include <iostream>
#include <memory> // For std::unique_ptr
#include <sstream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS (3GPP TS 38.322)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief RLC PDU Header Fields as per 3GPP TS 38.322
 */
struct RlcPduHeader {
  uint8_t si;  // 2-bit Segmentation Info (SI)
  uint16_t so; // 16-bit Segment Offset (SO) - present only if SI != 00
               // Note: For simplicity, D/C, FI, E, SN fields are omitted here.
               // We're focusing purely on SI and SO.
};

/**
 * @brief RLC Protocol Data Unit Structure (Simplified for segmentation)
 */
struct RlcPdu {
  RlcPduHeader header;       // RLC PDU Header
  std::vector<uint8_t> data; // RLC PDU Data field (segment of SDU)
  uint32_t pdu_id;           // A unique ID for this RLC PDU (for tracking)

  // Constructor for easy initialization
  RlcPdu(uint32_t id = 0) : pdu_id(id) {
    header.si = 0;
    header.so = 0;
  }
};

/**
 * @brief Segmentation Info (SI) Field Interpretation (3GPP TS 38.322)
 */
enum SiField {
  SI_COMPLETE_SDU = 0b00, // Data field contains all bytes of an RLC SDU
  SI_FIRST_SEGMENT =
      0b01,               // Data field contains the first segment of an RLC SDU
  SI_LAST_SEGMENT = 0b10, // Data field contains the last segment of an RLC SDU
  SI_MIDDLE_SEGMENT =
      0b11 // Data field contains neither the first nor last segment
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Print hex dump of byte array (for debugging and visualization)
 */
void print_hex(const std::string &label, const uint8_t *data, size_t length) {
  std::cout << label << " (length=" << length << " bytes):\n  0x";
  for (size_t i = 0; i < length; i++) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(data[i]);
    if ((i + 1) % 16 == 0 && i + 1 < length)
      std::cout << "\n  0x";
  }
  std::cout << std::dec << "\n";
}

/**
 * @brief Get string representation of SI field
 */
std::string get_si_description(uint8_t si_value) {
  switch (si_value) {
  case SI_COMPLETE_SDU:
    return "00 (Complete SDU)";
  case SI_FIRST_SEGMENT:
    return "01 (First Segment)";
  case SI_LAST_SEGMENT:
    return "10 (Last Segment)";
  case SI_MIDDLE_SEGMENT:
    return "11 (Middle Segment)";
  default:
    return "Invalid SI Value";
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: STAGE 1 - RLC SDU INPUT & PARAMETER SETUP
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 1: Prepare RLC SDU for segmentation
 *
 * This stage receives a higher-layer (e.g., PDCP) SDU and identifies
 * its properties needed for RLC processing.
 *
 * Steps in Stage 1:
 * 1. Receive incoming RLC Service Data Unit (SDU)
 * 2. Determine the total length of the RLC SDU
 * 3. Define the maximum RLC PDU data field size (from lower layer config)
 *
 * @param sdu_data          Raw byte vector of the RLC SDU
 * @param max_pdu_data_size Maximum data field size for an RLC PDU (bytes)
 * @param sdu_id            A unique ID for this SDU
 *
 * @return 0 on success, negative error code on failure
 *
 * @note 3GPP TS 38.322 Section 5.1 (RLC entities and service primitives)
 */
int stage1_prepare_sdu(const std::vector<uint8_t> &sdu_data,
                       uint16_t &max_pdu_data_size, uint32_t sdu_id) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 1: RLC SDU INPUT & PARAMETER SETUP                     ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "3GPP Reference: TS 38.322 Section 5.1 (RLC entities)\n\n";

  std::cout << "[STEP 1.1] Receiving RLC Service Data Unit (SDU)\n";
  std::cout << "  → SDU ID: " << sdu_id << "\n";
  std::cout << "  → SDU Length: " << sdu_data.size() << " bytes\n";
  print_hex("  → SDU Data (hex)", sdu_data.data(), sdu_data.size());

  std::cout << "\n[STEP 1.2] Determining RLC PDU Data Field Capacity\n";
  std::cout << "  → Configured Max RLC PDU Data Field Size: "
            << max_pdu_data_size << " bytes\n";
  std::cout << "  (This value is typically determined by lower layer (MAC) "
               "capacity)\n";

  std::cout << "\n[STAGE 1 SUMMARY] SDU Input Parameters:\n";
  std::cout << "  ┌─────────────────────────────────────────────────────┐\n";
  std::cout << "  │ Parameter                  │ Value                │\n";
  std::cout << "  ├─────────────────────────────────────────────────────┤\n";
  std::cout << "  │ SDU ID                     │ " << std::setw(18) << sdu_id
            << " │\n";
  std::cout << "  │ SDU Total Length (bytes)   │ " << std::setw(18)
            << sdu_data.size() << " │\n";
  std::cout << "  │ Max RLC PDU Data Size      │ " << std::setw(18)
            << max_pdu_data_size << " │\n";
  std::cout << "  └─────────────────────────────────────────────────────┘\n";

  std::cout << "\n✓ STAGE 1 COMPLETE: SDU prepared for segmentation\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: STAGE 2 - SEGMENTATION DECISION & HEADER CONSTRUCTION (SI)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 2: Decide segmentation and construct RLC PDU header (SI field)
 *
 * This stage determines whether the RLC SDU needs to be segmented based on
 * the available RLC PDU data field size. It then sets the Segmentation Info
 * (SI) field in the RLC PDU header.
 *
 * Steps in Stage 2:
 * 1. Compare SDU length with max PDU data size.
 * 2. If SDU fits, SI = '00' (Complete SDU).
 * 3. If SDU is larger, it needs segmentation:
 *    - First PDU: SI = '01' (First Segment)
 *    - Middle PDU(s): SI = '11' (Middle Segment)
 *    - Last PDU: SI = '10' (Last Segment)
 *
 * @param sdu_length        Total length of the original RLC SDU
 * @param current_offset    Current byte offset in the original SDU
 * @param bytes_this_pdu    Number of bytes this PDU will carry
 * @param max_pdu_data_size Maximum data field size for an RLC PDU
 * @param rlc_pdu          [out] RLC PDU to populate with header info
 *
 * @return 0 on success
 *
 * @note 3GPP TS 38.322 Section 6.2.3.4 (SI field)
 */
int stage2_segmentation_decision_si(uint32_t sdu_length,
                                    uint32_t current_offset,
                                    uint16_t bytes_this_pdu,
                                    uint16_t max_pdu_data_size,
                                    RlcPdu &rlc_pdu) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 2: SEGMENTATION DECISION & HEADER CONSTRUCTION (SI)    ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "3GPP Reference: TS 38.322 Section 6.2.3.4 (SI field)\n\n";

  std::cout << "[STEP 2.1] Analyzing SDU and PDU Capacity\n";
  std::cout << "  → SDU Total Length: " << sdu_length << " bytes\n";
  std::cout << "  → Current Offset in SDU: " << current_offset << " bytes\n";
  std::cout << "  → Bytes for this PDU: " << bytes_this_pdu << " bytes\n";
  std::cout << "  → Max RLC PDU Data Field Size: " << max_pdu_data_size
            << " bytes\n\n";

  // Step 2.2: Determine SI field value
  std::cout << "[STEP 2.2] Determining Segmentation Info (SI) Field\n";

  if (sdu_length <= max_pdu_data_size) {
    // SDU fits into a single PDU
    rlc_pdu.header.si = SI_COMPLETE_SDU;
    std::cout << "  → SDU fits in one PDU. SI = "
              << get_si_description(rlc_pdu.header.si) << "\n";
  } else {
    // SDU requires segmentation
    if (current_offset == 0) {
      // This is the first segment
      rlc_pdu.header.si = SI_FIRST_SEGMENT;
      std::cout << "  → First segment of SDU. SI = "
                << get_si_description(rlc_pdu.header.si) << "\n";
    } else if ((current_offset + bytes_this_pdu) >= sdu_length) {
      // This is the last segment
      rlc_pdu.header.si = SI_LAST_SEGMENT;
      std::cout << "  → Last segment of SDU. SI = "
                << get_si_description(rlc_pdu.header.si) << "\n";
    } else {
      // This is a middle segment
      rlc_pdu.header.si = SI_MIDDLE_SEGMENT;
      std::cout << "  → Middle segment of SDU. SI = "
                << get_si_description(rlc_pdu.header.si) << "\n";
    }
  }

  std::cout << "\n[STAGE 2 SUMMARY] RLC PDU Header (SI field):\n";
  std::cout << "  ┌─────────────────────────────────────────────────────┐\n";
  std::cout << "  │ Field   │ Value (Binary) │ Description            │\n";
  std::cout << "  ├─────────────────────────────────────────────────────┤\n";
  // FIXED: Proper bitset usage with separate conversion
  std::cout << "  │ SI      │ " << std::bitset<2>(rlc_pdu.header.si).to_string()
            << "b         │ " << std::setw(22) << std::left
            << get_si_description(rlc_pdu.header.si) << " │\n";
  std::cout << "  └─────────────────────────────────────────────────────┘\n";

  std::cout << "\n✓ STAGE 2 COMPLETE: SI field determined for RLC PDU "
            << rlc_pdu.pdu_id << "\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5: STAGE 3 - RLC PDU DATA FIELD POPULATION (DATA & SO)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 3: Populate RLC PDU data field and set Segment Offset (SO)
 *
 * This stage copies the relevant segment of the original RLC SDU into the
 * RLC PDU's data field and sets the Segment Offset (SO) if segmentation
 * has occurred (i.e., SI is not '00').
 *
 * Steps in Stage 3:
 * 1. Copy the segment from the SDU based on current_offset and bytes_this_pdu.
 * 2. If SI != '00', set the SO field to the current_offset.
 * 3. If SI == '00', the SO field is not present/relevant (implicitly 0).
 *
 * @param sdu_data          Original RLC SDU data
 * @param current_offset    Starting byte offset of this segment in SDU
 * @param bytes_this_pdu    Number of bytes this PDU carries
 * @param rlc_pdu          [out] RLC PDU to populate with data and SO
 *
 * @return 0 on success
 *
 * @note 3GPP TS 38.322 Section 6.2.3.5 (SO field)
 */
int stage3_populate_pdu_data_so(const std::vector<uint8_t> &sdu_data,
                                uint32_t current_offset,
                                uint16_t bytes_this_pdu, RlcPdu &rlc_pdu) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 3: RLC PDU DATA FIELD POPULATION (DATA & SO FIELD)     ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "3GPP Reference: TS 38.322 Section 6.2.3.5 (SO field)\n\n";

  std::cout << "[STEP 3.1] Copying SDU Segment to RLC PDU Data Field\n";
  std::cout << "  → Original SDU Length: " << sdu_data.size() << " bytes\n";
  std::cout << "  → Segment Start Offset: " << current_offset << " bytes\n";
  std::cout << "  → Segment Length: " << bytes_this_pdu << " bytes\n";

  rlc_pdu.data.resize(bytes_this_pdu);
  if (current_offset + bytes_this_pdu > sdu_data.size()) {
    std::cerr << "Error: Attempting to copy beyond SDU bounds.\n";
    return -1;
  }
  memcpy(rlc_pdu.data.data(), sdu_data.data() + current_offset, bytes_this_pdu);

  print_hex("  → RLC PDU Data Field (Segment)", rlc_pdu.data.data(),
            rlc_pdu.data.size());

  std::cout << "\n[STEP 3.2] Setting Segment Offset (SO) Field\n";
  if (rlc_pdu.header.si != SI_COMPLETE_SDU) {
    rlc_pdu.header.so = current_offset;
    std::cout << "  → SI indicates segmentation, SO field is present.\n";
    std::cout << "  → SO Value: " << rlc_pdu.header.so << " (0x" << std::hex
              << std::setw(4) << std::setfill('0') << rlc_pdu.header.so
              << std::dec << ") - This is the byte offset.\n";
    std::cout << "  → The first byte of this segment corresponds to byte "
              << rlc_pdu.header.so << " in the original SDU.\n";
  } else {
    std::cout << "  → SI indicates a complete SDU, SO field is not present.\n";
    // For internal representation, we can keep it 0 or explicitly mark as not
    // applicable.
    rlc_pdu.header.so = 0;
  }

  std::cout << "\n[STAGE 3 SUMMARY] RLC PDU Data Field & SO:\n";
  std::cout << "  ┌─────────────────────────────────────────────────────┐\n";
  std::cout << "  │ Field   │ Value                                   │\n";
  std::cout << "  ├─────────────────────────────────────────────────────┤\n";
  std::cout << "  │ Data    │ " << std::setw(39) << std::left
            << "Segment of SDU"
            << " │\n";
  std::cout << "  │ SO      │ " << std::setw(18) << std::right
            << (rlc_pdu.header.si != SI_COMPLETE_SDU
                    ? std::to_string(rlc_pdu.header.so) + " bytes"
                    : "Not Applicable")
            << std::right << "                  │\n";
  std::cout << "  └─────────────────────────────────────────────────────┘\n";

  std::cout << "\n✓ STAGE 3 COMPLETE: RLC PDU " << rlc_pdu.pdu_id
            << " data and SO populated\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 6: STAGE 4 - RLC PDU OUTPUT & TRANSMISSION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 4: Construct final RLC PDU for lower layer (MAC) transmission
 *
 * This stage combines the RLC PDU header (including SI and SO) and the
 * RLC PDU data field into a single byte stream ready for transmission
 * to the MAC layer.
 *
 * RLC PDU structure (simplified):
 *   +-----------------+--------------------------+
 *   | RLC PDU Header  | RLC PDU Data Field       |
 *   | (incl. SI, SO)  | (segment of RLC SDU)     |
 *   +-----------------+--------------------------+
 *
 * @param rlc_pdu       The fully constructed RLC PDU
 * @param output_buffer [out] Complete PDU ready for transmission
 * @param output_length [out] Total length of output PDU in bytes
 *
 * @return 0 on success
 *
 * @note 3GPP TS 38.322 Section 6.2 (RLC PDU formats)
 */
int stage4_output_rlc_pdu(const RlcPdu &rlc_pdu,
                          std::vector<uint8_t> &output_buffer) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 4: RLC PDU OUTPUT & TRANSMISSION PREPARATION            ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "3GPP Reference: TS 38.322 Section 6.2 (RLC PDU formats)\n\n";

  // Step 4.1: Determine RLC PDU header size (simplified to 1 or 3 bytes)
  // For simplicity, we assume:
  // - 1 byte header if SI = 00 (no SO field)
  // - 3 bytes header if SI != 00 (includes 16-bit SO field)
  size_t header_size = (rlc_pdu.header.si == SI_COMPLETE_SDU) ? 1 : 3;
  size_t total_pdu_length = header_size + rlc_pdu.data.size();
  output_buffer.resize(total_pdu_length);
  memset(output_buffer.data(), 0, total_pdu_length); // Clear buffer

  std::cout << "[STEP 4.1] Constructing Final RLC PDU Byte Stream\n";
  std::cout << "  RLC PDU Format:\n";
  std::cout
      << "  ┌──────────────────────────────────────────────────────────┐\n";
  std::cout
      << "  │ RLC Header (SI, [SO])    │ RLC PDU Data Field            │\n";
  std::cout
      << "  └──────────────────────────────────────────────────────────┘\n";
  std::cout << "  → Header Size: " << header_size << " bytes\n";
  std::cout << "  → Data Field Size: " << rlc_pdu.data.size() << " bytes\n";
  std::cout << "  → Total PDU Size: " << total_pdu_length << " bytes\n\n";

  // Assemble the header bytes
  // Byte 0: D/C, FI, E, RLC SN (omitted for simplicity, but SI is here)
  // For this demo, let's assume SI is encoded in the first byte.
  // In real RLC, SI bits are usually part of a larger header structure.
  // We'll put SI in the two MSBs of the first byte for demonstration.
  output_buffer[0] = (rlc_pdu.header.si << 6); // Shift SI to top 2 bits

  std::cout << "[STEP 4.2] Encoding RLC PDU Header\n";
  // FIXED: Proper bitset usage with separate conversion
  std::cout << "  → Encoded SI (first byte, bits [7:6]): "
            << std::bitset<2>(rlc_pdu.header.si).to_string() << "b\n";
  std::cout << "  → Header Byte 0: 0x" << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<int>(output_buffer[0])
            << std::dec << "\n";

  if (rlc_pdu.header.si != SI_COMPLETE_SDU) {
    // SO field is present (16 bits)
    output_buffer[1] = (rlc_pdu.header.so >> 8) & 0xFF; // MSB
    output_buffer[2] = rlc_pdu.header.so & 0xFF;        // LSB
    std::cout << "  → Encoded SO (bytes 1-2): 0x" << std::hex << std::setw(4)
              << std::setfill('0') << rlc_pdu.header.so << std::dec << "\n";
    std::cout << "  → Header Byte 1: 0x" << std::hex << std::setw(2)
              << std::setfill('0') << static_cast<int>(output_buffer[1])
              << std::dec << "\n";
    std::cout << "  → Header Byte 2: 0x" << std::hex << std::setw(2)
              << std::setfill('0') << static_cast<int>(output_buffer[2])
              << std::dec << "\n";
  } else {
    std::cout << "  → SO field not present (SI=00).\n";
  }

  // Copy data field
  memcpy(output_buffer.data() + header_size, rlc_pdu.data.data(),
         rlc_pdu.data.size());

  print_hex("\n  → Complete RLC PDU " + std::to_string(rlc_pdu.pdu_id),
            output_buffer.data(), total_pdu_length);

  std::cout << "\n[STAGE 4 SUMMARY] RLC PDU " << rlc_pdu.pdu_id
            << " Ready for Transmission:\n";
  std::cout << "  ┌─────────────────────────────────────────────────────┐\n";
  std::cout << "  │ Attribute                  │ Value                │\n";
  std::cout << "  ├─────────────────────────────────────────────────────┤\n";
  std::cout << "  │ PDU ID                     │ " << std::setw(18)
            << rlc_pdu.pdu_id << " │\n";
  std::cout << "  │ SI Field                   │ " << std::setw(18)
            << get_si_description(rlc_pdu.header.si) << " │\n";
  std::cout << "  │ SO Field                   │ " << std::setw(18)
            << (rlc_pdu.header.si != SI_COMPLETE_SDU
                    ? std::to_string(rlc_pdu.header.so)
                    : "N/A")
            << " │\n";
  std::cout << "  │ Header Size (bytes)        │ " << std::setw(18)
            << header_size << " │\n";
  std::cout << "  │ Data Field Size (bytes)    │ " << std::setw(18)
            << rlc_pdu.data.size() << " │\n";
  std::cout << "  │ Total PDU Size (bytes)     │ " << std::setw(18)
            << total_pdu_length << " │\n";
  std::cout << "  └─────────────────────────────────────────────────────┘\n";

  std::cout << "\n✓ STAGE 4 COMPLETE: RLC PDU " << rlc_pdu.pdu_id
            << " ready for MAC layer!\n";
  std::cout << "\n[TRANSMISSION] RLC PDU " << rlc_pdu.pdu_id
            << " sent to MAC layer...\n";

  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 7: MAIN DEMONSTRATION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "  5G NR RLC PDU SEGMENTATION FLOW DEMONSTRATION (Stages 1-4)\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "Standard Compliance:\n";
  std::cout << "  • 3GPP TS 38.322 - Radio Link Control (RLC) Protocol\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  // ═══════════════════════════════════════════════════════════════════════
  // SIMULATION SETUP
  // ═══════════════════════════════════════════════════════════════════════

  std::cout << "\n[SIMULATION] Sending an RLC SDU from PDCP to RLC Layer\n";

  // Example 1: SDU that fits in one PDU
  std::string short_message_str = "Short data SDU.";
  std::vector<uint8_t> short_sdu(short_message_str.begin(),
                                 short_message_str.end());
  uint16_t max_pdu_data_field_size = 20; // Max 20 bytes for RLC PDU data field

  std::cout << "\n--- Scenario 1: SDU Fits in a Single RLC PDU ---\n";
  std::cout << "  → RLC SDU length: " << short_sdu.size() << " bytes\n";
  std::cout << "  → Max RLC PDU Data Field Size: " << max_pdu_data_field_size
            << " bytes\n";

  if (stage1_prepare_sdu(short_sdu, max_pdu_data_field_size, 100) != 0)
    return -1;

  RlcPdu pdu1(1);
  uint32_t current_offset_s1 = 0;
  uint16_t bytes_for_pdu1_s1 = std::min(
      (uint16_t)short_sdu.size(), max_pdu_data_field_size); // SDU fits fully

  if (stage2_segmentation_decision_si(short_sdu.size(), current_offset_s1,
                                      bytes_for_pdu1_s1,
                                      max_pdu_data_field_size, pdu1) != 0)
    return -1;
  if (stage3_populate_pdu_data_so(short_sdu, current_offset_s1,
                                  bytes_for_pdu1_s1, pdu1) != 0)
    return -1;

  std::vector<uint8_t> final_pdu1_buffer;
  if (stage4_output_rlc_pdu(pdu1, final_pdu1_buffer) != 0)
    return -1;

  // Example 2: SDU that requires segmentation
  std::string long_message_str =
      "This is a much longer RLC Service Data Unit that clearly requires "
      "segmentation into multiple RLC Protocol Data Units. RLC ensures "
      "efficient transmission over the air interface by breaking large data "
      "blocks into smaller, manageable chunks, and providing reordering and "
      "error correction. The Segment Offset field is crucial for the receiver "
      "to correctly reassemble the original SDU.";
  std::vector<uint8_t> long_sdu(long_message_str.begin(),
                                long_message_str.end());
  max_pdu_data_field_size = 25; // Smaller PDU size to force more segments

  std::cout << "\n\n--- Scenario 2: SDU Requires Segmentation ---\n";
  std::cout << "  → RLC SDU length: " << long_sdu.size() << " bytes\n";
  std::cout << "  → Max RLC PDU Data Field Size: " << max_pdu_data_field_size
            << " bytes\n";

  if (stage1_prepare_sdu(long_sdu, max_pdu_data_field_size, 200) != 0)
    return -1;

  current_offset_s1 = 0;
  uint32_t pdu_count = 1;
  while (current_offset_s1 < long_sdu.size()) {
    std::cout << "\nProcessing PDU #" << pdu_count << " for SDU 200\n";
    RlcPdu current_pdu(pdu_count);

    // Determine how many bytes this PDU will carry
    uint16_t bytes_for_this_pdu =
        std::min((uint16_t)(long_sdu.size() - current_offset_s1),
                 max_pdu_data_field_size);

    if (stage2_segmentation_decision_si(
            long_sdu.size(), current_offset_s1, bytes_for_this_pdu,
            max_pdu_data_field_size, current_pdu) != 0)
      return -1;
    if (stage3_populate_pdu_data_so(long_sdu, current_offset_s1,
                                    bytes_for_this_pdu, current_pdu) != 0)
      return -1;

    std::vector<uint8_t> final_current_pdu_buffer;
    if (stage4_output_rlc_pdu(current_pdu, final_current_pdu_buffer) != 0)
      return -1;

    current_offset_s1 += bytes_for_this_pdu;
    pdu_count++;
  }

  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "  COMPLETE RLC SEGMENTATION FLOW SUMMARY\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "Scenario 1: SDU (\"" << short_message_str << "\")\n";
  std::cout << "  → Segmented into 1 RLC PDU.\n";
  std::cout << "Scenario 2: SDU (\"" << long_message_str.substr(0, 50) + "..."
            << "\")\n"; // Print truncated message
  std::cout << "  → Segmented into " << pdu_count - 1 << " RLC PDUs.\n";
  std::cout << "───────────────────────────────────────────────────────────────"
               "────────────\n";
  std::cout
      << "Purpose: RLC segmentation ensures efficient handling of varying "
         "SDU sizes\n";
  std::cout
      << "         and optimizes data transfer over the radio interface.\n";
  std::cout << "Key Fields: SI (Segmentation Info) and SO (Segment Offset) are "
               "crucial\n";
  std::cout << "            for accurate reassembly at the receiver.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  return 0;
}
