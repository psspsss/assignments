// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  TITLE:       5G NR PBCH: CHANNEL INTERLEAVING                          ║
 * ║  AUTHOR:      AI Technical Documentation Generator                      ║
 * ║  DATE:        2023-10-27                                                ║
 * ║  STANDARD:    3GPP TS 38.212 Section 7.1.1 (Table 7.1.5-1)              ║
 * ║                                                                         ║
 * ║  DESCRIPTION:                                                           ║
 * ║  Implements Stage 6 of the PBCH chain: Bit-level Interleaving.          ║
 * ║  The rate-matched bits are written row-by-row into a matrix, the        ║
 * ║  columns are permuted according to a fixed pattern T, and bits are      ║
 * ║  read out column-by-column. This breaks up burst errors.                ║
 * ║                                                                         ║
 * ║  ALGORITHM STEPS:                                                       ║
 * ║  1. Parameters: E=864, Cols=32, Rows=27.                                ║
 * ║  2. Write input bits row-by-row into 27x32 matrix.                      ║
 * ║  3. Permute columns based on Pattern T.                                 ║
 * ║  4. Read bits column-by-column to generate output.                      ║
 * ║                                                                         ║
 * ║  INPUT SCENARIO:                                                        ║
 * ║  - Input: 864 Rate-Matched bits (f)                                     ║
 * ║  - Output: 864 Interleaved bits (e_tilde)                               ║
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
const int INT_E_BITS = 864;  // Input/Output Length
const int INT_NUM_COLS = 32; // Fixed for PBCH
const int INT_NUM_ROWS = 27; // ceil(864 / 32)

using BitVector = std::vector<uint8_t>;

// ─── Interleaving Pattern T (Table 7.1.5-1) ───
const std::vector<int> PATTERN_T = {0,  1,  2,  4,  3,  5,  6,  7,  8,  16, 9,
                                    17, 10, 18, 11, 19, 12, 20, 13, 21, 14, 22,
                                    15, 23, 24, 25, 26, 28, 27, 29, 30, 31};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Prints a sequence of bits with formatting.
 */
void print_bits(const std::string &label, const BitVector &bits,
                size_t limit = 64) {
  std::cout << "  " << std::left << std::setw(25) << label << " │ ";
  for (size_t i = 0; i < bits.size(); ++i) {
    if (i >= limit) {
      std::cout << "... (" << bits.size() - i << " more)";
      break;
    }
    std::cout << (int)bits[i];
    if ((i + 1) % 8 == 0 && i != bits.size() - 1)
      std::cout << " ";
  }
  std::cout << "\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// STAGE 6: CHANNEL INTERLEAVING
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Executes Stage 6: Channel Interleaving
 *
 * @param f_bits Input 864 rate-matched bits
 * @return BitVector Output 864 interleaved bits
 */
BitVector stage6_channel_interleaving(const BitVector &f_bits) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 6: CHANNEL INTERLEAVING                               ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Standard Reference: TS 38.212 Table 7.1.5-1\n";
  std::cout << "Matrix: " << INT_NUM_ROWS << " rows x " << INT_NUM_COLS
            << " columns = " << (INT_NUM_ROWS * INT_NUM_COLS) << " bits\n\n";

  if (f_bits.size() != INT_E_BITS) {
    std::cerr << "Error: Input size " << f_bits.size() << " != E=" << INT_E_BITS
              << "\n";
    return BitVector();
  }

  // ─── Step 6.1: Write Row-by-Row ───
  // We simulate the matrix conceptually using a 2D approach for clarity
  // In optimized C++, one might calculate indices directly, but we allocate for
  // demonstration.

  std::vector<std::vector<uint8_t>> matrix(INT_NUM_ROWS,
                                           std::vector<uint8_t>(INT_NUM_COLS));

  std::cout << "[STEP 6.1] Writing bits Row-by-Row into Matrix\n";
  int bit_idx = 0;
  for (int r = 0; r < INT_NUM_ROWS; r++) {
    for (int c = 0; c < INT_NUM_COLS; c++) {
      if (bit_idx < (int)f_bits.size()) {
        matrix[r][c] = f_bits[bit_idx++];
      } else {
        matrix[r][c] =
            0; // Padding (NULL), theoretically shouldn't happen for 864
      }
    }
  }

  // Visualize Matrix Slice (Top-Left corner)
  std::cout << "  Visualizing Top-Left Matrix Slice (5x8):\n";
  std::cout << "      Col 0 1 2 3 4 5 6 7 ...\n";
  std::cout << "      ───────────────────────\n";
  for (int r = 0; r < 5; r++) {
    std::cout << "  Row " << r << " │ ";
    for (int c = 0; c < 8; c++)
      std::cout << (int)matrix[r][c] << " ";
    std::cout << "...\n";
  }
  std::cout << "  ...\n";

  // ─── Step 6.2 & 6.3: Permute Columns & Read Column-by-Column ───
  std::cout
      << "\n[STEP 6.2] Reading Column-by-Column using Permutation Pattern T\n";

  // Print Pattern T snippet
  std::cout << "  Pattern T: [";
  for (size_t i = 0; i < 10; i++)
    std::cout << PATTERN_T[i] << ", ";
  std::cout << "...]\n";

  BitVector e_bits;
  e_bits.reserve(INT_E_BITS);

  // Loop through the *permuted* column indices (0 to 31)
  // For output column index j, we pick original matrix column T[j]
  for (int j = 0; j < INT_NUM_COLS; j++) {
    int original_col = PATTERN_T[j];

    // Read the entire column (all rows)
    for (int r = 0; r < INT_NUM_ROWS; r++) {
      // Check for NULLs if E was not exact multiple (not needed here but good
      // practice) For PBCH 864, it's exact.
      e_bits.push_back(matrix[r][original_col]);
    }
  }

  // ─── Summary ───
  std::cout << "  Read Complete.\n";
  print_bits("Interleaved Output (ẽ)", e_bits, 32);

  std::cout << "\n✓ STAGE 6 COMPLETE\n";
  return e_bits;
}

// ═══════════════════════════════════════════════════════════════════════════
// DEMONSTRATION SCENARIO
// ═══════════════════════════════════════════════════════════════════════════

void run_demonstration() {
  // 1. Create a Structured Input
  // To visualize the interleaving effect, let's use a pattern where:
  // f[0] = 1, f[32] = 1, f[64] = 1... (Vertical line in matrix column 0)
  // If our logic is correct, these bits (which are in Col 0) should appear
  // sequentially in the output if T[0]=0.

  BitVector input_f(INT_E_BITS, 0);

  // Mark Row 0 completely with 1s to see where they go
  // Row 0 corresponds to f[0]...f[31].
  // After column reading, these should be separated by 27 positions.
  for (int i = 0; i < 32; i++)
    input_f[i] = 1;

  std::cout
      << "═══════════════════════════════════════════════════════════════\n";
  std::cout
      << "      5G NR PBCH INTERLEAVING DEMONSTRATION                    \n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  std::cout << "Input Pattern: First 32 bits (Row 0) are '1', rest are '0'.\n";
  std::cout << "Goal: Verify that burst of 1s is dispersed.\n";

  // Run Stage 6
  BitVector output_e = stage6_channel_interleaving(input_f);

  // Final Verification
  std::cout
      << "\n═══════════════════════════════════════════════════════════════\n";
  std::cout << "FINAL VERIFICATION\n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  // Check indices.
  // e[0] should be matrix[0][T[0]]. T[0]=0. matrix[0][0] = f[0] = 1.
  // e[1] should be matrix[1][T[0]]. T[0]=0. matrix[1][0] = f[32] = 0.
  // The next '1' should appear when we read matrix[0][T[1]].
  // T[1] = 1. This is the 2nd column of output.
  // A column has 27 rows. So e[27] should be matrix[0][1] = 1.

  std::cout << "1. Checking dispersion of Row 0 bits:\n";
  std::cout << "   e[0]  = " << (int)output_e[0] << " (Expected 1)\n";
  std::cout << "   e[1]  = " << (int)output_e[1]
            << " (Expected 0 - from Row 1)\n";
  std::cout << "   e[27] = " << (int)output_e[27]
            << " (Expected 1 - from Row 0, Col T[1])\n";
  std::cout << "   e[54] = " << (int)output_e[54]
            << " (Expected 1 - from Row 0, Col T[2])\n";

  int ones_count = 0;
  for (int b : output_e)
    ones_count += b;
  std::cout << "2. Conservation of Bits: Found " << ones_count
            << " ones (Expected 32).\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  run_demonstration();
  return 0;
}
