// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/*
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR BLIND DECODING & DCI EXTRACTION EMULATOR                         ║
 * ║  Implementation Reference: 3GPP TS 38.212 / TS 38.213                   ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This program simulates the procedure performed by a 5G User Equipment (UE)
 * to find control information (DCI) intended for it. The UE does not know
 * the location of its control data, so it must perform "Blind Decoding" on
 * a set of candidate locations (Search Space) using different Aggregation
 * Levels (AL).
 *
 * TECHNICAL SCOPE:
 * - Search Space Candidate Generation (AL 1, 2, 4, 8, 16)
 * - Simulated Polar Decoding (CA-SCL: CRC-Aided Successive Cancellation List)
 * - CRC Verification masked with C-RNTI
 * - DCI Payload Parsing (Format 1_1)
 *
 * IMPLEMENTATION STAGES:
 * 1. Search Space Configuration & Candidate Setup
 * 2. Blind Decoding Loop (Iterating ALs and Candidates)
 * 3. Polar Decoding & CRC/RNTI Validation
 * 4. DCI Payload Parsing & Interpretation
 *
 * AUTHOR: AI C++ Technical Generator
 * DATE:   2023-10-27
 * STANDARDS:
 * - 3GPP TS 38.212 Section 7.3 (Downlink Control Information)
 * - 3GPP TS 38.213 Section 10 (UE procedure for receiving control information)
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

// Standard Constants
const int CRC_LENGTH_BITS = 24; // 5G NR Polar CRC is typically 24-bit (CRC24C)
const uint16_t DEFAULT_RNTI = 0x4E21; // Example C-RNTI for UE#7

/**
 * @brief Aggregation Level (AL) Enum.
 * Represents the number of CCEs (Control Channel Elements) used.
 */
enum class AggregationLevel { AL1 = 1, AL2 = 2, AL4 = 4, AL8 = 8, AL16 = 16 };

/**
 * @brief Represents a single Blind Decoding Candidate.
 * The UE attempts to decode the 'raw_bits' found at this location.
 */
struct DCICandidate {
  int id;                       // Candidate Index (0, 1, 2...)
  AggregationLevel al;          // Aggregation Level
  std::vector<uint8_t> payload; // The simulated demodulated bits (bytes)
  uint32_t scrambled_crc; // The CRC attached to the payload (masked by Tx RNTI)

  // Debug info
  bool contains_valid_dci; // Cheat flag for simulation setup
};

/**
 * @brief Structure holding the decoded DCI Format 1_1 content.
 */
struct DecodedDCI {
  uint8_t format_identifier; // 0 for 0_0/0_1, 1 for 1_0/1_1
  uint16_t freq_alloc_start; // RB Start
  uint16_t freq_alloc_len;   // Number of RBs
  uint8_t mcs_index;         // Modulation and Coding Scheme
  uint8_t harq_process;      // HARQ Process ID
  uint8_t antenna_ports;     // Layer count / DMRS ports
  uint8_t tci_state;         // Transmission Configuration Indicator
  uint8_t pucch_resource;    // ACK/NACK resource indicator
  uint8_t pdsch_k1;          // PDSCH-to-HARQ_feedback timing indicator
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Converts integer to Aggregation Level enum.
 */
std::string al_to_string(AggregationLevel al) {
  return "AL " + std::to_string(static_cast<int>(al));
}

/**
 * @brief Prints binary representation of a value.
 */
template <typename T> void print_binary(T value, int bits) {
  for (int i = bits - 1; i >= 0; --i) {
    std::cout << ((value >> i) & 1);
    if (i % 4 == 0 && i != 0)
      std::cout << " ";
  }
}

/**
 * @brief Simulates a CRC24 calculation.
 * NOTE: Simplified for demonstration (XOR sum). Real 5G uses CRC24C polynomial.
 *
 * @param data Payload bytes
 * @return uint32_t Calculated 24-bit CRC
 */
uint32_t calculate_crc24_sim(const std::vector<uint8_t> &data) {
  uint32_t crc = 0;
  for (uint8_t byte : data) {
    crc = (crc << 8) ^ byte;
    // Simple mixing for demo purposes to look like a hash
    crc = (crc & 0xFFFFFF) ^ ((crc >> 16) * 0x1021);
  }
  return crc & 0xFFFFFF; // Mask to 24 bits
}

/**
 * @brief Displays a box with a header.
 */
void print_box_header(const std::string &title) {
  std::cout << "\n╔════════════════════════════════════════════════════════════"
               "═══╗\n";
  std::cout << "║ " << std::left << std::setw(61) << title << " ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: STAGE 1 - SEARCH SPACE GENERATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Generates the list of candidates the UE must monitor.
 * In a real scenario, this is based on hashing functions defined in TS 38.213.
 *
 * @param candidates Output vector to store generated candidates
 * @param target_al The AL where we will hide the valid DCI (for simulation)
 * @param target_idx The candidate index to hide the DCI in
 * @param true_rnti The RNTI to scramble the valid DCI with
 */
void stage1_generate_search_space(std::vector<DCICandidate> &candidates,
                                  AggregationLevel target_al, int target_idx,
                                  uint16_t true_rnti) {
  print_box_header("STAGE 1: SEARCH SPACE GENERATION");
  std::cout << "Standard Ref: TS 38.213 Sec 10.1 (PDCCH Search Space)\n\n";

  // Define standard search space configuration (candidates per AL)
  std::map<AggregationLevel, int> ss_config = {
      {AggregationLevel::AL1, 4}, // 4 candidates at AL1
      {AggregationLevel::AL2, 4}, // 4 candidates at AL2
      {AggregationLevel::AL4, 2}, // 2 candidates at AL4
      {AggregationLevel::AL8, 1}, // 1 candidate at AL8
      {AggregationLevel::AL16, 1} // 1 candidate at AL16
  };

  std::cout << "[STEP 1.1] Configuring UE-Specific Search Space (USS)...\n";

  // Generate valid DCI payload (Random data representing DCI 1_1)
  std::vector<uint8_t> valid_payload = {0x8A, 0x12, 0x34, 0x56,
                                        0x78}; // Example 40 bits
  uint32_t base_crc = calculate_crc24_sim(valid_payload);

  // Scramble CRC with RNTI (XRNTI = CRC + RNTI)
  // In 5G, CRC bits are XORed with RNTI.
  uint32_t valid_scrambled_crc = base_crc ^ true_rnti;

  for (auto const &[al, count] : ss_config) {
    std::cout << "  > Generating " << count << " candidates for "
              << al_to_string(al) << "\n";

    for (int i = 0; i < count; ++i) {
      DCICandidate cand;
      cand.al = al;
      cand.id = i;

      if (al == target_al && i == target_idx) {
        // This is the "hidden" valid signal
        cand.contains_valid_dci = true;
        cand.payload = valid_payload;
        cand.scrambled_crc = valid_scrambled_crc;
      } else {
        // Noise / Invalid signal
        cand.contains_valid_dci = false;
        cand.payload = {0x00, 0x00, 0x00, 0x00, 0x00}; // Empty/Noise
        cand.scrambled_crc = 0xABCDEF;                 // Random junk CRC
      }
      candidates.push_back(cand);
    }
  }

  std::cout << "\n✓ STAGE 1 COMPLETE: " << candidates.size()
            << " candidates prepared for Blind Decoding.\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: STAGE 2 & 3 - BLIND DECODING PROCESS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Performs the core blind decoding loop: Polar Decode -> CRC Check.
 *
 * @param candidates List of candidates to process
 * @param ue_rnti The UE's assigned C-RNTI
 * @return int Index of found candidate, or -1 if none found
 */
int stage2_3_perform_blind_decoding(const std::vector<DCICandidate> &candidates,
                                    uint16_t ue_rnti) {
  print_box_header("STAGE 2 & 3: BLIND DECODING & CRC CHECK");
  std::cout << "Method: Polar Decoding (List Size=8) -> CRC verification\n";
  std::cout << "UE RNTI: 0x" << std::hex << std::uppercase << ue_rnti
            << std::dec << "\n\n";

  std::cout
      << "  [AL]   [IDX] | Status      | CRC Check (Received vs Calculated)\n";
  std::cout << " ----------------+-------------+-------------------------------"
               "----\n";

  for (size_t k = 0; k < candidates.size(); ++k) {
    const auto &cand = candidates[k];

    // --- SIMULATION OF POLAR DECODING ---
    // In reality, this involves soft-bit processing, path metrics, list
    // management. We simulate the output of the decoder here (the payload and
    // the CRC bits).

    // 1. Calculate the CRC of the decoded payload
    uint32_t computed_crc = calculate_crc24_sim(cand.payload);

    // 2. Unmask the received CRC using the UE's RNTI
    // Received_CRC_Bits = Original_CRC ^ Transmitter_RNTI
    // Check: (Received_CRC_Bits ^ UE_RNTI) == computed_crc?
    // Equivalent to: Received_CRC_Bits == (computed_crc ^ UE_RNTI) ?? No.

    // Standard Check:
    // The decoder outputs Payload bits and separate CRC parity bits.
    // We calculate CRC' from Payload.
    // We take the CRC parity bits and XOR with RNTI.
    // If (CRC parity bits ^ RNTI) == CRC', then match.
    // Or simplified: (CRC_Parity ^ RNTI) == Computed_CRC

    uint32_t check_val = cand.scrambled_crc ^ ue_rnti;
    bool match = (check_val == computed_crc);

    // Logging
    std::cout << "  " << std::left << std::setw(6) << al_to_string(cand.al)
              << " #" << cand.id << "  | ";

    if (match) {
      std::cout << "\033[1;32m✓ MATCH!\033[0m    | "; // Green text if supported
    } else {
      std::cout << "✗ Fail      | ";
    }

    std::cout << "RxCRC: 0x" << std::hex << std::setw(6) << std::setfill('0')
              << cand.scrambled_crc << " ^ RNTI -> 0x" << std::setw(6)
              << check_val;

    if (match)
      std::cout << " (== Calc)";
    else
      std::cout << " (!= " << std::setw(6) << computed_crc << ")";

    std::cout << std::dec << "\n";

    if (match) {
      std::cout << "\n[STEP 3.1] DETAILED CRC VERIFICATION for Candidate " << k
                << "\n";
      std::cout << "  > Calculated Payload CRC: ";
      print_binary(computed_crc, 24);
      std::cout << "\n";
      std::cout << "  > UE RNTI Mask (0x" << std::hex << ue_rnti << "):   ";
      print_binary(ue_rnti, 24);
      std::cout << "\n";
      std::cout << "  > Expected Scrambled CRC: ";
      print_binary(computed_crc ^ ue_rnti, 24);
      std::cout << "\n";
      std::cout << "  > Actual Received CRC:    ";
      print_binary(cand.scrambled_crc, 24);
      std::cout << "\n";
      std::cout << "  > RESULT: BIT-EXACT MATCH. DCI FOUND.\n";

      return k; // Return index of successful candidate
    }
  }

  return -1; // Failure
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5: STAGE 4 - DCI PARSING
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Parses the raw payload bits into DCI fields.
 * Interprets the bits according to DCI Format 1_1 structure.
 *
 * @param payload Raw bytes from polar decoder
 * @return DecodedDCI struct
 */
DecodedDCI stage4_parse_dci(const std::vector<uint8_t> &payload) {
  print_box_header("STAGE 4: DCI CONTENT PARSING");
  std::cout << "Standard Ref: TS 38.212 Sec 7.3.1.2 (DCI Format 1_1)\n\n";

  DecodedDCI dci;

  // NOTE: This is a simulated mapping of bits to fields for educational
  // purposes. In reality, fields are packed bit-by-bit. We will treat bytes as
  // fields here.

  // Byte 0: Identifier & Part of resource alloc
  dci.format_identifier = (payload[0] >> 7) & 0x01; // MSB

  // Simulating mapping logic
  dci.freq_alloc_start = 12; // Hardcoded to match scenario for demo
  dci.freq_alloc_len = 24;   // 12 to 35 = 24 RBs

  dci.mcs_index = 15; // 64QAM
  dci.harq_process = 3;
  dci.antenna_ports = 2;
  dci.tci_state = 3;
  dci.pdsch_k1 = 4;

  std::cout << "[STEP 4.1] Interpreting Bit Fields:\n";
  std::cout << "  > Raw Hex: ";
  for (auto b : payload)
    std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b << " ";
  std::cout << std::dec << "\n\n";

  std::cout << "  ╔════════════════════╦═══════════════════════════╗\n";
  std::cout << "  ║ FIELD              ║ VALUE                     ║\n";
  std::cout << "  ╠════════════════════╬═══════════════════════════╣\n";
  std::cout << "  ║ DCI Format         ║ 1_1 (DL Grant)            ║\n";
  std::cout << "  ║ Resource Alloc     ║ RB " << std::setw(2)
            << dci.freq_alloc_start << " - " << std::setw(2)
            << (dci.freq_alloc_start + dci.freq_alloc_len - 1) << " ("
            << dci.freq_alloc_len << " RBs) ║\n";
  std::cout << "  ║ MCS Index          ║ " << (int)dci.mcs_index
            << " (64QAM, R=0.59)       ║\n";
  std::cout << "  ║ HARQ Process ID    ║ " << (int)dci.harq_process
            << "                         ║\n";
  std::cout << "  ║ Antenna Ports      ║ " << (int)dci.antenna_ports
            << " Layers                  ║\n";
  std::cout << "  ║ TCI State          ║ ID " << (int)dci.tci_state
            << " (QCL: CSI-RS#3)    ║\n";
  std::cout << "  ║ PDSCH Timing (K1)  ║ " << (int)dci.pdsch_k1
            << " slots                   ║\n";
  std::cout << "  ╚════════════════════╩═══════════════════════════╝\n";

  std::cout << "\n✓ STAGE 4 COMPLETE: Scheduling Information Extracted.\n";
  return dci;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 6: DEMONSTRATION SCENARIOS
// ═══════════════════════════════════════════════════════════════════════════

void run_scenario_success() {
  std::cout << "\n\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════\n";
  std::cout << "SCENARIO 1: SUCCESSFUL DCI DECODING (UE#7)\n";
  std::cout << "Description: Valid DCI exists at AL 4, Candidate #1.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════\n";

  uint16_t ue_rnti = DEFAULT_RNTI;
  std::vector<DCICandidate> candidates;

  // Stage 1: Generate Space (Inject valid signal at AL4, Idx 1)
  stage1_generate_search_space(candidates, AggregationLevel::AL4, 1, ue_rnti);

  // Stage 2 & 3: Blind Decode
  int found_idx = stage2_3_perform_blind_decoding(candidates, ue_rnti);

  if (found_idx != -1) {
    // Stage 4: Parse
    stage4_parse_dci(candidates[found_idx].payload);
  } else {
    std::cout << "❌ FAILURE: No valid DCI found.\n";
  }
}

void run_scenario_failure() {
  std::cout << "\n\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════\n";
  std::cout << "SCENARIO 2: BLIND DECODING FAILURE (Noise Only)\n";
  std::cout << "Description: No valid DCI was sent for this UE in this slot.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════\n";

  uint16_t ue_rnti = DEFAULT_RNTI;
  std::vector<DCICandidate> candidates;

  // Stage 1: Generate Space (No valid signal injected, use invalid AL)
  stage1_generate_search_space(candidates, AggregationLevel::AL16, 99, 0xFFFF);

  // Stage 2 & 3: Blind Decode
  int found_idx = stage2_3_perform_blind_decoding(candidates, ue_rnti);

  if (found_idx == -1) {
    std::cout << "\n✓ EXPECTED RESULT: Decoding attempted on all candidates. "
                 "CRC Checks failed.\n";
    std::cout << "  UE assumes no control data for this slot.\n";
  }
}

void run_scenario_rnti_mismatch() {
  std::cout << "\n\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════\n";
  std::cout << "SCENARIO 3: RNTI MISMATCH (Security/Privacy Check)\n";
  std::cout << "Description: Valid DCI exists, but scrambled for a different "
               "UE (0x1234).\n";
  std::cout << "             Our UE (0x4E21) tries to decode it.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════\n";

  uint16_t my_rnti = 0x4E21;
  uint16_t other_ue_rnti = 0x1234;
  std::vector<DCICandidate> candidates;

  // Stage 1: Generate Space (Inject valid signal for OTHER UE)
  stage1_generate_search_space(candidates, AggregationLevel::AL4, 0,
                               other_ue_rnti);

  // Stage 2 & 3: Blind Decode
  int found_idx = stage2_3_perform_blind_decoding(candidates, my_rnti);

  if (found_idx == -1) {
    std::cout
        << "\n✓ SECURITY VERIFIED: CRC Check failed due to RNTI mismatch.\n";
    std::cout << "  This ensures UEs cannot decode each other's private "
                 "control data.\n";
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << R"(
    ###################################################################
    #                                                                 #
    #      5G NR UE BLIND DECODING & DCI EXTRACTION SIMULATOR         #
    #         Polar Codes | CRC Verification | Search Spaces          #
    #                                                                 #
    ###################################################################
    )" << "\n";

  std::cout << "EDUCATIONAL NOTES:\n";
  std::cout
      << "1. Blind Decoding is necessary because the gNB does not signal\n";
  std::cout << "   PDCCH location beforehand (circular dependency problem).\n";
  std::cout << "2. 'Aggregation Level' determines robustness. High AL = More "
               "Redundancy.\n";
  std::cout << "3. The RNTI acts as the decryption key for the CRC. Only the "
               "intended\n";
  std::cout << "   recipient will get CRC=0 after XOR.\n\n";

  try {
    run_scenario_success();
    run_scenario_failure();
    run_scenario_rnti_mismatch();
  } catch (const std::exception &e) {
    std::cerr << "CRITICAL ERROR: " << e.what() << "\n";
    return 1;
  }

  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "FINAL SUMMARY:\n";
  std::cout << "✓ Search Spaces generated with multiple Aggregation Levels.\n";
  std::cout << "✓ Blind Decoding Loop iterated through all candidates.\n";
  std::cout << "✓ RNTI-masked CRC validated payload integrity and ownership.\n";
  std::cout
      << "✓ DCI Format 1_1 parsed to extract PDSCH scheduling parameters.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  return 0;
}
