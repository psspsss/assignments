// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/*
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR CORESET RESOURCE GRID GENERATOR & VISUALIZER                     ║
 * ║  Implementation Reference: 3GPP TS 38.211 / TS 38.213                   ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This program simulates the Physical Downlink Control Channel (PDCCH)
 * mapping within a Control Resource Set (CORESET). It demonstrates the
 * translation of logical Control Channel Elements (CCEs) into physical
 * Resource Element Groups (REGs) and finally to individual Resource
 * Elements (REs) on the time-frequency grid.
 *
 * TECHNICAL SCOPE:
 * - Resource Grid Construction (Subcarriers x Symbols)
 * - CCE to REG Mapping (Non-interleaved mode)
 * - DMRS (Demodulation Reference Signal) Pattern Generation
 * - ASCII-based Spectral Visualization
 *
 * IMPLEMENTATION STAGES:
 * 1. Configuration & Grid Initialization
 * 2. CCE Allocation & Hashing Simulation
 * 3. DMRS & Data Insertion (Physical Layer Mapping)
 * 4. Resource Grid Visualization
 *
 * AUTHOR: AI C++ Technical Generator
 * DATE:   2023-10-27
 * STANDARDS: 3GPP TS 38.211 Section 7.3.2 (PDCCH)
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Represents the physical content type of a Resource Element (RE).
 */
enum class REType {
  EMPTY,      ///< Unused resource element (or PDSCH area outside CORESET)
  PDCCH_DATA, ///< Control channel payload data
  DMRS_PILOT, ///< Demodulation Reference Signal (Pilot)
  GUARD       ///< Guard bands (if applicable)
};

/**
 * @brief Configuration parameters for the CORESET.
 * References TS 38.331 ControlResourceSet IE.
 */
struct CoresetConfig {
  int frequency_domain_rbs; ///< Total RBs in frequency domain (e.g., 48)
  int duration_symbols;     ///< Time duration in symbols (1, 2, or 3)
  int cce_count;            ///< Number of CCEs to allocate
  int reg_bundle_size;      ///< Size of REG bundle (L) typically 6 for
                            ///< non-interleaved
  bool interleaved;         ///< True if interleaved mapping is used
  int shift_index;          ///< Simulates the hashing result offset
};

/**
 * @brief Represents a single Resource Element (1 Subcarrier x 1 Symbol).
 */
struct ResourceElement {
  REType type;
  double power_scale; ///< Simulation of power allocation (0.0 - 1.0)

  ResourceElement() : type(REType::EMPTY), power_scale(0.0) {}
};

/**
 * @brief Represents a Resource Block (RB) consisting of 12 subcarriers.
 */
struct ResourceBlock {
  int rb_index;
  // Map: [Symbol Index] -> [Vector of 12 Subcarriers]
  std::map<int, std::vector<ResourceElement>> symbols;

  ResourceBlock(int idx) : rb_index(idx) {}
};

// CONSTANTS
const int SC_PER_RB = 12;   ///< Subcarriers per Resource Block
const int REGS_PER_CCE = 6; ///< Resource Element Groups per CCE
const std::vector<int> DMRS_OFFSETS = {
    1, 5, 9}; ///< DMRS subcarrier positions (0-indexed)

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Returns a UTF-8 character representation of the RE Type.
 *
 * @param type The Resource Element Type
 * @return std::string ASCII/Unicode block character
 */
std::string get_re_symbol(REType type) {
  switch (type) {
  case REType::PDCCH_DATA:
    return "▪"; // Dense square for Data
  case REType::DMRS_PILOT:
    return "♦"; // Diamond for Pilot
  case REType::EMPTY:
    return " "; // Empty space
  default:
    return "?";
  }
}

/**
 * @brief Returns a color code/string description for logging.
 */
std::string get_type_name(REType type) {
  switch (type) {
  case REType::PDCCH_DATA:
    return "PDCCH (Data)";
  case REType::DMRS_PILOT:
    return "DMRS (Pilot)";
  case REType::EMPTY:
    return "Unused/PDSCH";
  default:
    return "Unknown";
  }
}

/**
 * @brief Prints a horizontal separator line for console output.
 */
void print_separator() {
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
}

/**
 * @brief Header helper for major stages.
 */
void print_stage_header(int stage_num, const std::string &title) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE " << stage_num << ": " << std::left << std::setw(58)
            << title << "║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: STAGE 1 - INITIALIZATION & CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Initializes the Resource Grid structure based on configuration.
 *
 * @param config The CORESET configuration
 * @param grid [out] The vector of ResourceBlocks to be initialized
 * @return 0 on success, -1 on invalid config
 */
int stage1_init_grid(const CoresetConfig &config,
                     std::vector<ResourceBlock> &grid) {
  print_stage_header(1, "GRID INITIALIZATION & VALIDATION");

  // Step 1.1: Validation
  std::cout << "[STEP 1.1] Validating Configuration Inputs...\n";
  std::cout << "  > RBs: " << config.frequency_domain_rbs << "\n";
  std::cout << "  > Symbols: " << config.duration_symbols << "\n";
  std::cout << "  > CCEs Requested: " << config.cce_count << "\n";

  if (config.frequency_domain_rbs <= 0 || config.duration_symbols <= 0) {
    std::cerr << "  [!] Error: Invalid grid dimensions.\n";
    return -1;
  }

  // Calculate total REG capacity
  // Total REGs = RBs * Symbols
  int total_capacity_regs =
      config.frequency_domain_rbs * config.duration_symbols;
  int required_regs = config.cce_count * REGS_PER_CCE;

  std::cout << "  > Total Capacity (REGs): " << total_capacity_regs << "\n";
  std::cout << "  > Required (REGs):       " << required_regs << "\n";

  if (required_regs > total_capacity_regs) {
    std::cerr << "  [!] Error: Required CCEs exceed CORESET capacity.\n";
    return -1;
  }

  // Step 1.2: Memory Allocation
  std::cout << "\n[STEP 1.2] Allocating Memory for Resource Blocks...\n";
  grid.clear();
  for (int i = 0; i < config.frequency_domain_rbs; ++i) {
    ResourceBlock rb(i);
    // Initialize symbols within RB
    for (int sym = 0; sym < config.duration_symbols; ++sym) {
      rb.symbols[sym] = std::vector<ResourceElement>(SC_PER_RB);
    }
    grid.push_back(rb);
  }

  std::cout << "  > Allocated " << grid.size() << " Resource Block objects.\n";
  std::cout << "  > Initialized " << SC_PER_RB << " subcarriers per symbol.\n";

  std::cout << "\n✓ STAGE 1 COMPLETE: Grid Ready.\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: STAGE 2 - LOGICAL CCE MAPPING (HASHING)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Determines which REGs are occupied based on CCE index and Hashing.
 *
 * In a real scenario, this involves the hashing function:
 * L_CCE = (L * Y_p,n_s,f + m) mod (N_CCE / L) ... [TS 38.213]
 * Here we simulate the result using 'shift_index' as the start position.
 *
 * @param config Input configuration
 * @param allocated_regs [out] List of pair<RB_Index, Symbol_Index>
 */
void stage2_calculate_mapping(
    const CoresetConfig &config,
    std::vector<std::pair<int, int>> &allocated_regs) {
  print_stage_header(2, "CCE TO REG MAPPING CALCULATION");

  std::cout << "Standard Reference: TS 38.211 Sec 7.3.2.2 (REG Mapping)\n\n";

  // Step 2.1: Calculate Logical Start
  std::cout << "[STEP 2.1] Determining Start Index (Simulating Hash)...\n";
  int total_regs_needed = config.cce_count * REGS_PER_CCE;

  // In non-interleaved, REGs are mapped sequentially Time-First or
  // Frequency-First. Standard CORESET REG bundle mapping is time-first, then
  // frequency-first BUT for non-interleaved (Interleaver size L=6), a CCE
  // corresponds to 6 REGs.

  std::cout << "  > Mode: Non-Interleaved (Contiguous Mapping)\n";
  std::cout << "  > Start Offset (from Hash): " << config.shift_index
            << " REGs\n";

  // Step 2.2: Generate List of Active REGs
  std::cout << "\n[STEP 2.2] Generating REG Coordinate List...\n";
  std::cout << "  Mapping Strategy: Frequency-First filling for simplicity in "
               "this visualizer\n";
  std::cout << "  (Note: Real HW mapping depends on 'cce-REG-MappingType')\n\n";

  int current_reg_count = 0;
  int reg_global_idx = 0;

  // Iterate through grid to find start and end positions
  // We treat the grid as a linear array of REGs for mapping purposes
  // Order: RB 0 Sym 0, RB 1 Sym 0... (Frequency First)

  // Visualize the mapping table
  std::cout << "  +-------+----------+--------+---------+\n";
  std::cout << "  | REG # | RB Index | Symbol | Status  |\n";
  std::cout << "  +-------+----------+--------+---------+\n";

  for (int rb = 0; rb < config.frequency_domain_rbs; ++rb) {
    for (int sym = 0; sym < config.duration_symbols; ++sym) {

      bool is_allocated = false;

      // Check if this global REG index falls within our allocated block
      if (reg_global_idx >= config.shift_index &&
          current_reg_count < total_regs_needed) {

        allocated_regs.push_back({rb, sym});
        is_allocated = true;
        current_reg_count++;
      }

      // Print first few and last few for brevity
      if (reg_global_idx < config.shift_index + 2 ||
          (is_allocated && current_reg_count < 3) ||
          (is_allocated && current_reg_count > total_regs_needed - 2)) {

        std::cout << "  | " << std::setw(5) << reg_global_idx << " | "
                  << std::setw(8) << rb << " | " << std::setw(6) << sym << " | "
                  << (is_allocated ? "ALLOCATED" : "SKIP   ") << " |\n";
      } else if (reg_global_idx == config.shift_index + 3) {
        std::cout << "  |  ...  |    ...   |   ...  |   ...   |\n";
      }

      reg_global_idx++;
    }
  }
  std::cout << "  +-------+----------+--------+---------+\n";

  std::cout << "\n✓ STAGE 2 COMPLETE: Mapped " << allocated_regs.size()
            << " REGs.\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5: STAGE 3 - PHYSICAL LAYER POPULATION (DMRS & DATA)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Fills the REs in the grid with actual Data and Pilots.
 */
void stage3_populate_grid(
    std::vector<ResourceBlock> &grid,
    const std::vector<std::pair<int, int>> &allocated_regs) {
  print_stage_header(3, "PHYSICAL LAYER POPULATION (DMRS insertion)");

  std::cout << "Standard Reference: TS 38.211 Sec 7.4.1.3 (DMRS for PDCCH)\n";
  std::cout << "Rule: DMRS inserted at k = {1, 5, 9} modulo 12 in every "
               "allocated REG.\n\n";

  int filled_regs = 0;

  for (const auto &reg_coord : allocated_regs) {
    int rb_idx = reg_coord.first;
    int sym_idx = reg_coord.second;

    // Access the specific Resource Block
    // Note: In production code, add bounds checking here
    ResourceBlock &rb = grid[rb_idx];
    std::vector<ResourceElement> &subcarriers = rb.symbols[sym_idx];

    // [STEP 3.1] Iterate Subcarriers k=0..11
    for (int k = 0; k < SC_PER_RB; ++k) {

      // Check if k is a DMRS position
      bool is_dmrs = false;
      for (int offset : DMRS_OFFSETS) {
        if (k == offset) {
          is_dmrs = true;
          break;
        }
      }

      if (is_dmrs) {
        subcarriers[k].type = REType::DMRS_PILOT;
        subcarriers[k].power_scale = 1.0; // Pilot usually boosted or unity
      } else {
        subcarriers[k].type = REType::PDCCH_DATA;
        subcarriers[k].power_scale = 0.8; // Data
      }
    }
    filled_regs++;
  }

  std::cout << "[STEP 3.2] Population Summary:\n";
  std::cout << "  > Processed " << filled_regs << " REGs.\n";
  std::cout << "  > DMRS Pattern Applied: k={1,5,9}\n";
  std::cout << "  > Remaining REs set to PDCCH Data.\n";

  std::cout << "\n✓ STAGE 3 COMPLETE: Grid Populated.\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 6: STAGE 4 - VISUALIZATION & OUTPUT
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Renders the Grid to console using ASCII art.
 * Focuses on a specific range of RBs to keep output manageable.
 */
void stage4_render_grid(const std::vector<ResourceBlock> &grid, int symbols,
                        int start_rb, int end_rb) {
  print_stage_header(4, "CORESET RESOURCE GRID VISUALIZATION");

  std::cout << "Legend:\n";
  std::cout << " " << get_re_symbol(REType::PDCCH_DATA) << " : PDCCH Data RE\n";
  std::cout << " " << get_re_symbol(REType::DMRS_PILOT)
            << " : DMRS Pilot RE (k=1,5,9)\n";
  std::cout << "   : Unused/PDSCH\n\n";

  std::cout << "Displaying RBs " << end_rb << " down to " << start_rb
            << " (Frequency Axis ↓)\n";

  // Draw Top Border
  std::cout << "      ╔";
  for (int s = 0; s < symbols; ++s) {
    std::cout << "══════════════════════════";
    if (s < symbols - 1)
      std::cout << "╦";
  }
  std::cout << "╗\n";

  // Header
  std::cout << "      ║";
  for (int s = 0; s < symbols; ++s) {
    std::cout << "      Symbol " << std::left << std::setw(2) << s
              << "          ";
    if (s < symbols - 1)
      std::cout << "║";
  }
  std::cout << "║\n";

  // Middle separator
  std::cout << "╔═════╬";
  for (int s = 0; s < symbols; ++s) {
    std::cout << "══════════════════════════";
    if (s < symbols - 1)
      std::cout << "╬";
  }
  std::cout << "╣\n";

  // Iterate RBs downwards (high freq to low freq usually, or index based)
  // We will print RB index descending as per standard grid plots
  for (int r = end_rb; r >= start_rb; --r) {
    if (r >= (int)grid.size())
      continue;

    const ResourceBlock &rb = grid[r];

    // We need to print lines for the RB content.
    // To make it look like a grid, we'll print the subcarriers horizontally?
    // No, standard is: RB is vertical block.
    // But for text console, printing 12 subcarriers vertically takes 12 lines
    // per RB. Let's condense: Print 1 line per RB, showing the content as a
    // string.

    std::cout << "║RB " << std::setw(2) << r << "║ ";

    for (int s = 0; s < symbols; ++s) {
      // Check if this symbol in this RB has data
      // We scan the 12 SCs
      std::string visualization = "";
      bool has_data = false;

      // Construct a visual representation.
      // Since we can't print 12 chars easily and keep alignment perfectly with
      // wide chars, We will check the content state.

      const auto &scs = rb.symbols.at(s);

      // Analyze the RB content
      int dmrs_count = 0;
      int data_count = 0;

      for (const auto &re : scs) {
        if (re.type == REType::DMRS_PILOT)
          dmrs_count++;
        if (re.type == REType::PDCCH_DATA)
          data_count++;
      }

      if (dmrs_count > 0 || data_count > 0) {
        // It's an active REG
        // Visual: [▪♦▪▪♦▪▪▪♦▪▪▪]
        std::cout << "[";
        for (const auto &re : scs) {
          std::cout << get_re_symbol(re.type);
        }
        std::cout << "] ";
      } else {
        // Empty REG
        std::cout << " . . . . . . . . . . . .  ";
      }

      if (s < symbols - 1)
        std::cout << "║ ";
    }
    std::cout << "║\n";
  }

  // Bottom Border
  std::cout << "╚═════╩";
  for (int s = 0; s < symbols; ++s) {
    std::cout << "══════════════════════════";
    if (s < symbols - 1)
      std::cout << "╩";
  }
  std::cout << "╝\n";

  std::cout << "\n✓ STAGE 4 COMPLETE: Visualization rendered.\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 7: DEMONSTRATION SCENARIOS
// ═══════════════════════════════════════════════════════════════════════════

void run_scenario_standard() {
  print_separator();
  std::cout << "SCENARIO A: STANDARD 4 CCE ALLOCATION (Start Index 0)\n";
  std::cout << "Description: Allocates 4 CCEs (24 REGs) starting from RB 0.\n";
  print_separator();

  CoresetConfig config;
  config.frequency_domain_rbs = 48; // 48 RBs
  config.duration_symbols = 2;      // 2 Symbols
  config.cce_count = 4;             // 4 CCEs requested
  config.shift_index = 0;           // Start at 0
  config.interleaved = false;

  // Execution Pipeline
  std::vector<ResourceBlock> grid;
  std::vector<std::pair<int, int>> allocated_regs;

  if (stage1_init_grid(config, grid) == 0) {
    stage2_calculate_mapping(config, allocated_regs);
    stage3_populate_grid(grid, allocated_regs);
    // Visualizing RBs 0 to 15 to see the activity
    stage4_render_grid(grid, config.duration_symbols, 0, 15);
  }
}

void run_scenario_offset() {
  print_separator();
  std::cout << "SCENARIO B: OFFSET ALLOCATION (Hashing Simulation)\n";
  std::cout
      << "Description: Simulates a specific User/SearchSpace hash mapping.\n";
  std::cout << "             Starting at REG index 10 (RB 5, Sym 0 approx).\n";
  print_separator();

  CoresetConfig config;
  config.frequency_domain_rbs = 48;
  config.duration_symbols = 2;
  config.cce_count = 2;    // Smaller allocation
  config.shift_index = 10; // Offset
  config.interleaved = false;

  std::vector<ResourceBlock> grid;
  std::vector<std::pair<int, int>> allocated_regs;

  if (stage1_init_grid(config, grid) == 0) {
    stage2_calculate_mapping(config, allocated_regs);
    stage3_populate_grid(grid, allocated_regs);
    // Visualizing relevant range
    stage4_render_grid(grid, config.duration_symbols, 0, 12);
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  // UTF-8 check for console (Windows specific setup might be needed in real
  // env) std::setlocale(LC_ALL, "en_US.UTF-8");

  std::cout << R"(
    ###############################################################
    #                                                             #
    #         5G NR CORESET TECHNICAL IMPLEMENTATION              #
    #           Based on 3GPP TS 38.211 / 38.213                  #
    #                                                             #
    ###############################################################
    )" << "\n";

  std::cout << "EDUCATIONAL NOTES:\n";
  std::cout << "1. CCE (Control Channel Element) = 6 REGs.\n";
  std::cout << "2. REG (Resource Element Group) = 1 RB x 1 Symbol.\n";
  std::cout << "3. This code simulates the 'Non-Interleaved' mapping mode.\n";
  std::cout << "4. DMRS positions are fixed relative to the RB start.\n\n";

  try {
    // Run Demonstrations
    run_scenario_standard();
    std::cout << "\n\n";
    run_scenario_offset();

  } catch (const std::exception &e) {
    std::cerr << "CRITICAL FAILURE: " << e.what() << "\n";
    return 1;
  }

  std::cout << "\n";
  print_separator();
  std::cout << "FINAL SUMMARY:\n";
  std::cout << "✓ Validated grid dimensioning (48 RBs x 2 Sym).\n";
  std::cout << "✓ Implemented logical-to-physical mapping logic.\n";
  std::cout << "✓ Generated DMRS pilots at subcarriers 1, 5, 9.\n";
  std::cout << "✓ Visualized bit/resource allocation patterns.\n";
  std::cout << "\n[!] SECURITY NOTE: In production 5G stacks, randomizing\n";
  std::cout << "    CCE location (Hashing) is critical to prevent collisions\n";
  std::cout << "    and Blind Decoding attacks.\n";
  print_separator();

  return 0;
}
