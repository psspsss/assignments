// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/*
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH STAGE 5: RATE MATCHING (HARQ RV SELECTION)                 ║
 * ║  Implementation Reference: 3GPP TS 38.212 Section 5.4                   ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This program implements the 5G NR Rate Matching process. It simulates the
 * Circular Buffer interface where bits are selected from the LDPC Mother Code.
 * Depending on the Redundancy Version (RV 0, 1, 2, or 3), the reading starts
 * at different positions to prioritize either Systematic bits (Self-decodable)
 * or Parity bits (Incremental Redundancy).
 *
 * TECHNICAL SCOPE:
 * - Circular Buffer Rate Matching (CBRM)
 * - RV Starting Position Calculation (k0)
 * - Bit Selection (Puncturing vs. Repetition)
 * - Handling of Base Graph 1 and Base Graph 2 offsets
 *
 * IMPLEMENTATION STAGES:
 * 1. Configuration (Allocated REs -> Target Bits E)
 * 2. Starting Position (k0) Calculation based on RV ID
 * 3. Circular Buffer Read Operation
 * 4. Comparison of RV0 vs RV2 outputs
 *
 * AUTHOR: AI C++ Technical Generator
 * DATE:   2026-02-06
 * STANDARDS:
 * - 3GPP TS 38.212 Section 5.4.2.1 (Circular buffer rate matching)
 * - 3GPP TS 38.212 Table 5.4.2.1-2 (Starting position of different RVs)
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

using Bit = uint8_t;
using BitStream = std::vector<Bit>;

enum class BaseGraph {
  BG1, // Large Transport Blocks
  BG2  // Small Transport Blocks
};

/**
 * @brief Input parameters for Rate Matching.
 */
struct RateMatchConfig {
  BaseGraph bg;         // LDPC Base Graph
  int Zc;               // Lifting Size used in encoding
  int rv_id;            // Redundancy Version (0, 1, 2, 3)
  int target_bits_E;    // Capacity of the resource allocation (output length)
  int modulation_order; // Qm (e.g., 2, 4, 6, 8)
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: LOOKUP TABLES (TS 38.212)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Table 5.4.2.1-2: Starting column index for each RV.
 * Key: RV Index (0-3)
 * Value: Column Index in the Base Matrix (relative to start of Ncb)
 */
const std::vector<int> RV_OFFSETS_BG1 = {0, 17, 33, 56};
const std::vector<int> RV_OFFSETS_BG2 = {0, 13, 25, 43};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Generates a dummy Mother Code buffer for demonstration.
 * Creates a pattern where Systematic bits are 0x01 and Parity bits are 0xFF
 * (logically). Actually uses 0 and 1 bit patterns.
 */
BitStream generate_mock_mother_code(int size, int Zc, BaseGraph bg) {
  BitStream buffer(size);
  int Kb = (bg == BaseGraph::BG1) ? 22 : 10;
  int systematic_len = Kb * Zc;

  for (int i = 0; i < size; ++i) {
    if (i < systematic_len) {
      // Systematic Region: Sequential pattern
      buffer[i] = (i / Zc) % 2;
    } else {
      // Parity Region: Random-ish pattern
      buffer[i] = ((i + 7) / 3) % 2;
    }
  }
  return buffer;
}

/**
 * @brief Prints a segment of bits.
 */
void print_bit_segment(const std::string &label, const BitStream &bits,
                       int limit = 20) {
  std::cout << "  " << std::left << std::setw(20) << label << ": [ ";
  for (int i = 0; i < std::min((int)bits.size(), limit); ++i) {
    std::cout << (int)bits[i];
  }
  if ((int)bits.size() > limit)
    std::cout << "...";
  std::cout << " ] (" << bits.size() << " bits)\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: STAGE 1 - STARTING POSITION CALCULATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Calculates k0 (Starting bit index in the circular buffer).
 *
 * Formula: k0 = (ColumnOffset * Zc)
 */
int calculate_start_index(const RateMatchConfig &cfg) {
  const std::vector<int> &offsets =
      (cfg.bg == BaseGraph::BG1) ? RV_OFFSETS_BG1 : RV_OFFSETS_BG2;

  // Validate RV ID
  if (cfg.rv_id < 0 || cfg.rv_id > 3) {
    std::cerr << "Error: Invalid RV ID " << cfg.rv_id << "\n";
    return 0;
  }

  int col_offset = offsets[cfg.rv_id];
  int k0 = col_offset * cfg.Zc;

  std::cout << "[STEP 1] Calculating Start Position (k0)...\n";
  std::cout << "  > Base Graph: " << (cfg.bg == BaseGraph::BG1 ? "BG1" : "BG2")
            << "\n";
  std::cout << "  > RV ID:      " << cfg.rv_id << "\n";
  std::cout << "  > Col Offset: " << col_offset << "\n";
  std::cout << "  > Lifting Zc: " << cfg.Zc << "\n";
  std::cout << "  > Result k0:  " << k0 << " (Bit Index)\n";

  return k0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5: STAGE 2 - CIRCULAR BUFFER READ
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Performs the Rate Matching operation.
 *
 * Logic:
 * 1. Determine Circular Buffer Length (Ncb).
 *    (For simulation, Ncb = Size of Mother Code).
 * 2. Start reading from k0.
 * 3. Read 'target_bits_E' bits, wrapping around if necessary.
 */
BitStream perform_rate_matching(const BitStream &mother_code,
                                const RateMatchConfig &cfg) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 2: CIRCULAR BUFFER READ OPERATION                      "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";

  // 1. Get Start Index
  int k0 = calculate_start_index(cfg);
  int Ncb = mother_code.size();
  int E = cfg.target_bits_E;

  BitStream output;
  output.reserve(E);

  std::cout << "[STEP 2.1] Configuration\n";
  std::cout << "  > Circular Buffer Size (Ncb): " << Ncb << "\n";
  std::cout << "  > Target Output Size (E):     " << E << "\n";

  // Determine Operation Mode
  if (E < Ncb) {
    std::cout << "  > Mode: PUNCTURING (Selecting a subset of bits)\n";
  } else {
    std::cout << "  > Mode: REPETITION (Sending full buffer + wrapping)\n";
  }

  std::cout << "[STEP 2.2] Reading Bits...\n";
  std::cout << "  > Reading from Index " << k0 << " with wrap-around at " << Ncb
            << "...\n";

  // 2. Perform Read Loop
  for (int i = 0; i < E; ++i) {
    int read_index = (k0 + i) % Ncb;
    // NOTE: In real hardware, <NULL> filler bits are skipped here.
    // We assume the mother code passed in has resolved fillers or we just copy
    // them.
    output.push_back(mother_code[read_index]);
  }

  // Visualizing the Selection
  std::cout << "\n  Visual Representation of Selection:\n";
  std::cout << "  Buffer: [0..........................Ncb]\n";

  // Simple ASCII bar chart logic
  int total_width = 40;
  int start_pos = (long)k0 * total_width / Ncb;
  int len_pos = (long)E * total_width / Ncb;

  std::cout << "  Read:   [";
  for (int i = 0; i < total_width; ++i) {
    // Logic to draw the read window accounting for wrap around
    bool in_window = false;
    if (len_pos < total_width) {
      // Normal case
      if (i >= start_pos && i < start_pos + len_pos)
        in_window = true;
      // Wrap case
      if (start_pos + len_pos >= total_width) {
        int end = (start_pos + len_pos) % total_width;
        if (i < end)
          in_window = true;
      }
    } else {
      in_window = true; // Repetition covers everything
    }

    std::cout << (in_window ? "▓" : "░");
  }
  std::cout << "]\n";

  return output;
}

// ═══════════════════════════════════════════════════════════════════════════
// DEMONSTRATION SCENARIOS
// ═══════════════════════════════════════════════════════════════════════════

void run_harq_scenario() {
  std::cout << "\n============================================================="
               "==========\n";
  std::cout << "SCENARIO: HARQ RETRANSMISSION (RV0 vs RV2)\n";
  std::cout << "Goal: Show that different RVs transmit different parts of the "
               "code.\n";
  std::cout << "==============================================================="
               "========\n";

  // Setup
  int Zc = 10;
  BaseGraph bg = BaseGraph::BG1;  // Kb=22. Sys=220 bits.
  int mother_code_size = 66 * Zc; // 660 bits
  int target_E = 300; // Capacity (Less than mother code -> Puncturing)

  // Generate simulated encoded data
  BitStream mother_code = generate_mock_mother_code(mother_code_size, Zc, bg);

  // --- 1. Initial Transmission (RV0) ---
  std::cout << "\n--- TRANSMISSION 1 (RV 0) ---\n";
  RateMatchConfig cfg0 = {bg, Zc, 0, target_E, 2}; // RV=0
  BitStream output_rv0 = perform_rate_matching(mother_code, cfg0);

  // --- 2. Retransmission (RV2) ---
  std::cout << "\n--- TRANSMISSION 2 (RV 2) ---\n";
  RateMatchConfig cfg2 = {bg, Zc, 2, target_E, 2}; // RV=2 (Index 33)
  BitStream output_rv2 = perform_rate_matching(mother_code, cfg2);

  // --- 3. Comparison ---
  std::cout << "\n--- COMPARISON ---\n";
  print_bit_segment("Mother Code (Ref)", mother_code);
  print_bit_segment("RV0 Output", output_rv0);
  print_bit_segment("RV2 Output", output_rv2);

  // Verify they are different
  if (output_rv0 != output_rv2) {
    std::cout << "\n✓ SUCCESS: RV0 and RV2 bitstreams are distinct.\n";
    std::cout << "  RV0 focused on Systematic bits (Start index 0).\n";
    std::cout << "  RV2 focused on Parity bits (Start index " << (33 * Zc)
              << ").\n";
    std::cout << "  This provides 'Incremental Redundancy' gain.\n";
  } else {
    std::cout << "✗ ERROR: Bitstreams are identical.\n";
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << R"(
    ###############################################################
    #                                                             #
    #      5G NR PDSCH - STAGE 5: RATE MATCHING                   #
    #         Implementation of 3GPP TS 38.212 Sec 5.4            #
    #                                                             #
    ###############################################################
    )" << "\n";

  std::cout << "EDUCATIONAL NOTES:\n";
  std::cout << "1. Rate Matching adapts the LDPC output size to the PDSCH "
               "allocation (E).\n";
  std::cout << "2. It uses a Circular Buffer logic.\n";
  std::cout << "3. RV (Redundancy Version) determines the start point in the "
               "buffer.\n";
  std::cout
      << "4. RV0 contains the Data (Systematic bits) and is self-decodable.\n";
  std::cout
      << "5. RV1/2/3 contain Parity bits for helping failed decodes (HARQ).\n";

  try {
    run_harq_scenario();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "FINAL SUMMARY:\n";
  std::cout << "✓ Implemented Table 5.4.2.1-2 (RV starting offsets).\n";
  std::cout << "✓ Implemented Circular Buffer Read logic.\n";
  std::cout << "✓ Visualized Puncturing/Repetition behavior.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  return 0;
}
