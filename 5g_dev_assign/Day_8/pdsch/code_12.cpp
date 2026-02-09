// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH IMPLEMENTATION: LAYER MAPPING                              ║
 * ║  Reference: 3GPP TS 38.211 Section 7.3.1.3                              ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This code implements the Layer Mapping stage. It takes 1 or 2 streams of
 * modulation symbols (Codewords) and distributes them across 1 to 8
 * spatial layers.
 *
 * RULES (Table 7.3.1.3-1):
 * - Layers 1-4: Input is Codeword 0.
 * - Layers 5-8: Input is Codeword 0 AND Codeword 1.
 *
 * AUTHOR: Technical Documentation Bot
 * DATE:   2026-02-06
 */

#include <complex>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// TYPES
// ═══════════════════════════════════════════════════════════════════════════

using ComplexSymbol = std::complex<double>;
using SymbolStream = std::vector<ComplexSymbol>; // Represents a Codeword
using LayerGrid = std::vector<SymbolStream>;     // Represents Layers [v][i]

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: MAPPING LOGIC
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Distributes symbols from codewords to layers.
 *
 * @param codewords Input vector containing 1 or 2 SymbolStreams.
 * @param num_layers Target number of layers (v).
 * @return LayerGrid A vector of size 'num_layers', each containing symbols.
 */
LayerGrid perform_layer_mapping(const std::vector<SymbolStream> &codewords,
                                int num_layers) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE: LAYER MAPPING (Spatial Multiplexing)                  ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";

  // 1. Validation
  if (num_layers < 1 || num_layers > 8) {
    throw std::invalid_argument("Error: 5G NR supports 1 to 8 layers only.");
  }

  int num_codewords = codewords.size();
  int required_codewords = (num_layers > 4) ? 2 : 1;

  std::cout << "[INFO] Configuration:\n";
  std::cout << "  • Requested Layers: " << num_layers << "\n";
  std::cout << "  • Input Codewords:  " << num_codewords << "\n";

  if (num_codewords != required_codewords) {
    std::cerr << "⚠ CRITICAL ERROR: " << num_layers << " layers require "
              << required_codewords << " codeword(s), but " << num_codewords
              << " provided.\n";
    return {};
  }

  // 2. Initialize Output Layers
  LayerGrid layers(num_layers);

  // 3. Mapping Algorithm (Switch based on Total Layers)
  // See TS 38.211 Table 7.3.1.3-1

  if (num_layers <= 4) {
    // --- SINGLE CODEWORD MAPPING ---
    const auto &cw0 = codewords[0];
    int M_symb0 = cw0.size();

    if (M_symb0 % num_layers != 0) {
      std::cerr << "⚠ Warning: Input size not divisible by number of layers.\n";
    }

    std::cout << "  • Mode: Single Codeword Distribution\n";

    // Round Robin distribution
    // Layer 0 gets indices 0, v, 2v...
    // Layer 1 gets indices 1, v+1, 2v+1...
    for (int i = 0; i < M_symb0; i++) {
      int layer_idx = i % num_layers;
      layers[layer_idx].push_back(cw0[i]);
    }

  } else {
    // --- DUAL CODEWORD MAPPING (5-8 Layers) ---
    const auto &cw0 = codewords[0];
    const auto &cw1 = codewords[1];

    std::cout << "  • Mode: Dual Codeword Distribution\n";

    // Logic split based on layer count
    int layers_for_cw0 = 0;
    int layers_for_cw1 = 0;

    switch (num_layers) {
    case 5:
      layers_for_cw0 = 2;
      layers_for_cw1 = 3;
      break;
    case 6:
      layers_for_cw0 = 3;
      layers_for_cw1 = 3;
      break;
    case 7:
      layers_for_cw0 = 3;
      layers_for_cw1 = 4;
      break;
    case 8:
      layers_for_cw0 = 4;
      layers_for_cw1 = 4;
      break;
    }

    // Map CW0 to first set of layers
    for (int i = 0; i < (int)cw0.size(); i++) {
      int layer_idx = i % layers_for_cw0;
      layers[layer_idx].push_back(cw0[i]);
    }

    // Map CW1 to remaining layers
    for (int i = 0; i < (int)cw1.size(); i++) {
      int relative_layer_idx = i % layers_for_cw1;
      int absolute_layer_idx = layers_for_cw0 + relative_layer_idx;
      layers[absolute_layer_idx].push_back(cw1[i]);
    }
  }

  // 4. Report Sizes
  std::cout << "[INFO] Output Status:\n";
  for (int v = 0; v < num_layers; v++) {
    std::cout << "  -> Layer " << v << ": " << layers[v].size() << " symbols\n";
  }

  return layers;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILS
// ═══════════════════════════════════════════════════════════════════════════

SymbolStream generate_dummy_symbols(int count, double val_start) {
  SymbolStream s;
  for (int i = 0; i < count; i++) {
    s.push_back(ComplexSymbol(val_start + i, 0)); // Real part increments
  }
  return s;
}

void print_layer_head(const LayerGrid &layers) {
  std::cout << "\n[VISUALIZATION] First 4 symbols per Layer:\n";
  std::cout << "-------------------------------------------\n";
  for (size_t v = 0; v < layers.size(); v++) {
    std::cout << "L" << v << ": ";
    for (size_t i = 0; i < std::min((size_t)4, layers[v].size()); i++) {
      std::cout << "[" << std::real(layers[v][i]) << "] ";
    }
    std::cout << "...\n";
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  try {
    // SCENARIO 1: 2 Layers (MIMO) -> Requires 1 Codeword
    // --------------------------------------------------
    std::cout << "\n--- TEST CASE A: 2 Layers (1 Codeword) ---";
    SymbolStream cw_a = generate_dummy_symbols(10, 0.0); // 0, 1, 2...9

    std::vector<SymbolStream> input_a = {cw_a};
    LayerGrid output_a = perform_layer_mapping(input_a, 2);

    print_layer_head(output_a);
    // Expected:
    // L0: 0, 2, 4...
    // L1: 1, 3, 5...

    // SCENARIO 2: 8 Layers (High Throughput) -> Requires 2 Codewords
    // --------------------------------------------------
    std::cout << "\n--- TEST CASE B: 8 Layers (2 Codewords) ---";

    // CW0 maps to L0, L1, L2, L3 (needs 4x symbols)
    SymbolStream cw_b0 = generate_dummy_symbols(16, 100.0); // 100, 101...
    // CW1 maps to L4, L5, L6, L7 (needs 4x symbols)
    SymbolStream cw_b1 = generate_dummy_symbols(16, 200.0); // 200, 201...

    std::vector<SymbolStream> input_b = {cw_b0, cw_b1};
    LayerGrid output_b = perform_layer_mapping(input_b, 8);

    print_layer_head(output_b);
    // Expected:
    // L0: 100, 104... | L4: 200, 204...

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
