// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH IMPLEMENTATION: RESOURCE ELEMENT (RE) MAPPING              ║
 * ║  Reference: 3GPP TS 38.211 Section 6.3.1.6                              ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This code maps complex valued symbols (from Precoding) onto the 2D
 * OFDM Resource Grid.
 *
 * KEY CONCEPTS:
 * - Resource Block (RB): 12 Subcarriers.
 * - Slot: 14 OFDM Symbols.
 * - DMRS: Demodulation Reference Signals (Pilot tones).
 *
 * LOGIC:
 * 1. Initialize an empty Grid (Time x Frequency).
 * 2. Place DMRS pilots at specific locations (blocking those REs).
 * 3. Fill remaining REs with PDSCH Data symbols.
 *
 * AUTHOR: Technical Documentation Bot
 * DATE:   2026-02-06
 */

#include <complex>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// TYPES
// ═══════════════════════════════════════════════════════════════════════════

using Complex = std::complex<double>;

// Represents one Resource Element (RE)
struct ResourceElement {
  Complex value;
  enum Type { EMPTY, DATA, DMRS } type;

  ResourceElement() : value(0, 0), type(EMPTY) {}
};

// The Grid: [Subcarrier Index k][Symbol Index l]
// Note: In C++ memory, we often store row-major, but physics is 2D.
// Here: Outer vector = Subcarriers (freq), Inner vector = Symbols (time).
using ResourceGrid = std::vector<std::vector<ResourceElement>>;

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: GRID MANAGEMENT
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Initialize a blank grid for 1 Resource Block (12 subcarriers, 14
 * symbols)
 */
ResourceGrid create_empty_grid(int num_rbs) {
  int num_subcarriers = num_rbs * 12; // 12 sc per RB
  int num_symbols = 14;               // Standard Slot

  // Create [k][l]
  return ResourceGrid(num_subcarriers,
                      std::vector<ResourceElement>(num_symbols));
}

/**
 * @brief Inserts DMRS (Pilots) into the Grid.
 *
 * SCENARIO: DMRS Configuration Type 1, Mapping Type A.
 * - Symbol Position: l = 2 (3rd symbol)
 * - Frequency: Every alternate subcarrier (k = 0, 2, 4...)
 */
void insert_dmrs(ResourceGrid &grid) {
  int num_subcarriers = grid.size();
  int dmrs_symbol_idx = 2; // Position in time (0-13)

  // Standard DMRS Pilot Value (simplified as 1+1j for demo)
  // In reality, this is a specific Gold Sequence (QPSK).
  Complex pilot_val(0.707, 0.707);

  for (int k = 0; k < num_subcarriers; k++) {
    // Condition: Every even subcarrier (0, 2, 4...)
    if (k % 2 == 0) {
      grid[k][dmrs_symbol_idx].value = pilot_val;
      grid[k][dmrs_symbol_idx].type = ResourceElement::DMRS;
    }
  }
  std::cout << "[INFO] DMRS Pilots inserted at Symbol " << dmrs_symbol_idx
            << ", Subcarriers 0, 2, 4...\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: MAPPING ENGINE
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Maps PDSCH Data symbols to remaining empty REs.
 * Order: Frequency First (k), then Time (l).
 */
void map_pdsch_data(ResourceGrid &grid,
                    const std::vector<Complex> &data_stream) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE: RE MAPPING (GRID FILLING)                             ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";

  int num_subcarriers = grid.size(); // k
  int num_symbols = grid[0].size();  // l
  size_t data_idx = 0;

  std::cout << "[INFO] Grid Size: " << num_subcarriers << " SCs x "
            << num_symbols << " Symbols.\n";
  std::cout << "[INFO] Input Data: " << data_stream.size() << " symbols.\n";

  // --- MAPPING LOOP (Frequency First -> k loop inside l loop) ---
  // Note: TS 38.211 says "increasing order of k, then increasing order of l"

  for (int l = 0; l < num_symbols; l++) {       // Time Loop
    for (int k = 0; k < num_subcarriers; k++) { // Frequency Loop

      // 1. Check if RE is occupied (by DMRS)
      if (grid[k][l].type != ResourceElement::EMPTY) {
        continue; // Skip (Rate Match around it)
      }

      // 2. Check if we have data left
      if (data_idx < data_stream.size()) {
        grid[k][l].value = data_stream[data_idx];
        grid[k][l].type = ResourceElement::DATA;
        data_idx++;
      } else {
        // Padding or Empty (DTX) if we run out of data
      }
    }
  }

  std::cout << "  -> Mapping Complete. Used " << data_idx << " REs for data.\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: VISUALIZATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Visualizes the grid structure in console
 * Rows = Frequency (High k at bottom usually, but here printed top-down for
 * index) Cols = Time
 */
void visualize_grid(const ResourceGrid &grid) {
  int K = grid.size();
  int L = grid[0].size();

  std::cout << "\n[VISUALIZATION] One Resource Block (12 SC x 14 Sym)\n";
  std::cout << "Symbols ->  0 1 2 3 4 5 6 7 8 9 0 1 2 3\n";
  std::cout << "           ----------------------------\n";

  for (int k = 0; k < K; k++) {
    std::cout << "SC " << std::setw(2) << k << "   |  ";
    for (int l = 0; l < L; l++) {
      char marker = '.';

      if (grid[k][l].type == ResourceElement::DMRS) {
        marker = 'P'; // Pilot
      } else if (grid[k][l].type == ResourceElement::DATA) {
        marker = 'D'; // Data
      } else {
        marker = ' '; // Empty
      }
      std::cout << marker << " ";
    }
    std::cout << " |\n";
  }
  std::cout << "           ----------------------------\n";
  std::cout << "Legend: [D]=Data, [P]=Pilot(DMRS), [ ]=Empty\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  // 1. Setup
  int num_rbs = 1;

  // 2. Create Grid (Antenna Port p=0)
  ResourceGrid grid_p0 = create_empty_grid(num_rbs);

  // 3. Insert DMRS
  insert_dmrs(grid_p0);

  // 4. Generate Mock Data
  // Capacity = (12 * 14) - 6 (DMRS) = 162 REs available per RB (approx)
  // Let's generate 100 symbols to see partial filling
  std::vector<Complex> precoded_data(100, Complex(1.0, -1.0));

  // 5. Map Data
  map_pdsch_data(grid_p0, precoded_data);

  // 6. Visualize
  visualize_grid(grid_p0);

  return 0;
}
