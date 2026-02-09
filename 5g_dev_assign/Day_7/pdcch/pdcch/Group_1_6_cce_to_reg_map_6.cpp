/**
 * ═══════════════════════════════════════════════════════════════════════════
 *  TITLE:       5G NR CCE-TO-REG MAPPING (RESOURCE MAPPING)
 *  AUTHOR:      AI C++ Generator
 *  DATE:        2023-10-27
 *  STANDARD:    3GPP TS 38.211 V17.0.0 (Section 7.3.2.4 & 7.4.1.3.2)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *  DESCRIPTION:
 *  This program simulates the mapping of complex modulation symbols to physical
 *  Resource Elements (REs). It handles the hierarchy of the Control Channel:
 *
 *  Hierarchy:
 *  PDCCH Candidate (AL4) -> 4 CCEs -> 24 REGs -> 288 REs
 *
 *  MAPPING LOGIC:
 *  1. [STRUCTURE]  Aggregation Level 4 = 4 CCEs.
 *  2. [REGs]       Each CCE = 6 REGs (Resource Element Groups). Total = 24
 * REGs.
 *  3. [REs]        Each REG = 12 Subcarriers (1 PRB) x 1 Symbol.
 *  4. [DMRS]       Demodulation Reference Signals occupy subcarriers 1, 5, 9
 *                  in every REG (Overhead = 3 REs/REG).
 *  5. [DATA]       Remaining 9 REs/REG are filled with QPSK symbols.
 *                  Capacity: 24 REGs * 9 Data REs = 216 Symbols.
 *
 *  SCENARIO:
 *  - Input:  216 QPSK Symbols
 *  - Mode:   Non-Interleaved (Contiguous Frequency Allocation)
 *  - Output: Physical Grid Visualization
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <complex>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

using ComplexSym = std::complex<double>;

// Dimensions
const int AL_LEVEL = 4;                         // Aggregation Level 4
const int REGS_PER_CCE = 6;                     // Fixed by standard
const int SC_PER_REG = 12;                      // 1 RB width
const int REGS_TOTAL = AL_LEVEL * REGS_PER_CCE; // 24

/**
 * @brief Enum for Resource Element Type
 */
enum class REType {
  EMPTY,
  DATA, // PDSCH/PDCCH Payload
  DMRS  // Demodulation Reference Signal (Pilot)
};

/**
 * @brief Represents a single Resource Element (Smallest Physical Unit)
 */
struct ResourceElement {
  REType type;
  ComplexSym value;
  int sc_index; // 0..11 within the REG
};

/**
 * @brief Represents a Resource Element Group (12 REs)
 */
struct REG {
  int id;         // 0..23
  int cce_parent; // Which CCE this belongs to
  ResourceElement subcarriers[SC_PER_REG];
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Generates dummy QPSK symbols for simulation input
 */
std::vector<ComplexSym> generate_qpsk_symbols(int count) {
  std::vector<ComplexSym> syms;
  const double val = 0.7071; // 1/sqrt(2)
  for (int i = 0; i < count; ++i) {
    // Toggle quadrants for visual variety
    if (i % 4 == 0)
      syms.push_back({val, val});
    else if (i % 4 == 1)
      syms.push_back({val, -val});
    else if (i % 4 == 2)
      syms.push_back({-val, val});
    else
      syms.push_back({-val, -val});
  }
  return syms;
}

/**
 * @brief Visualizes a CCE Block
 */
void print_cce_block(const std::vector<REG> &grid, int cce_idx) {
  int start_reg = cce_idx * REGS_PER_CCE;
  int end_reg = start_reg + REGS_PER_CCE;

  std::cout << " ╔═════════════════════════════════════════════════════════════"
               "════════════════╗\n";
  std::cout << " ║ CCE #" << cce_idx << " (REGs " << start_reg << "-"
            << end_reg - 1 << ") " << std::string(55, ' ') << "║\n";
  std::cout << " ╠══════╤═══╤═══╤═══╤═══╤═══╤═══╤═══╤═══╤═══╤═══╤═══╤═══╣\n";
  std::cout << " ║ SC-> │ 0 │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │10 │11 ║\n";
  std::cout << " ╟──────┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───┼───╢\n";

  for (int r = start_reg; r < end_reg; ++r) {
    std::cout << " ║ REG" << std::setw(2) << r << "│";
    for (int sc = 0; sc < SC_PER_REG; ++sc) {
      REType t = grid[r].subcarriers[sc].type;
      if (t == REType::DMRS) {
        std::cout << " P │"; // P for Pilot/DMRS
      } else if (t == REType::DATA) {
        std::cout << " D │"; // D for Data
      } else {
        std::cout << " . │";
      }
    }
    std::cout << "\n";
  }
  std::cout << " ╚══════╧═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╧═══╝\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: CORE IMPLEMENTATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 1: GRID INITIALIZATION & DMRS INSERTION
 *
 * Sets up the 24 REGs and places the DMRS pilots at indices 1, 5, 9.
 */
std::vector<REG> stage1_init_grid() {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 1: RESOURCE GRID SETUP & DMRS INSERTION                ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Standard: TS 38.211 Section 7.4.1.3.2\n";
  std::cout
      << "Rule: DMRS REs are mapped to subcarriers k = 1, 5, 9 within REG.\n\n";

  std::vector<REG> grid(REGS_TOTAL);

  // Initialize layout
  for (int i = 0; i < REGS_TOTAL; ++i) {
    grid[i].id = i;
    grid[i].cce_parent = i / REGS_PER_CCE;

    // Setup Subcarriers
    for (int k = 0; k < SC_PER_REG; ++k) {
      grid[i].subcarriers[k].sc_index = k;

      // Check DMRS positions (1, 5, 9)
      if (k == 1 || k == 5 || k == 9) {
        grid[i].subcarriers[k].type = REType::DMRS;
        grid[i].subcarriers[k].value = {0, 0}; // Placeholder for Pilot Value
      } else {
        grid[i].subcarriers[k].type = REType::EMPTY;
      }
    }
  }

  std::cout << "[STEP 1.1] Initialized " << REGS_TOTAL << " REGs.\n";
  std::cout << "[STEP 1.2] Inserted DMRS Pilots (3 per REG).\n";
  std::cout << "           Overhead: " << REGS_TOTAL * 3 << " REs.\n";
  std::cout << "           Data Capacity: " << REGS_TOTAL * (12 - 3)
            << " REs.\n";

  std::cout << "✓ STAGE 1 COMPLETE\n";
  return grid;
}

/**
 * @brief STAGE 2: DATA MAPPING
 *
 * Fills the EMPTY slots in the grid with the 216 QPSK symbols.
 * Order: Frequency first (k), then Time (l) - (within REG is just freq).
 * Mapping fills REG 0, then REG 1, etc. (Non-interleaved).
 */
void stage2_map_symbols(std::vector<REG> &grid,
                        const std::vector<ComplexSym> &symbols) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 2: SYMBOL-TO-RE MAPPING                                ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Input: " << symbols.size() << " QPSK Symbols.\n";

  int sym_idx = 0;

  // Iterate through all REGs (0 to 23)
  for (int r = 0; r < REGS_TOTAL; ++r) {
    // Iterate through subcarriers (0 to 11)
    for (int k = 0; k < SC_PER_REG; ++k) {
      // Skip if DMRS
      if (grid[r].subcarriers[k].type != REType::EMPTY)
        continue;

      // Map Symbol
      if (sym_idx < (int)symbols.size()) {
        grid[r].subcarriers[k].type = REType::DATA;
        grid[r].subcarriers[k].value = symbols[sym_idx];
        sym_idx++;
      }
    }
  }

  std::cout << "[STEP 2.1] Mapped " << sym_idx << " symbols into Data REs.\n";
  if (sym_idx == (int)symbols.size()) {
    std::cout << "           All input symbols successfully placed.\n";
  } else {
    std::cout << "Warning: Mismatch in grid capacity vs symbol count.\n";
  }

  std::cout << "✓ STAGE 2 COMPLETE\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION & VISUALIZATION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════╗\n";
  std::cout << "║           5G NR PDCCH CCE-TO-REG MAPPING SIMULATOR           "
               "     ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════╝\n";
  std::cout << " Aggregation Level 4 | Non-Interleaved Mapping\n\n";

  // 1. INPUT
  // Generate the 216 symbols expected from the previous stage
  std::vector<ComplexSym> input_symbols = generate_qpsk_symbols(216);

  // 2. SETUP
  // Initialize Grid with Pilots
  std::vector<REG> grid = stage1_init_grid();

  // 3. MAP
  // Fill Grid with Data
  stage2_map_symbols(grid, input_symbols);

  // 4. VISUALIZE
  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << " VISUALIZATION: RESOURCE ELEMENT GRID\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << " Legend: [D] = Data Symbol, [P] = DMRS Pilot (Ref Signal)\n";
  std::cout << " Note:   DMRS at indices 1, 5, 9 in every REG.\n";

  // Print all 4 CCEs
  for (int c = 0; c < AL_LEVEL; ++c) {
    print_cce_block(grid, c);
  }

  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << " SUMMARY\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << "1. Total REs Utilized:  288 REs (24 REGs x 12 subcarriers)\n";
  std::cout << "2. DMRS Overhead:        72 REs (24 REGs x 3 pilots)\n";
  std::cout
      << "3. Data Payload:        216 REs (Perfect fit for AL4 QPSK payload)\n";
  std::cout << "4. Mapping Type:        Non-Interleaved (Localized Beamforming "
               "capable)\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";

  return 0;
}
