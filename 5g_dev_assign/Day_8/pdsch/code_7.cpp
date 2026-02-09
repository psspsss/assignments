// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH IMPLEMENTATION: RATE MATCHING & RV SELECTION               ║
 * ║  Reference: 3GPP TS 38.212 Section 5.4                                  ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This code implements the Rate Matching stage of the PDSCH (Physical Downlink
 * Shared Channel) Transmit Chain. It specifically simulates the Circular Buffer
 * Rate Matching (CBRM) mechanism and the selection of bits based on Redundancy
 * Versions (RV).
 *
 * The code demonstrates how Hybrid Automatic Repeat Request (HARQ) works at
 * the physical layer by selecting different segments of the LDPC encoded
 * "Mother Code" depending on the RV index (0, 1, 2, or 3).
 *
 * IMPLEMENTATION STAGES:
 * 1. CONFIGURATION: Setup LDPC Base Graph parameters (BG1/BG2) and Allocation.
 * 2. RV CALCULATION: Determine starting bit index (k0) based on RV ID.
 * 3. CIRCULAR BUFFER: Perform the bit selection (Puncturing or Repetition).
 * 4. VALIDATION: Verify output lengths and compare HARQ transmissions.
 *
 * AUTHOR: Technical Documentation Bot
 * DATE:   2026-02-06
 *
 * STANDARDS:
 * - 3GPP TS 38.212 V16.x Section 5.4.2 (Rate matching)
 * - Table 5.4.2.1-2: Starting position of different Redundancy Versions
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════

#include <algorithm>
#include <cmath>
#include <cstdint> // <--- ADDED: Required for uint8_t
#include <iomanip>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief LDPC Base Graphs defined in TS 38.212
 */
enum class BaseGraph {
  BG1, // Used for larger Transport Blocks (Kb=22)
  BG2  // Used for smaller Transport Blocks (Kb=10)
};

/**
 * @brief Configuration for the Rate Matching Unit
 */
struct RateMatchConfig {
  BaseGraph bg; // LDPC Base Graph type
  int Zc;       // Lifting size (Zc)
  int rv_id;    // Redundancy Version (0, 1, 2, 3)
  int E;        // Target output bits (Capacity of PDSCH allocation)
  int Qm;       // Modulation Order (e.g., 2=QPSK, 4=16QAM)

  // Calculated internally
  int Kb;  // Number of systematic columns
  int Ncb; // Size of the circular buffer
};

using BitVector = std::vector<uint8_t>;

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Prints a visual representation of the bit buffer usage.
 *
 * @param title Label for the visualization
 * @param start_idx Starting index (k0)
 * @param length Length of selection (E)
 * @param total_size Total buffer size (Ncb)
 */
void visualize_buffer_usage(const std::string &title, int start_idx, int length,
                            int total_size) {
  const int WIDTH = 50;        // ASCII width
  std::string bar(WIDTH, ' '); // ░

  // Convert bits to ASCII char positions
  double scale = (double)WIDTH / total_size;
  int pos_start = (int)(start_idx * scale);
  int pos_len = (int)(length * scale);

  for (int i = 0; i < WIDTH; ++i) {
    bool filled = false;

    // Handle Wrap-around logic for visualization
    if (pos_start + pos_len <= WIDTH) {
      // No wrap
      if (i >= pos_start && i < pos_start + pos_len)
        filled = true;
    } else {
      // Wrap around
      int end_marker = (pos_start + pos_len) % WIDTH;
      if (i >= pos_start || i < end_marker)
        filled = true;
      if (length >= total_size)
        filled = true; // Repetition fills all
    }

    bar[i] = filled ? '#' : '.';
  }

  std::cout << "  " << std::left << std::setw(15) << title << " [";
  for (char c : bar) {
    if (c == '#')
      std::cout << "█";
    else
      std::cout << "░";
  }
  std::cout << "]\n";
}

/**
 * @brief Helper to generate a deterministic "Mother Code" pattern.
 *
 * Logic:
 * - First part (Systematic): 1, 1, 1, 1...
 * - Second part (Parity 1):  0, 1, 0, 1...
 * - Third part (Parity 2):   0, 0, 1, 1...
 */
BitVector generate_mock_ldpc_output(int ncb, int Kb, int Zc) {
  BitVector buffer(ncb);
  int sys_len = Kb * Zc;

  for (int i = 0; i < ncb; i++) {
    if (i < sys_len) {
      buffer[i] = 1; // Systematic bits are all 1s for visibility
    } else if (i < sys_len + (sys_len / 2)) {
      buffer[i] = i % 2; // Parity set 1
    } else {
      buffer[i] = (i % 4) < 2 ? 0 : 1; // Parity set 2
    }
  }
  return buffer;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: STAGE 1 - RV OFFSET CALCULATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Calculates k0 (Starting position) based on RV ID and Base Graph.
 *
 * Reference: 3GPP TS 38.212 Table 5.4.2.1-2
 *
 * @param cfg Configuration struct
 * @return int The bit index k0
 */
int calculate_rv_start_index(const RateMatchConfig &cfg) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 1: RV STARTING POSITION (k0) CALCULATION               ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Standard Reference: TS 38.212 Table 5.4.2.1-2\n\n";

  // Table definition
  const int rv_offsets_bg1[] = {0, 17, 33, 56};
  const int rv_offsets_bg2[] = {0, 13, 25, 43};

  const int *selected_table =
      (cfg.bg == BaseGraph::BG1) ? rv_offsets_bg1 : rv_offsets_bg2;

  // Validation
  if (cfg.rv_id < 0 || cfg.rv_id > 3) {
    std::cerr << "⚠ CRITICAL ERROR: Invalid RV ID " << cfg.rv_id << std::endl;
    return 0;
  }

  // Calculation: k0 = (offset * Zc)
  int spacing_columns = selected_table[cfg.rv_id];
  int k0 = spacing_columns * cfg.Zc;

  std::cout << "[STEP 1.1] Parameters:\n";
  std::cout << "  • Base Graph:      "
            << (cfg.bg == BaseGraph::BG1 ? "BG1" : "BG2") << "\n";
  std::cout << "  • Lifting Size Zc: " << cfg.Zc << "\n";
  std::cout << "  • RV Index:        " << cfg.rv_id << "\n";

  std::cout << "[STEP 1.2] Lookup & Calculate:\n";
  std::cout << "  • Column Offset:   " << spacing_columns
            << " (from standard table)\n";
  std::cout << "  • Formula:         k0 = Offset * Zc\n";
  std::cout << "  • Result k0:       " << k0 << " (Bit Index)\n";

  std::cout << "\n✓ STAGE 1 COMPLETE: Starting position determined.\n";
  return k0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: STAGE 2 - CIRCULAR BUFFER READ
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Selects bits from the circular buffer (Rate Matching).
 *
 * This function simulates reading 'E' bits starting from 'k0', wrapping
 * around the buffer of size 'Ncb'.
 *
 * @param buffer The LDPC Mother Code (Circular Buffer)
 * @param k0 Starting index
 * @param E Target number of bits
 * @return BitVector The rate-matched output bits
 */
BitVector perform_circular_read(const BitVector &buffer, int k0, int E) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 2: CIRCULAR BUFFER READ (PUNCTURING/REPETITION)        ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";

  int Ncb = buffer.size();
  BitVector output;
  output.reserve(E);

  std::cout << "[STEP 2.1] Buffer Status:\n";
  std::cout << "  • Buffer Size (Ncb): " << Ncb << " bits\n";
  std::cout << "  • Requested (E):     " << E << " bits\n";
  std::cout << "  • Start Index (k0):  " << k0 << "\n";

  // Determine Mode
  if (E < Ncb) {
    std::cout << "  • Mode:              PUNCTURING (E < Ncb)\n";
    std::cout << "    (Transmitting a subset of the code bits)\n";
  } else {
    std::cout << "  • Mode:              REPETITION (E >= Ncb)\n";
    std::cout << "    (Transmitting full buffer + repeated bits)\n";
  }

  std::cout << "[STEP 2.2] Bit Extraction Loop:\n";
  // Simulation Loop
  for (int i = 0; i < E; i++) {
    int idx = (k0 + i) % Ncb; // Circular logic
    output.push_back(buffer[idx]);
  }

  // Visualize the read operation
  visualize_buffer_usage("Buffer Read", k0, E, Ncb);

  // Show first few bits
  std::cout << "\n[STEP 2.3] Output Preview (First 16 bits):\n  ";
  for (size_t i = 0; i < std::min((size_t)16, output.size()); i++) {
    std::cout << (int)output[i];
    if ((i + 1) % 4 == 0)
      std::cout << " ";
  }
  std::cout << "...\n";

  std::cout << "\n✓ STAGE 2 COMPLETE: " << output.size()
            << " bits extracted.\n";
  return output;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5: DEMONSTRATION SCENARIOS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Scenario: Standard Initial Transmission (RV=0)
 * Uses Base Graph 1.
 */
void scenario_initial_tx() {
  std::cout << "\n============================================================="
               "===========\n";
  std::cout << "SCENARIO 1: INITIAL TRANSMISSION (RV=0)\n";
  std::cout << "Goal: Transmit Systematic bits first (Self-decodable).\n";
  std::cout << "==============================================================="
               "=========\n";

  RateMatchConfig cfg;
  cfg.bg = BaseGraph::BG1;
  cfg.Zc = 20; // Small lifting size for demo
  cfg.rv_id = 0;
  cfg.Kb = 22;
  cfg.Ncb = 66 * cfg.Zc; // Approx standard sizing (66*Zc for BG1)
  cfg.E = 800;           // Puncturing case (E < Ncb)

  // 1. Generate Mother Code
  BitVector mother_code = generate_mock_ldpc_output(cfg.Ncb, cfg.Kb, cfg.Zc);

  // 2. Calculate k0
  int k0 = calculate_rv_start_index(cfg);

  // 3. Rate Match
  BitVector rm_out = perform_circular_read(mother_code, k0, cfg.E);

  std::cout
      << "ANALYSIS: RV0 starts at index 0. This includes the systematic\n";
  std::cout
      << "part of the LDPC code, essential for the first decode attempt.\n";
}

/**
 * @brief Scenario: HARQ Retransmission (RV=2)
 * Demonstrates Incremental Redundancy.
 */
void scenario_retransmission() {
  std::cout << "\n============================================================="
               "===========\n";
  std::cout << "SCENARIO 2: HARQ RETRANSMISSION (RV=2)\n";
  std::cout << "Goal: Transmit Parity bits to aid decoding (Incremental "
               "Redundancy).\n";
  std::cout << "==============================================================="
               "=========\n";

  RateMatchConfig cfg;
  cfg.bg = BaseGraph::BG1;
  cfg.Zc = 20;
  cfg.rv_id = 2; // Requesting RV2
  cfg.Kb = 22;
  cfg.Ncb = 66 * cfg.Zc;
  cfg.E = 800; // Same allocation size as initial

  // 1. Generate SAME Mother Code (Receiver has buffered the failed attempt)
  BitVector mother_code = generate_mock_ldpc_output(cfg.Ncb, cfg.Kb, cfg.Zc);

  // 2. Calculate k0
  int k0 = calculate_rv_start_index(cfg);

  // 3. Rate Match
  BitVector rm_out = perform_circular_read(mother_code, k0, cfg.E);

  std::cout
      << "ANALYSIS: RV2 starts at a significant offset. Notice the buffer\n";
  std::cout << "visualization shows a different region is selected compared to "
               "Scenario 1.\n";
}

/**
 * @brief Scenario: Edge Case - Repetition (RV=3, E > Ncb)
 * Used when channel conditions are very poor.
 */
void scenario_repetition() {
  std::cout << "\n============================================================="
               "===========\n";
  std::cout << "SCENARIO 3: POOR CHANNEL / REPETITION (RV=3)\n";
  std::cout << "Goal: Transmit full buffer + repeat bits for maximum energy.\n";
  std::cout << "==============================================================="
               "=========\n";

  RateMatchConfig cfg;
  cfg.bg = BaseGraph::BG2; // Smaller graph
  cfg.Zc = 10;
  cfg.rv_id = 3;
  cfg.Kb = 10;
  cfg.Ncb = 50 * cfg.Zc; // 500 bits
  cfg.E = 800;           // E > Ncb (Requires repetition)

  BitVector mother_code = generate_mock_ldpc_output(cfg.Ncb, cfg.Kb, cfg.Zc);

  int k0 = calculate_rv_start_index(cfg);
  perform_circular_read(mother_code, k0, cfg.E);

  std::cout << "ANALYSIS: E (" << cfg.E << ") > Ncb (" << cfg.Ncb << ").\n";
  std::cout << "The reader wraps around the circular buffer to fill the "
               "allocation.\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  // Console formatting
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════╗\n";
  std::cout << "║            5G NR PDSCH RATE MATCHING SIMULATOR               "
               "     ║\n";
  std::cout << "║                 REDUNDANCY VERSIONS (RV)                     "
               "     ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════╝\n";
  std::cout << "Standard: 3GPP TS 38.212 Section 5.4\n";
  std::cout << "Type:     Educational Simulation\n\n";

  std::cout << "EDUCATIONAL NOTE: CIRCULAR BUFFER RATE MATCHING\n";
  std::cout
      << "In 5G, the LDPC encoder generates a large 'Mother Code'. We cannot\n";
  std::cout
      << "always transmit all of it. We select a window of bits to send.\n";
  std::cout
      << "The 'Redundancy Version' (RV) dictates where that window starts.\n\n";
  std::cout << "RV Sequence typically: 0 -> 2 -> 3 -> 1\n\n";

  // Run Scenarios
  try {
    scenario_initial_tx();     // RV 0
    scenario_retransmission(); // RV 2
    scenario_repetition();     // RV 3 with Repetition
  } catch (const std::exception &e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return 1;
  }

  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << "FINAL SUMMARY & TAKEAWAYS\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << "1. k0 Calculation: Dependent on Base Graph (BG1/BG2) and Zc.\n";
  std::cout << "2. RV0 (Initial):  Starts at 0. Prioritizes systematic info.\n";
  std::cout
      << "3. RV > 0 (HARQ):  Starts at offsets. Prioritizes parity info.\n";
  std::cout
      << "4. Incremental Redundancy: Combining RV0 + RV2 at the receiver\n";
  std::cout
      << "   yields a lower effective code rate and higher coding gain.\n";
  std::cout
      << "5. Circular Buffer: Allows seamless Transition between Puncturing\n";
  std::cout << "   and Repetition based on available resources (E).\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";

  return 0;
}
