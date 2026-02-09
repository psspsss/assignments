// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH IMPLEMENTATION: MIMO PRECODING                             ║
 * ║  Reference: 3GPP TS 38.211 Section 6.3.1.5                              ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This code implements the MIMO Precoding stage.
 * It transforms the "Layer" domain into the "Antenna Port" domain using a
 * Precoding Matrix (W).
 *
 * INPUT:  v Layers (Data streams)
 * OUTPUT: P Antenna Ports (Physical signals)
 * MATH:   Y = W * X
 *
 * SCENARIOS IMPLEMENTED:
 * 1. Pass-through (Identity Matrix): Used for simple testing.
 * 2. DFT Beamforming: Directing signal energy using phase shifts.
 *
 * AUTHOR: Technical Documentation Bot
 * DATE:   2026-02-06
 */

#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// TYPES
// ═══════════════════════════════════════════════════════════════════════════

using Complex = std::complex<double>;
using DataGrid = std::vector<std::vector<Complex>>; // [Stream][Time]

// The Precoding Matrix W (Rows = Antennas, Cols = Layers)
using PrecodingMatrix = std::vector<std::vector<Complex>>;

const double PI = 3.14159265358979323846;

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: PRECODING ENGINE
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Applies the Precoding Matrix W to the input Layers.
 *
 * @param layers Input data [v][i] (v = num_layers)
 * @param W Precoding Matrix [P][v] (P = num_antennas)
 * @return DataGrid Output data [P][i]
 */
DataGrid apply_precoding(const DataGrid &layers, const PrecodingMatrix &W) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE: MIMO PRECODING (y = Wx)                               ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";

  // 1. Validate Dimensions
  size_t num_layers = layers.size();
  if (num_layers == 0)
    return {};
  size_t num_symbols = layers[0].size();

  size_t P_antennas = W.size(); // Rows of W
  size_t W_cols = W[0].size();  // Cols of W

  if (num_layers != W_cols) {
    std::cerr << "⚠ MATRIX MISMATCH: W expects " << W_cols
              << " layers, but input has " << num_layers << ".\n";
    return {};
  }

  std::cout << "[INFO] Configuration:\n";
  std::cout << "  • Input Layers (v):    " << num_layers << "\n";
  std::cout << "  • Tx Antennas (P):     " << P_antennas << "\n";
  std::cout << "  • Symbols per Stream:  " << num_symbols << "\n";

  // 2. Initialize Output Grid (P x N)
  DataGrid antenna_ports(P_antennas, std::vector<Complex>(num_symbols));

  // 3. Perform Matrix Multiplication for each time instant
  // y(i) = W * x(i)

  for (size_t i = 0; i < num_symbols; i++) {
    // For each antenna port 'p'
    for (size_t p = 0; p < P_antennas; p++) {
      Complex sum(0, 0);

      // Dot product of W[row p] and x(i)
      for (size_t l = 0; l < num_layers; l++) {
        sum += W[p][l] * layers[l][i];
      }
      antenna_ports[p][i] = sum;
    }
  }

  std::cout << "  -> Transformation Complete.\n";
  return antenna_ports;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: MATRIX GENERATORS (HELPER)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Generates an Identity Matrix (Pass-through).
 * Used when Layers maps 1-to-1 with Antennas.
 */
PrecodingMatrix get_identity_matrix(int size) {
  PrecodingMatrix W(size, std::vector<Complex>(size, 0.0));
  for (int i = 0; i < size; i++)
    W[i][i] = 1.0;
  return W;
}

/**
 * @brief Generates a DFT Beamforming Vector (1 Layer -> P Antennas).
 * This simulates steering a beam in a specific direction.
 *
 * Formula elements: exp(j * 2*pi * n * k / N)
 */
PrecodingMatrix get_dft_beam_vector(int P_antennas, int beam_index) {
  PrecodingMatrix W(P_antennas, std::vector<Complex>(1)); // P x 1 Matrix

  // Normalization factor to keep power constant
  double scale = 1.0 / std::sqrt(P_antennas);

  for (int p = 0; p < P_antennas; p++) {
    double angle = 2.0 * PI * p * beam_index / P_antennas;
    W[p][0] = std::polar(scale, angle);
  }
  return W;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: VISUALIZATION
// ═══════════════════════════════════════════════════════════════════════════

void print_grid_head(const DataGrid &grid, std::string label) {
  std::cout << "\n[" << label << "] First symbol (Time i=0):\n";
  for (size_t row = 0; row < grid.size(); row++) {
    Complex val = grid[row][0];
    std::cout << "  Row " << row << ": " << std::fixed << std::setprecision(3)
              << std::showpos << val.real() << " " << val.imag() << "j"
              << "  (Mag: " << std::abs(val) << ", Phase: " << std::noshowpos
              << std::arg(val) * 180 / PI << "°)\n";
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  // SETUP: Create 2 Layers of Mock Data (1 Symbol)
  // Layer 0: symbol (1+0j)
  // Layer 1: symbol (0+1j)
  DataGrid input_layers = {
      {{1.0, 0.0}}, // Layer 0
      {{0.0, 1.0}}  // Layer 1
  };

  // =========================================================
  // SCENARIO 1: 2x2 SPATIAL MULTIPLEXING (Identity)
  // =========================================================
  // Signal from Layer 0 goes to Ant 0, Layer 1 to Ant 1.
  std::cout << "\n--- TEST 1: 2 Layers -> 2 Antennas (Direct Map) ---";

  PrecodingMatrix W_identity = get_identity_matrix(2);
  DataGrid out_siso = apply_precoding(input_layers, W_identity);

  print_grid_head(out_siso, "Output at Antennas");
  // Expect: Ant0 gets Layer0, Ant1 gets Layer1.

  // =========================================================
  // SCENARIO 2: BEAMFORMING (1 Layer -> 4 Antennas)
  // =========================================================
  // We take Layer 0 only and steer it using 4 antennas.
  std::cout << "\n--- TEST 2: 1 Layer -> 4 Antennas (DFT Beamforming) ---";

  DataGrid single_layer = {{{1.0, 0.0}}}; // Input is just "1"

  // Create a DFT beam pointing in a specific direction (Index 1)
  // This creates phase progression: 0°, 90°, 180°, 270° across antennas
  PrecodingMatrix W_beam = get_dft_beam_vector(4, 1);

  DataGrid out_beam = apply_precoding(single_layer, W_beam);

  print_grid_head(out_beam, "Output at Antennas");

  std::cout << "\n[ANALYSIS]\n";
  std::cout << "Notice in Test 2 that the Magnitude is constant (0.500) across "
               "all antennas,\n";
  std::cout << "but the Phase rotates (0->90->180->270). This phase shift "
               "causes the\n";
  std::cout << "signals to constructively interfere at a specific angle in the "
               "air.\n";

  return 0;
}
