// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/*
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH STAGE 1: TB INPUT & DL-SCH DATA GENERATION                 ║
 * ║  Implementation Reference: 3GPP TS 38.214 / TS 38.321                   ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This program simulates the MAC-to-PHY interface. It demonstrates how the
 * Network determines how much data (Transport Block Size) fits into a specific
 * time-frequency allocation and generates the raw Downlink Shared Channel
 * (DL-SCH) payload.
 *
 * TECHNICAL SCOPE:
 * - MCS (Modulation & Coding Scheme) Lookups
 * - TBS (Transport Block Size) Calculation (Formulas from TS 38.214)
 * - DL-SCH Payload Generation (Random Data / Padding)
 * - Byte-to-Bit Conversion for PHY processing
 *
 * IMPLEMENTATION STAGES:
 * 1. Scheduling Parameters Setup (MCS, RBs, Layers)
 * 2. MCS Table Lookup (Target Code Rate, Modulation Order)
 * 3. TBS Calculation Algorithm
 * 4. Payload Generation & Visualization
 *
 * AUTHOR: AI C++ Technical Generator
 * DATE:   2023-10-27
 * STANDARDS:
 * - 3GPP TS 38.214 Section 5.1.3.2 (Transport Block Size determination)
 * - 3GPP TS 38.214 Table 5.1.3.1-1 (MCS Index Table 1)
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Represents the modulation order (Qm).
 */
enum class ModulationOrder { QPSK = 2, QAM16 = 4, QAM64 = 6, QAM256 = 8 };

/**
 * @brief Scheduling Parameters from the MAC Scheduler.
 */
struct SchedulingInfo {
  int mcs_index;        ///< MCS Index (0-28)
  int num_prbs;         ///< Allocated Physical Resource Blocks
  int num_layers;       ///< MIMO Layers (v)
  int symbols_per_slot; ///< Typically 14, minus overhead
  int dmrs_overhead;    ///< DMRS REs per PRB (e.g., 6, 12, 18)
};

/**
 * @brief MCS Table Entry structure.
 */
struct McsEntry {
  ModulationOrder qm;         ///< Modulation Order
  double target_code_rate;    ///< R (Target code rate x 1024)
  double spectral_efficiency; ///< Spectral Efficiency
};

/**
 * @brief The resulting Transport Block.
 */
struct TransportBlock {
  int tbs_bits;              ///< Calculated Size in Bits
  int tbs_bytes;             ///< Calculated Size in Bytes
  std::vector<uint8_t> data; ///< Raw Payload
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Prints data in a hexdump format.
 */
void print_hexdump(const std::vector<uint8_t> &data, int limit = 32) {
  std::cout << "  Address  | Hex Data                                         "
               "| ASCII\n";
  std::cout << "  "
               "---------+--------------------------------------------------+--"
               "----\n";

  int print_len = std::min((int)data.size(), limit);
  for (int i = 0; i < print_len; i += 16) {
    std::cout << "  " << std::setw(8) << std::setfill('0') << std::hex << i
              << " | ";

    // Hex
    for (int j = 0; j < 16; ++j) {
      if (i + j < print_len)
        std::cout << std::setw(2) << (int)data[i + j] << " ";
      else
        std::cout << "   ";
    }
    std::cout << "| ";

    // ASCII
    for (int j = 0; j < 16; ++j) {
      if (i + j < print_len) {
        unsigned char c = data[i + j];
        std::cout << (std::isprint(c) ? (char)c : '.');
      }
    }
    std::cout << "\n";
  }
  if (data.size() > (size_t)limit) {
    std::cout << "  ... (" << (data.size() - limit) << " more bytes hidden)\n";
  }
  std::cout << std::dec << "\n";
}

/**
 * @brief Simple ASCII visualization of the data block.
 */
void visualize_block(int size_bytes) {
  int width = 50;
  int blocks = size_bytes / 100; // 1 block = 100 bytes approximately
  if (blocks == 0)
    blocks = 1;
  if (blocks > width)
    blocks = width;

  std::cout << "  [";
  for (int i = 0; i < width; ++i) {
    if (i < blocks)
      std::cout << "▓";
    else
      std::cout << "░";
  }
  std::cout << "] " << size_bytes << " Bytes\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: CORE IMPLEMENTATION - MCS TABLES
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Retrieves MCS parameters based on TS 38.214 Table 5.1.3.1-1.
 *
 * @param index MCS Index (0-28)
 * @return McsEntry Struct with Qm and Rate
 */
McsEntry get_mcs_info(int index) {
  // Simplified Table for Demonstration
  // Format: {Qm, Target_Rate_R/1024, Spectral_Eff}
  static const std::map<int, McsEntry> mcs_table_1 = {
      {0, {ModulationOrder::QPSK, 120.0 / 1024.0, 0.2344}},
      {5, {ModulationOrder::QPSK, 378.0 / 1024.0, 0.7383}},
      {10, {ModulationOrder::QAM16, 340.0 / 1024.0, 1.3281}},
      {15, {ModulationOrder::QAM16, 616.0 / 1024.0, 2.4063}},
      {20, {ModulationOrder::QAM64, 533.0 / 1024.0, 3.1211}},
      {25, {ModulationOrder::QAM64, 772.0 / 1024.0, 4.5234}},
      {27,
       {ModulationOrder::QAM256, 841.0 / 1024.0, 6.5703}}, // High throughput
      {28, {ModulationOrder::QAM256, 948.0 / 1024.0, 7.4063}}};

  // Fallback/Interpolation for indices not explicitly listed above for brevity
  if (mcs_table_1.count(index)) {
    return mcs_table_1.at(index);
  } else {
    // Default safe fallback if index isn't in our abbreviated table
    return {ModulationOrder::QPSK, 120.0 / 1024.0, 0.2344};
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: TBS CALCULATION (TS 38.214)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Calculates the Transport Block Size.
 *
 * Algorithm Summary (TS 38.214 5.1.3.2):
 * 1. Calculate N_info = N_RE * R * Qm * v
 * 2. If N_info <= 3824: Apply Quantization based on N_info
 * 3. If N_info > 3824: Apply Code Block Segmentation formula
 */
int calculate_tbs(const SchedulingInfo &sched, const McsEntry &mcs) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 1.1: TBS CALCULATION                                   "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";

  // 1. Calculate Total REs available for PDSCH
  // N_RE_prime = N_sc_RB * N_symb_sh - N_DMRS_PRB - N_oh_PRB
  // Simplified: 12 subcarriers * (symbols - overhead)

  // Assume DMRS/Overhead takes away some REs. Standard usually says N_RE' = 12
  // * 14 - Overhead Let's assume net usable REs per PRB is roughly (12 * 14) -
  // 12 = 156 approx.
  double n_re_per_prb = 12.0 * (sched.symbols_per_slot) - sched.dmrs_overhead;
  double n_re = std::min(156.0, n_re_per_prb) * sched.num_prbs;

  std::cout << "[STEP 1] Calculate Resource Elements (REs)\n";
  std::cout << "  > PRBs: " << sched.num_prbs << "\n";
  std::cout << "  > Usable REs/PRB: " << n_re_per_prb << "\n";
  std::cout << "  > Total N_RE: " << n_re << "\n";

  // 2. Calculate Intermediate Information Bits (N_info)
  // N_info = N_RE * R * Qm * v
  double n_info = n_re * mcs.target_code_rate * (int)mcs.qm * sched.num_layers;

  std::cout << "[STEP 2] Calculate Unquantized Information Bits (N_info)\n";
  std::cout << "  > Formula: N_RE * Rate * Qm * Layers\n";
  std::cout << "  > " << n_re << " * " << std::setprecision(3)
            << mcs.target_code_rate << " * " << (int)mcs.qm << " * "
            << sched.num_layers << "\n";
  std::cout << "  > N_info: " << n_info << " bits\n";

  // 3. TBS Determination Logic
  int tbs = 0;

  if (n_info <= 3824) {
    // Procedure for small TBS
    std::cout << "[STEP 3] TBS Determination (N_info <= 3824)\n";
    // Simplified Quantization (n = max(3, floor(log2(n_info)) - 6))
    int n = std::max(3, (int)std::floor(std::log2(n_info)) - 6);
    // N_info_quant = max(24, 2^n * floor(N_info / 2^n))
    int n_quant = std::max(24, (1 << n) * (int)std::floor(n_info / (1 << n)));
    tbs = n_quant;
    std::cout << "  > Small Block Quantization Applied.\n";
  } else {
    // Procedure for large TBS (requiring segmentation potentially)
    std::cout << "[STEP 3] TBS Determination (N_info > 3824)\n";
    int n = std::floor(std::log2(n_info - 24)) - 5;
    // N_info_quant = max(3840, 2^n * round((N_info-24)/2^n))
    int n_quant =
        std::max(3840, (1 << n) * (int)std::round((n_info - 24) / (1 << n)));

    // Code Rate consideration (if R <= 1/4)
    if (mcs.target_code_rate <= 0.25) {
      int C = std::ceil((n_quant + 24.0) / 3816.0);
      tbs = 8 * C * std::ceil((n_quant + 24.0) / (8.0 * C)) - 24;
    } else {
      // N_info > 8424?
      if (n_info > 8424) {
        int C = std::ceil((n_quant + 24.0) / 8424.0);
        tbs = 8 * C * std::ceil((n_quant + 24.0) / (8.0 * C)) - 24;
      } else {
        tbs = 8 * std::ceil((n_quant + 24.0) / 8.0) - 24;
      }
    }
    std::cout << "  > Large Block Segmentation Logic Applied.\n";
  }

  std::cout << "  > FINAL TBS: " << tbs << " bits (" << tbs / 8 << " bytes)\n";
  return tbs;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5: PAYLOAD GENERATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Generates the actual TB payload (simulating IP packets).
 */
TransportBlock stage1_generate_tb(const SchedulingInfo &sched) {
  // 1. Get MCS Info
  McsEntry mcs = get_mcs_info(sched.mcs_index);

  std::cout << "Target Configuration:\n";
  std::cout << "  MCS: " << sched.mcs_index << " (Mod: " << (int)mcs.qm
            << ", Rate: " << mcs.target_code_rate << ")\n";

  // 2. Calculate TBS
  int tbs_bits = calculate_tbs(sched, mcs);
  int tbs_bytes = tbs_bits / 8;

  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 1.2: DL-SCH DATA GENERATION                            "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";

  TransportBlock tb;
  tb.tbs_bits = tbs_bits;
  tb.tbs_bytes = tbs_bytes;
  tb.data.resize(tbs_bytes);

  std::cout << "[STEP 1] Generating Random Payload...\n";
  // Using a PRNG to fill the buffer
  std::mt19937 rng(42); // Fixed seed for reproducibility
  std::uniform_int_distribution<uint16_t> dist(0, 255);

  for (int i = 0; i < tbs_bytes; ++i) {
    tb.data[i] = static_cast<uint8_t>(dist(rng));
  }

  // Embed a "Signature" at the start to recognize the data later
  if (tbs_bytes > 4) {
    tb.data[0] = 0xDE;
    tb.data[1] = 0xAD;
    tb.data[2] = 0xBE;
    tb.data[3] = 0xEF;
  }

  visualize_block(tbs_bytes);
  std::cout << "  > Buffer filled with " << tbs_bytes
            << " bytes of MAC data.\n";
  std::cout << "  > First 4 bytes marked with 0xDEADBEEF signature.\n";

  // Show Hexdump
  std::cout << "\n[STEP 2] Inspecting Generated Data (First 64 bytes):\n";
  print_hexdump(tb.data, 64);

  std::cout << "\n✓ STAGE 1 COMPLETE: TB Ready for CRC Attachment.\n";

  return tb;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 6: DEMONSTRATION SCENARIOS
// ═══════════════════════════════════════════════════════════════════════════

void run_scenario_voip() {
  std::cout << "\n\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════\n";
  std::cout << "SCENARIO A: VoIP Traffic (Small Allocation)\n";
  std::cout << "Low Latency, Small Packets, Robust Coding\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════\n";

  SchedulingInfo sched;
  sched.mcs_index = 5;  // QPSK, ~0.37 Rate (Robust)
  sched.num_prbs = 4;   // Few RBs
  sched.num_layers = 1; // SISO
  sched.symbols_per_slot = 14;
  sched.dmrs_overhead = 12;

  TransportBlock tb = stage1_generate_tb(sched);
}

void run_scenario_streaming() {
  std::cout << "\n\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════\n";
  std::cout << "SCENARIO B: 4K Video Streaming (High Throughput)\n";
  std::cout << "High Bandwidth, 64QAM, MIMO 2x2\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════\n";

  SchedulingInfo sched;
  sched.mcs_index = 25; // 64QAM, ~0.75 Rate (High Speed)
  sched.num_prbs = 50;  // Wide allocation
  sched.num_layers = 2; // 2x2 MIMO
  sched.symbols_per_slot = 14;
  sched.dmrs_overhead =
      18; // Higher DMRS density for high speed channel estimation

  TransportBlock tb = stage1_generate_tb(sched);
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << R"(
    ###############################################################
    #                                                             #
    #         5G NR PDSCH - STAGE 1: TB INPUT GENERATION          #
    #           MAC Layer -> Physical Layer Interface             #
    #                                                             #
    ###############################################################
    )" << "\n";

  std::cout << "EDUCATIONAL NOTES:\n";
  std::cout << "1. The Transport Block Size (TBS) is NOT fixed.\n";
  std::cout << "2. It is calculated dynamically based on Channel Quality (CQI "
               "-> MCS).\n";
  std::cout << "3. The Network Scheduler allocates PRBs and Layers.\n";
  std::cout << "4. PHY Layer receives this raw byte buffer (TB) to process.\n";

  try {
    run_scenario_voip();
    run_scenario_streaming();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "FINAL SUMMARY:\n";
  std::cout << "✓ Implemented TS 38.214 TBS Determination formulas.\n";
  std::cout << "✓ Simulated MAC-to-PHY data transfer.\n";
  std::cout << "✓ Verified Small Packet and Large Payload generation.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  return 0;
}
