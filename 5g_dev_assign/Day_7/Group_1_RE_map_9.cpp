// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  TITLE:       5G NR SSB: RESOURCE ELEMENT (RE) MAPPING                  ║
 * ║  AUTHOR:      AI Technical Documentation Generator                      ║
 * ║  DATE:        2023-10-27                                                ║
 * ║  STANDARD:    3GPP TS 38.211 Section 7.4.3 (SS/PBCH Block)              ║
 * ║                                                                         ║
 * ║  DESCRIPTION:                                                           ║
 * ║  Implements Stage 9 of the PBCH chain. Maps the generated components    ║
 * ║  onto the Synchronization Signal Block (SSB) Time-Frequency Grid.       ║
 * ║                                                                         ║
 * ║  GRID STRUCTURE (4 Symbols x 240 Subcarriers):                          ║
 * ║  • Symbol 0: PSS (Primary Sync Signal)                                  ║
 * ║  • Symbol 1: PBCH Data + DMRS                                           ║
 * ║  • Symbol 2: SSS + PBCH Data + DMRS                                     ║
 * ║  • Symbol 3: PBCH Data + DMRS                                           ║
 * ║                                                                         ║
 * ║  INPUT SCENARIO:                                                        ║
 * ║  • Cell ID: 276, SSB Index: 3                                           ║
 * ║  • Input Payload: 432 Complex QPSK Symbols                              ║
 * ║  • DMRS Init: 575488                                                    ║
 * ║                                                                         ║
 * ║  OUTPUT:                                                                ║
 * ║  • 960 Resource Elements (REs) populated with complex values.           ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

// ─── SSB Geometry ───
const int SSB_SYMBOLS = 4;
const int SSB_SC = 240; // Subcarriers (20 RBs * 12 SC)
const int RE_TOTAL = SSB_SYMBOLS * SSB_SC;

// ─── Component Constraints ───
const int NUM_PBCH_SYMBOLS = 432; // Input QPSK data size
const int NUM_DMRS_SYMBOLS = 144; // Total DMRS count
const int PSS_SSS_WIDTH = 127;    // Width of sync signals
const int PSS_SSS_START = 56;     // Start index for PSS/SSS (56 to 182)

// ─── Configuration ───
const int N_ID_CELL = 276;
const int I_SSB = 3;
const uint32_t DMRS_C_INIT = 575488; // Pre-calculated from prompt

// ─── Types ───
using ComplexSym = std::complex<double>;

// Enum to tag what is inside an RE for visualization
enum class REType { EMPTY, PSS, SSS, PBCH_DATA, PBCH_DMRS, GUARD };

struct ResourceElement {
  ComplexSym value;
  REType type;
};

// The SSB Grid: 4 rows (symbols) x 240 columns (subcarriers)
using SSBGrid = std::vector<std::vector<ResourceElement>>;

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Converts REType enum to a short string code for printing.
 */
std::string get_type_str(REType t) {
  switch (t) {
  case REType::EMPTY:
    return " . ";
  case REType::PSS:
    return "PSS";
  case REType::SSS:
    return "SSS";
  case REType::PBCH_DATA:
    return "DAT";
  case REType::PBCH_DMRS:
    return "DMR";
  case REType::GUARD:
    return "___";
  default:
    return " ? ";
  }
}

/**
 * @brief Helper to format complex numbers for display.
 */
std::string fmt_complex(ComplexSym c) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2);
  if (std::abs(c.real()) < 0.01 && std::abs(c.imag()) < 0.01)
    return "      0      ";

  ss << (c.real() >= 0 ? "+" : "") << c.real();
  ss << (c.imag() >= 0 ? "+j" : "-j") << std::abs(c.imag());
  return ss.str();
}

/**
 * @brief Pseudo-Random Sequence Generator (Gold Sequence)
 * Simplified 3GPP TS 38.211 Sec 5.2.1 implementation logic for DMRS.
 *
 * @param c_init Initialization seed
 * @param length Number of bits needed
 * @return std::vector<uint8_t> Sequence of bits
 */
std::vector<uint8_t> generate_gold_sequence(uint32_t c_init, int length) {
  std::vector<uint8_t> c(length);
  // Note: In a full implementation, this uses two length-31 m-sequences.
  // For this reference implementation, we simulate the PRBS output
  // to focus on the mapping logic.

  // We'll use a Linear Congruential Generator seeded with c_init for
  // determinism sufficient for educational mapping demonstration.
  uint64_t state = c_init;
  for (int i = 0; i < length; i++) {
    state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
    c[i] = (state >> 16) & 1;
  }
  return c;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: CORE IMPLEMENTATION - STAGE 9
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Stage 9: Maps physical channels to the SSB Grid.
 *
 * @param pbch_symbols Input 432 complex QPSK symbols from Stage 8
 * @param output_grid [out] The populated 4x240 grid
 * @return int 0 on success
 */
int stage9_re_mapping(const std::vector<ComplexSym> &pbch_symbols,
                      SSBGrid &grid) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 9: RESOURCE ELEMENT (RE) MAPPING                       ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Standard Reference: TS 38.211 7.4.3\n";
  std::cout << "Target Grid: " << SSB_SYMBOLS << " Symbols x " << SSB_SC
            << " Subcarriers\n\n";

  // Validate Input
  if (pbch_symbols.size() != NUM_PBCH_SYMBOLS) {
    std::cerr << "Error: Expected " << NUM_PBCH_SYMBOLS << " PBCH symbols, got "
              << pbch_symbols.size() << "\n";
    return -1;
  }

  // ─── Initialize Grid ───
  // 4 rows, 240 columns
  grid.assign(SSB_SYMBOLS,
              std::vector<ResourceElement>(SSB_SC, {{0, 0}, REType::EMPTY}));

  // DMRS Shift v0 = N_ID_CELL % 4
  int v0 = N_ID_CELL % 4;
  std::cout << "[INFO] Cell ID: " << N_ID_CELL << " → v0 = " << v0
            << " (DMRS Shift)\n";

  // ════════════════════════════════════════════════════════════
  // STEP 9.1: MAP PSS AND SSS (Fixed Locations)
  // ════════════════════════════════════════════════════════════
  std::cout << "[STEP 9.1] Mapping PSS (Sym 0) and SSS (Sym 2)...\n";

  // PSS: Symbol 0, k=56..182
  for (int k = PSS_SSS_START; k < PSS_SSS_START + PSS_SSS_WIDTH; k++) {
    grid[0][k].type = REType::PSS;
    grid[0][k].value = {1.0, 0.0}; // Placeholder magnitude
  }

  // SSS: Symbol 2, k=56..182
  for (int k = PSS_SSS_START; k < PSS_SSS_START + PSS_SSS_WIDTH; k++) {
    grid[2][k].type = REType::SSS;
    grid[2][k].value = {1.0, 0.0}; // Placeholder magnitude
  }

  // ════════════════════════════════════════════════════════════
  // STEP 9.2: GENERATE AND MAP PBCH DMRS
  // ════════════════════════════════════════════════════════════
  std::cout << "[STEP 9.2] Generating and Mapping PBCH DMRS...\n";
  std::cout << "  c_init = " << DMRS_C_INIT << "\n";

  // We need 144 DMRS QPSK symbols. Each needs 2 bits -> 288 bits.
  // However, the standard sequence runs over indices. We'll verify placement
  // first. Total REs for DMRS = 60 (Sym1) + 24 (Sym2) + 60 (Sym3) = 144.
  // Correct.

  std::vector<uint8_t> dmrs_bits =
      generate_gold_sequence(DMRS_C_INIT, 2 * NUM_DMRS_SYMBOLS);

  int dmrs_idx = 0;

  auto place_dmrs = [&](int symbol, int k) {
    if (dmrs_idx >= NUM_DMRS_SYMBOLS)
      return;

    // QPSK Modulation for DMRS: 1/sqrt(2) * (1-2c(2m)) ...
    double inv_sqrt2 = 0.70710678;
    double re = inv_sqrt2 * (1.0 - 2.0 * dmrs_bits[2 * dmrs_idx]);
    double im = inv_sqrt2 * (1.0 - 2.0 * dmrs_bits[2 * dmrs_idx + 1]);

    grid[symbol][k].type = REType::PBCH_DMRS;
    grid[symbol][k].value = {re, im};
    dmrs_idx++;
  };

  // Symbol 1: DMRS at 0+v0, 4+v0...
  for (int k = 0; k < SSB_SC; k++) {
    if ((k % 4) == v0)
      place_dmrs(1, k);
  }

  // Symbol 2: DMRS at 0..47 and 192..239
  // Note: DMRS exists where PBCH exists (excluding SSS gap)
  for (int k = 0; k < SSB_SC; k++) {
    // Check PBCH regions in Sym 2
    bool region_low = (k < 48);
    bool region_high = (k >= 192);

    if ((region_low || region_high) && (k % 4 == v0)) {
      place_dmrs(2, k);
    }
  }

  // Symbol 3: DMRS at 0+v0, 4+v0...
  for (int k = 0; k < SSB_SC; k++) {
    if ((k % 4) == v0)
      place_dmrs(3, k);
  }

  std::cout << "  ✓ Placed " << dmrs_idx << " DMRS Symbols.\n";

  // ════════════════════════════════════════════════════════════
  // STEP 9.3: MAP PBCH DATA
  // ════════════════════════════════════════════════════════════
  std::cout << "[STEP 9.3] Mapping 432 PBCH Data Symbols...\n";
  std::cout << "  Mapping Order: Symbol 1 → Symbol 2 → Symbol 3\n";

  int data_idx = 0;
  int mapped_sym1 = 0;
  int mapped_sym2 = 0;
  int mapped_sym3 = 0;

  // ─── Symbol 1 Mapping ───
  for (int k = 0; k < SSB_SC; k++) {
    if (grid[1][k].type == REType::EMPTY) {
      grid[1][k].type = REType::PBCH_DATA;
      grid[1][k].value = pbch_symbols[data_idx++];
      mapped_sym1++;
    }
  }

  // ─── Symbol 2 Mapping ───
  // According to Prompt/Math: exactly 72 REs needed here.
  // Regions: 0-47 (excl DMRS) + 192-239 (excl DMRS).
  // Note: Prompt mentions k=48-55 and k=183-191 but math (432 total)
  // strongly suggests only the main blocks are used to equal 72.
  // 0-47: 48 REs - 12 DMRS = 36.
  // 192-239: 48 REs - 12 DMRS = 36.
  // Total = 72. Perfect fit for remainder.

  for (int k = 0; k < SSB_SC; k++) {
    // Skip SSS
    if (grid[2][k].type == REType::SSS)
      continue;

    // Allowed PBCH Regions for this implementation
    bool valid_region = (k < 48) || (k >= 192);

    if (valid_region && grid[2][k].type == REType::EMPTY) {
      grid[2][k].type = REType::PBCH_DATA;
      grid[2][k].value = pbch_symbols[data_idx++];
      mapped_sym2++;
    }
  }

  // ─── Symbol 3 Mapping ───
  for (int k = 0; k < SSB_SC; k++) {
    if (grid[3][k].type == REType::EMPTY) {
      grid[3][k].type = REType::PBCH_DATA;
      grid[3][k].value = pbch_symbols[data_idx++];
      mapped_sym3++;
    }
  }

  std::cout << "  Symbol 1 PBCH REs: " << mapped_sym1 << " (Expected 180)\n";
  std::cout << "  Symbol 2 PBCH REs: " << mapped_sym2 << " (Expected 72)\n";
  std::cout << "  Symbol 3 PBCH REs: " << mapped_sym3 << " (Expected 180)\n";
  std::cout << "  Total Mapped:      " << data_idx << " / 432\n";

  if (data_idx != 432) {
    std::cout << "  ⚠️ WARNING: Mismatch in PBCH symbol count!\n";
  }

  std::cout << "\n✓ STAGE 9 COMPLETE\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// VISUALIZATION FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

void print_grid_segment(const SSBGrid &grid, int start_k, int end_k) {
  std::cout << "      ";
  for (int k = start_k; k < end_k; k++)
    printf(" k=%-3d ", k);
  std::cout << "\n";

  // [FIX] Loop to print divider line to avoid char overflow warning
  std::cout << "      ";
  for (int i = 0; i < (end_k - start_k) * 7; ++i) {
    std::cout << "─";
  }
  std::cout << "\n";

  for (int l = 0; l < SSB_SYMBOLS; l++) {
    std::cout << "Sym " << l << "│";
    for (int k = start_k; k < end_k; k++) {
      std::cout << " " << get_type_str(grid[l][k].type) << "  │";
    }
    std::cout << "\n";
  }
  std::cout << "\n";
}

void print_full_report(const SSBGrid &grid) {
  std::cout << "\n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";
  std::cout << "GRID VISUALIZATION REPORT\n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  std::cout << "LEGEND: [PSS]=Pri.Sync, [SSS]=Sec.Sync, [DAT]=PBCH Data, "
               "[DMR]=Ref.Signal, [ . ]=Empty\n\n";

  std::cout << "--- [LOWER EDGE: k=0 to 11] (PBCH & DMRS start) ---\n";
  print_grid_segment(grid, 0, 12);

  std::cout << "--- [LOWER MID: k=45 to 59] (Transition to PSS/SSS) ---\n";
  print_grid_segment(grid, 45, 60);

  std::cout << "--- [CENTER: k=115 to 125] (PSS/SSS Center) ---\n";
  print_grid_segment(grid, 115, 126);

  std::cout << "--- [UPPER MID: k=180 to 195] (End PSS/SSS, Resume PBCH) ---\n";
  print_grid_segment(grid, 180, 196);

  std::cout << "--- [UPPER EDGE: k=228 to 239] (End of Block) ---\n";
  print_grid_segment(grid, 228, 240);
}

// ═══════════════════════════════════════════════════════════════════════════
// DEMONSTRATION SCENARIOS
// ═══════════════════════════════════════════════════════════════════════════

void run_demonstration() {
  // 1. Generate Input Data (Simulating Output of Stage 8)
  // 432 QPSK Symbols. For demo, we use unit magnitude with rotating phase.
  std::vector<ComplexSym> input_symbols;
  input_symbols.reserve(NUM_PBCH_SYMBOLS);

  double phase_step = 2.0 * M_PI / NUM_PBCH_SYMBOLS;
  for (int i = 0; i < NUM_PBCH_SYMBOLS; i++) {
    // Generate rotating phasor for traceability
    double angle = i * phase_step;
    input_symbols.push_back({std::cos(angle), std::sin(angle)});
  }

  // 2. Prepare Grid
  SSBGrid ssb_grid;

  // 3. Execute Stage 9
  stage9_re_mapping(input_symbols, ssb_grid);

  // 4. Validate Specific REs (Self-Check)
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";
  std::cout << "VERIFICATION CHECKS\n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  struct Check {
    int l;
    int k;
    REType expected;
    std::string desc;
  };
  std::vector<Check> checks = {
      {0, 0, REType::EMPTY, "Sym0 Guard Band (Low)"},
      {0, 56, REType::PSS, "Sym0 PSS Start"},
      {1, 0, REType::PBCH_DMRS, "Sym1 DMRS (v0=0)"},
      {1, 1, REType::PBCH_DATA, "Sym1 PBCH Data"},
      {1, 4, REType::PBCH_DMRS, "Sym1 DMRS (v0+4)"},
      {2, 0, REType::PBCH_DMRS, "Sym2 DMRS (Low Region)"},
      {2, 56, REType::SSS, "Sym2 SSS Start"},
      {2, 48, REType::EMPTY, "Sym2 Gap (48-55) [Unused in this config]"},
      {3, 239, REType::PBCH_DATA, "Sym3 Last SC"}};

  for (const auto &c : checks) {
    REType actual = ssb_grid[c.l][c.k].type;
    std::cout << "Check RE(l=" << c.l << ", k=" << std::setw(3) << c.k << "): "
              << "Expected [" << get_type_str(c.expected) << "] "
              << "Actual [" << get_type_str(actual) << "] "
              << "→ " << (actual == c.expected ? "PASS ✓" : "FAIL ✗") << " ("
              << c.desc << ")\n";
  }

  // 5. Visualize
  print_full_report(ssb_grid);
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  try {
    run_demonstration();
  } catch (const std::exception &e) {
    std::cerr << "CRITICAL ERROR: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
