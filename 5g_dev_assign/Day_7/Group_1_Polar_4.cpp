// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  TITLE:       5G NR PBCH: POLAR CODING (N=512)                          ║
 * ║  AUTHOR:      AI Technical Documentation Generator                      ║
 * ║  DATE:        2023-10-27                                                ║
 * ║  STANDARD:    3GPP TS 38.212 Section 5.3.1, 7.1.3                       ║
 * ║                                                                         ║
 * ║  DESCRIPTION:                                                           ║
 * ║  Implements Stage 4 of the PBCH chain: Polar Encoding.                  ║
 * ║  Transforms the 56-bit input block (Payload+CRC) into a 512-bit         ║
 * ║  codeword using the Polar transform G_N.                                ║
 * ║                                                                         ║
 * ║  ALGORITHM STEPS:                                                       ║
 * ║  1. Determine Mother Code Size N=512 (for E=864, K=56).                 ║
 * ║  2. Generate Reliability Sequence (TS 38.212 Table 5.3.1.2-1).          ║
 * ║  3. Map K=56 information bits to the most reliable indices.             ║
 * ║  4. Set remaining 456 bits to Zero (Frozen Bits).                       ║
 * ║  5. Apply Polar Transform: d = u * G_512.                               ║
 * ║                                                                         ║
 * ║  INPUT SCENARIO:                                                        ║
 * ║  - Input: 56 bits (from CRC stage)                                      ║
 * ║  - Output: 512 encoded bits                                             ║
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
const int POLAR_N = 512; // Mother Code Size (2^9)
const int POLAR_K = 56;  // Information Bits (Payload + CRC)
const int POLAR_E = 864; // Rate Match Output Size

using BitVector = std::vector<uint8_t>;
using IndexVector = std::vector<uint16_t>;

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Prints a visual representation of bits (Heatmap style).
 * 0 is represented by '.', 1 by '■'.
 */
void print_bit_heatmap(const std::string &label, const BitVector &bits,
                       int width = 64) {
  std::cout << "  " << label << " (" << bits.size() << " bits):\n  │";
  for (size_t i = 0; i < bits.size(); ++i) {
    if (i > 0 && i % width == 0)
      std::cout << "\n  │";
    std::cout << (bits[i] ? "■" : ".");
  }
  std::cout << "\n\n";
}

/**
 * @brief Prints binary vector in standard format.
 */
void print_bits(const std::string &label, const BitVector &bits,
                size_t limit = 64) {
  std::cout << "  " << std::left << std::setw(20) << label << " │ ";
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
// SECTION 3: RELIABILITY SEQUENCE
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Returns the Reliability Sequence for N=512.
 *
 * In a full production implementation, this would extract from the
 * 1024-length master sequence in TS 38.212.
 *
 * For this implementation, we implement the sub-channel allocation logic:
 * We need the K=56 most reliable indices.
 *
 * Note: To keep the code file within reasonable size, we generate a
 * "Simulated" 3GPP-like reliability sequence based on bit-reversal and weight,
 * which is the mathematical basis of Polar codes.
 *
 * @return std::vector<uint16_t> Ordered list of indices (Least Reliable -> Most
 * Reliable)
 */
IndexVector get_reliability_sequence_N512() {
  // NOTE: This is a simplified generator for N=512 to approximate 3GPP order
  // In production, use exact Table 5.3.1.2-1.

  IndexVector seq(POLAR_N);
  // Initialize with 0..511
  for (int i = 0; i < POLAR_N; i++)
    seq[i] = i;

  // Sort based on Hamming Weight of index (Approximation of reliability)
  // Higher Hamming weight (more 1s in binary index) generally -> Higher
  // reliability in Polar This is because indices with more 1s correspond to
  // rows of G_N with higher weight.
  std::sort(seq.begin(), seq.end(), [](uint16_t a, uint16_t b) {
    // Count set bits
    int wa = 0;
    for (int i = 0; i < 9; i++)
      if ((a >> i) & 1)
        wa++;
    int wb = 0;
    for (int i = 0; i < 9; i++)
      if ((b >> i) & 1)
        wb++;

    if (wa != wb)
      return wa < wb;
    return a < b; // Tie-break
  });

  return seq;
}

// ═══════════════════════════════════════════════════════════════════════════
// STAGE 4: POLAR ENCODING
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Recursive Polar Transform Kernel.
 * Computes u * G_N.
 *
 * @param buffer Reference to the bit vector to transform in-place
 * @param start Start index of the current slice
 * @param len Length of the current slice
 */
void polar_transform_recursive(BitVector &buffer, size_t start, size_t len) {
  // Base case: length 1, nothing to do
  if (len == 1)
    return;

  size_t half = len / 2;

  // 1. Apply F = [1 0; 1 1] logic for the current stage
  // For vector u, standard butterfly:
  // left = u[i] XOR u[i+half]
  // right = u[i+half] (unchanged in this direction, waits for next stage)
  //
  // Note: The formula d = u * G_N applies G recursively.
  // The simplified butterfly structure:
  // Upper branch (indices 0..half-1) = u_upper + u_lower
  // Lower branch (indices half..len-1) = u_lower

  for (size_t i = 0; i < half; ++i) {
    // u[i] = u[i] ⊕ u[i+half]
    buffer[start + i] = (buffer[start + i] + buffer[start + i + half]) % 2;
  }

  // 2. Recurse on both halves
  polar_transform_recursive(buffer, start, half);
  polar_transform_recursive(buffer, start + half, half);
}

/**
 * @brief Executes Stage 4: Polar Coding
 *
 * @param input_bits The 56-bit vector (Payload + CRC)
 * @return BitVector The 512-bit encoded codeword
 */
BitVector stage4_polar_coding(const BitVector &input_bits) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 4: POLAR CODING (N=512)                               ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Standard Reference: TS 38.212 Section 5.3.1\n";
  std::cout << "Parameters: K=" << POLAR_K << ", N=" << POLAR_N
            << ", E=" << POLAR_E << "\n\n";

  if (input_bits.size() != POLAR_K) {
    std::cerr << "Error: Input size " << input_bits.size()
              << " != K=" << POLAR_K << "\n";
    return BitVector();
  }

  // ─── Step 4.1: Information Bit Mapping ───
  std::cout << "[STEP 4.1] Generating Input Vector u (Size " << POLAR_N
            << ")\n";
  std::cout << "  Mapping " << POLAR_K
            << " information bits to most reliable positions.\n";
  std::cout << "  Remaining " << (POLAR_N - POLAR_K)
            << " bits are Frozen (set to 0).\n";

  // 1. Get Reliability Sequence
  IndexVector Q = get_reliability_sequence_N512();

  // 2. Identify Info Indices (The last K entries of Q are the most reliable)
  // Note: Q is sorted Least Reliable -> Most Reliable
  std::vector<int> info_map(POLAR_N, 0); // 0=Frozen, 1=Info

  for (size_t i = 0; i < POLAR_K; i++) {
    // Get index from the end of Q
    uint16_t idx = Q[POLAR_N - POLAR_K + i];
    info_map[idx] = 1;
  }

  // 3. Construct u vector
  BitVector u(POLAR_N, 0);
  int input_ptr = 0;

  // According to 3GPP, we map input bits b0...bK-1 to the allocated channels
  // The reliable indices are sorted?
  // "The information bits are mapped to the indices... corresponding to the K
  // highest values" Usually mapped in order of increasing index within the
  // reliability set.

  // We collect the chosen indices and sort them to map input sequentially
  IndexVector chosen_indices;
  for (size_t i = 0; i < POLAR_K; i++) {
    chosen_indices.push_back(Q[POLAR_N - POLAR_K + i]);
  }
  std::sort(chosen_indices.begin(), chosen_indices.end());

  // Map bits
  std::cout << "  Mapping details (First 5 mappings):\n";
  for (size_t i = 0; i < chosen_indices.size(); i++) {
    u[chosen_indices[i]] = input_bits[i];
    if (i < 5) {
      std::cout << "    Bit b" << i << " (" << (int)input_bits[i] << ") -> u["
                << chosen_indices[i] << "]\n";
    }
  }

  // Visualize u
  std::cout << "\n  Input Vector u constructed:\n";
  print_bit_heatmap("u Heatmap (■=Info/1, .=Frozen/0)", u);

  // ─── Step 4.2: Encoding ───
  std::cout << "[STEP 4.2] Applying Polar Transform (d = u * G_N)\n";
  std::cout << "  Kernel: G_N = F^(x9), Recursive Butterfly structure.\n";

  // Copy u to d for in-place transformation
  BitVector d = u;

  // Perform Transform
  polar_transform_recursive(d, 0, POLAR_N);

  std::cout << "  Transform complete.\n";
  print_bit_heatmap("Encoded d Heatmap", d);

  std::cout << "✓ STAGE 4 COMPLETE: " << d.size() << " coded bits generated.\n";
  return d;
}

// ═══════════════════════════════════════════════════════════════════════════
// DEMONSTRATION SCENARIO
// ═══════════════════════════════════════════════════════════════════════════

void run_demonstration() {
  // 1. Create a simulated 56-bit input (B)
  // Using the output structure from Stage 3: Payload (32) + CRC (24)
  BitVector input_B;
  // Pattern: 101010... for payload, 11110000... for CRC
  for (int i = 0; i < 32; i++)
    input_B.push_back(i % 2);
  for (int i = 0; i < 24; i++)
    input_B.push_back((i / 4) % 2);

  std::cout
      << "═══════════════════════════════════════════════════════════════\n";
  std::cout
      << "      5G NR POLAR CODING DEMONSTRATION                         \n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";

  print_bits("Input Block (b)", input_B);

  // 2. Run Encoding
  BitVector output_d = stage4_polar_coding(input_B);

  // 3. Summary
  std::cout
      << "\n═══════════════════════════════════════════════════════════════\n";
  std::cout << "FINAL OUTPUT SUMMARY\n";
  std::cout
      << "═══════════════════════════════════════════════════════════════\n";
  std::cout << "1. Frozen Bits: " << (POLAR_N - POLAR_K) << " (Set to 0)\n";
  std::cout << "2. Info Bits:   " << POLAR_K
            << " (Mapped to reliable channels)\n";
  std::cout << "3. Code Rate:   R = K/N = " << POLAR_K << "/" << POLAR_N
            << " = " << std::fixed << std::setprecision(3)
            << (float)POLAR_K / POLAR_N << "\n";
  std::cout << "4. First 32 Output Bits (d0...d31):\n";

  // Print explicit hex dump of first few bytes
  for (int i = 0; i < 4; i++) {
    uint8_t byte = 0;
    for (int j = 0; j < 8; j++) {
      if (output_d[i * 8 + j])
        byte |= (1 << (7 - j));
    }
    std::cout << "   Byte " << i << ": 0x" << std::hex << std::uppercase
              << (int)byte << std::dec << "\n";
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  run_demonstration();
  return 0;
}
