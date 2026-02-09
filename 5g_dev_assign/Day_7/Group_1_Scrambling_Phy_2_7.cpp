// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  TITLE:       5G NR PBCH: BIT-LEVEL SCRAMBLING (SCRAMBLING #2)          ║
 * ║  AUTHOR:      AI Technical Documentation Generator                      ║
 * ║  DATE:        2023-10-27                                                ║
 * ║  STANDARD:    3GPP TS 38.211 Section 5.2.1 and 7.3.3.1                  ║
 * ║                                                                         ║
 * ║  DESCRIPTION:                                                           ║
 * ║  Implements Stage 7 of the PBCH chain: The second scrambling stage.     ║
 * ║  This stage applies a cell-specific scrambling sequence to the          ║
 * ║  encoded/interleaved bits before modulation.                            ║
 * ║                                                                         ║
 * ║  CRITICAL MECHANISM:                                                    ║
 * ║  - Uses Gold Sequence c(n) of length 31.                                ║
 * ║  - Initialization c_init = CellID.                                      ║
 * ║  - Time-varying offset v determines which part of the sequence is used. ║
 * ║  - v = SSB_Index % 4 (for L_max=4).                                     ║
 * ║                                                                         ║
 * ║  INPUT SCENARIO:                                                        ║
 * ║  - Cell ID: 276                                                         ║
 * ║  - SSB Index: 3 (implies v=3)                                           ║
 * ║  - M_bit: 864                                                           ║
 * ║  - Offset: 3 * 864 = 2592                                               ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════
#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

// ─── Constants ───
const int SCRAM_M_BIT = 864; // Total PBCH bits per transmission
const int SCRAM_NC = 1600;   // 3GPP Gold Sequence Offset
const int LFSR_LEN = 31;     // Length of Gold Sequence registers

// ─── Configuration Struct ───
struct ScramblingConfig {
  uint16_t cell_id;
  uint8_t ssb_index;
  uint8_t l_max; // 4, 8, or 64
};

using BitVector = std::vector<uint8_t>;

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Prints a sequence of bits with formatted indices.
 */
void print_bits_labeled(const std::string &label, const BitVector &bits,
                        size_t limit = 16) {
  std::cout << "  " << std::left << std::setw(15) << label << " │ ";
  for (size_t i = 0; i < bits.size(); ++i) {
    if (i >= limit) {
      std::cout << "...";
      break;
    }
    std::cout << (int)bits[i] << " ";
  }
  std::cout << "\n";
}

/**
 * @brief 3GPP Gold Sequence Generator (TS 38.211 §5.2.1)
 *
 * Generates pseudo-random sequence c(n).
 *
 * @param c_init Initialization value (31 bits)
 * @param output_len Number of bits to return
 * @param skip_offset Number of bits to skip (Nc + v*M) before recording output
 *
 * @return BitVector The Gold sequence
 */
BitVector generate_gold_sequence(uint32_t c_init, size_t output_len,
                                 size_t skip_offset) {
  BitVector output;
  output.reserve(output_len);

  // ─── Initialize LFSRs ───
  // x1 init: x1(0)=1, all others 0
  std::vector<uint8_t> x1(LFSR_LEN, 0);
  x1[0] = 1;

  // x2 init: c_init (LSB at index 0)
  // x2(i) = (c_init >> i) & 1
  std::vector<uint8_t> x2(LFSR_LEN, 0);
  for (int i = 0; i < LFSR_LEN; i++) {
    x2[i] = (c_init >> i) & 1;
  }

  // Total cycles to run = skip_offset + output_len
  size_t total_cycles = skip_offset + output_len;

  // ─── Run LFSRs ───
  for (size_t n = 0; n < total_cycles; n++) {
    // 1. Calculate Outputs (before shift)
    // c(n) = (x1(n+Nc) + x2(n+Nc)) mod 2
    // In our loop, n is absolute time from 0.
    // We only save if n >= skip_offset

    if (n >= skip_offset) {
      uint8_t cn = (x1[0] + x2[0]) % 2;
      output.push_back(cn);
    }

    // 2. Calculate Feedback
    // x1 poly: D^31 + D^3 + 1 -> Taps at indices 0 and 3 (relative to current
    // output 0) Standard: x1(n+31) = (x1(n+3) + x1(n)) mod 2
    uint8_t next_x1 = (x1[3] + x1[0]) % 2;

    // x2 poly: D^31 + D^3 + D^2 + D + 1 -> Taps at 0, 1, 2, 3
    // Standard: x2(n+31) = (x2(n+3) + x2(n+2) + x2(n+1) + x2(n)) mod 2
    uint8_t next_x2 = (x2[3] + x2[2] + x2[1] + x2[0]) % 2;

    // 3. Shift Registers
    for (int i = 0; i < LFSR_LEN - 1; i++) {
      x1[i] = x1[i + 1];
      x2[i] = x2[i + 1];
    }
    x1[LFSR_LEN - 1] = next_x1;
    x2[LFSR_LEN - 1] = next_x2;
  }

  return output;
}

// ═══════════════════════════════════════════════════════════════════════════
// STAGE 7: SCRAMBLING #2
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Executes Stage 7: Bit-Level Scrambling
 *
 * Formula: b_tilde(i) = (b(i) + c(i + v*M_bit)) mod 2
 *
 * @param input_b The interleaved bits (864 bits)
 * @param cfg Configuration (CellID, SSB Index)
 * @return BitVector Scrambled bits b_tilde
 */
BitVector stage7_scrambling2(const BitVector &input_b,
                             const ScramblingConfig &cfg) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 7: BIT-LEVEL SCRAMBLING (Scrambling #2)               ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Standard Reference: TS 38.211 Section 7.3.3.1\n\n";

  if (input_b.size() != SCRAM_M_BIT) {
    std::cerr << "Error: Input size " << input_b.size()
              << " != M_bit=" << SCRAM_M_BIT << "\n";
    return BitVector();
  }

  // ─── Step 7.1: Calculate Parameters ───
  // v = SSB_index mod 4 (assuming L_max <= 8, technically determined by bits)
  // For L_max=4, it is just SSB_index.
  int v = cfg.ssb_index % 4;

  // Total offset = Nc + (v * M_bit)
  // Nc = 1600
  size_t sequence_offset = SCRAM_NC + (v * SCRAM_M_BIT);

  std::cout << "[STEP 7.1] Parameter Calculation\n";
  std::cout << "  c_init (N_ID^cell) = " << cfg.cell_id << "\n";
  std::cout << "  v (SSB mod 4)      = " << v << "\n";
  std::cout << "  M_bit              = " << SCRAM_M_BIT << "\n";
  std::cout << "  Sequence Offset    = v * M_bit = " << (v * SCRAM_M_BIT)
            << "\n";
  std::cout << "  Gold Seq Start     = 1600 (Nc) + " << (v * SCRAM_M_BIT)
            << " = " << sequence_offset << "\n";

  // ─── Step 7.2: Generate Gold Sequence ───
  std::cout << "\n[STEP 7.2] Generating Gold Sequence c(n)\n";

  // Visualization of Initialization
  std::cout << "  Initializing x2 with c_init = " << cfg.cell_id << " ("
            << std::bitset<10>(cfg.cell_id) << ")\n";
  std::cout << "  Advancing LFSRs by " << sequence_offset << " steps...\n";

  BitVector c_seq =
      generate_gold_sequence(cfg.cell_id, SCRAM_M_BIT, sequence_offset);

  print_bits_labeled("Sequence c(n)", c_seq, 16);

  // ─── Step 7.3: XOR Operation ───
  std::cout
      << "\n[STEP 7.3] Applying XOR: b_tilde(i) = b(i) XOR c(i + offset)\n";

  BitVector b_tilde;
  b_tilde.reserve(SCRAM_M_BIT);

  std::cout << "  Index │ b(i) (Input) │ c(i+offset)   │ b_tilde(i)\n";
  std::cout << "  ──────┼──────────────┼───────────────┼───────────\n";

  for (size_t i = 0; i < SCRAM_M_BIT; ++i) {
    uint8_t res = (input_b[i] + c_seq[i]) % 2;
    b_tilde.push_back(res);

    if (i < 16) {
      std::cout << "  " << std::setw(5) << i << " │ " << std::setw(12)
                << (int)input_b[i] << " │ " << std::setw(13) << (int)c_seq[i]
                << " │ " << (int)res << "\n";
    }
  }
  std::cout << "  ...   │ ...          │ ...           │ ...\n";

  std::cout << "\n✓ STAGE 7 COMPLETE: " << b_tilde.size()
            << " scrambled bits generated.\n";
  return b_tilde;
}

// ═══════════════════════════════════════════════════════════════════════════
// DEMONSTRATION SCENARIO
// ═══════════════════════════════════════════════════════════════════════════

void run_demonstration() {
  // Setup Configuration matching the prompt
  ScramblingConfig cfg;
  cfg.cell_id = 276;
  cfg.ssb_index = 3; // v = 3
  cfg.l_max = 4;

  // Construct the specific input vector b(i) from the prompt
  // "0 1 1 0 1 0 0 1 1 0 1 1 0 0 1 0"
  std::vector<int> prefix = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0};

  BitVector input_b(SCRAM_M_BIT, 0); // Initialize with zeros

  // Copy prefix to input
  for (size_t i = 0; i < prefix.size(); i++) {
    input_b[i] = prefix[i];
  }

  std::cout
      << "═══════════════════════════════════════════════════════════════\n";
  std::cout
      << "      5G NR PBCH SCRAMBLING #2 DEMONSTRATION                   \n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  // Execute Stage
  BitVector output = stage7_scrambling2(input_b, cfg);

  // ─── Verification ───
  std::cout
      << "\n═══════════════════════════════════════════════════════════════\n";
  std::cout << "VERIFICATION (First 4 bits)\n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  bool all_match = true;
  // Expected output from prompt: 1 0 1 1
  std::vector<int> expected = {1, 0, 1, 1};

  for (int i = 0; i < 4; i++) {
    std::cout << "  Bit " << i << ": Input=" << prefix[i] << " XOR Gold="
              << ((prefix[i] + output[i]) % 2) // Recover Gold bit for display
              << " -> Output=" << (int)output[i] << " [Expected " << expected[i]
              << "] ";

    if (output[i] == expected[i]) {
      std::cout << "✓\n";
    } else {
      std::cout << "✗\n";
      all_match = false;
    }
  }

  if (all_match) {
    std::cout
        << "\n  SUCCESS: Calculated output matches theoretical verification.\n";
  } else {
    std::cout << "\n  WARNING: Discrepancy detected.\n";
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  try {
    run_demonstration();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
