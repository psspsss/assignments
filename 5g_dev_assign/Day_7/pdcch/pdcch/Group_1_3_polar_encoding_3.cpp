/**
 * ═══════════════════════════════════════════════════════════════════════════
 *  TITLE:       5G NR POLAR ENCODER (3GPP TS 38.212)
 *  AUTHOR:      AI C++ Generator
 *  DATE:        2023-10-27
 *  STANDARD:    3GPP TS 38.212 V17.0.0 (Section 5.3.1 Polar Coding)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *  DESCRIPTION:
 *  This program simulates the Polar Encoding process for the Physical Downlink
 *  Control Channel (PDCCH). It takes the DCI+CRC bits (K=66) and encodes them
 *  into a block of N=512 bits.
 *
 *  PROCESS OVERVIEW:
 *  1. [SETUP]      Define input information bits (DCI payload + CRC).
 *  2. [MAPPING]    Map info bits to the "Most Reliable" sub-channels (Indices).
 *                  Set "Frozen" bits (unreliable positions) to 0.
 *  3. [TRANSFORM]  Apply the Polar Transform matrix G_N using the recursive
 *                  butterfly structure (Fast Walsh-Hadamard Transform logic).
 *  4. [OUTPUT]     Generate the N=512 coded bit sequence.
 *
 *  SCENARIO PARAMETERS:
 *  - K (Message Length): 66 bits
 *  - N (Mother Code):    512 bits (Power of 2)
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint> // Added for uint8_t
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

using Bit = uint8_t;
using BitVector = std::vector<Bit>;

// Mother Code Size (Must be power of 2)
const int N_POLAR = 512;
// Information Bit Length
const int K_INFO = 66;

/**
 * @brief Represents the internal state of the Polar Encoder
 */
struct PolarContext {
  BitVector info_bits;                  // Input K bits
  std::vector<int> reliability_indices; // Ordered list of indices (0..N-1)
  BitVector u_vector;  // Input vector 'u' (Map K info bits + N-K frozen)
  BitVector x_encoded; // Output vector 'x' (Encoded)
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Converts binary string to BitVector
 */
BitVector parse_bits(const std::string &str) {
  BitVector bits;
  for (char c : str) {
    if (c == '1')
      bits.push_back(1);
    else if (c == '0')
      bits.push_back(0);
  }
  return bits;
}

/**
 * @brief Visualization helper for bit arrays
 */
void print_buffer(const BitVector &buf, const std::string &label,
                  int limit = 64) {
  std::cout
      << "┌───────────────────────────────────────────────────────────────┐\n";
  std::cout << "│ " << std::left << std::setw(61) << label << " │\n";
  std::cout
      << "├───────────────────────────────────────────────────────────────┤\n";

  // Print binary grid
  for (size_t i = 0; i < buf.size(); i += 32) {
    if ((int)i >= limit && limit != -1) {
      std::cout << "│ ... (" << (buf.size() - i)
                << " bits remaining)                         │\n";
      break;
    }
    std::cout << "│ ";
    for (size_t j = 0; j < 32; ++j) {
      if (i + j < buf.size())
        std::cout << (int)buf[i + j];
      else
        std::cout << " ";
    }
    // Proper padding calculation
    size_t remaining = (buf.size() - i) < 32 ? (32 - (buf.size() - i)) : 0;
    std::cout << std::string(remaining, ' ');
    std::cout << "      │\n";
  }
  std::cout
      << "└───────────────────────────────────────────────────────────────┘\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: CORE IMPLEMENTATION (Reliability & Transform)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Generate Reliability Sequence (Simplified for Education)
 *
 * In production 5G (TS 38.212), this uses the detailed Q-sequence (1024 max).
 * Here, we implement a simplified "Bit-Reversal" weight calculation which
 * closely mimics the concept of reliability ordering for Polar Codes.
 * Indices with higher "weight" (more 1s in binary rep) are generally more
 * reliable.
 *
 * @return Sorted indices from Least Reliable to Most Reliable
 */
std::vector<int> get_reliability_sequence_N512() {
  std::vector<std::pair<int, int>> weights; // {weight, index}

  // Use a simplified sorting strategy to simulate channel reliability
  // Real 5G sequence is a hardcoded table (Q_sequence).

  std::vector<int> Q(N_POLAR);
  for (int i = 0; i < N_POLAR; i++)
    Q[i] = i;

  std::sort(Q.begin(), Q.end(), [](int a, int b) {
    // Sort by bit-reversed value (simplified polar construction)
    // This puts "good" channels at the end.
    unsigned int ra = 0, rb = 0;
    for (int i = 0; i < 9; i++) { // log2(512)=9
      if ((a >> i) & 1)
        ra |= (1 << (8 - i));
      if ((b >> i) & 1)
        rb |= (1 << (8 - i));
    }
    return ra < rb;
  });

  return Q;
}

/**
 * @brief STAGE 2: SUB-CHANNEL ALLOCATION (Mapping)
 *
 * Maps the K info bits to the K most reliable positions in the u-vector.
 * All other positions (Frozen bits) are set to 0.
 */
void stage2_map_bits(PolarContext &ctx) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 2: SUB-CHANNEL ALLOCATION (MAPPING)                    ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Standard: TS 38.212 Section 5.3.1.2\n";
  std::cout << "Logic: Map " << ctx.info_bits.size()
            << " info bits to most reliable indices.\n";

  // 1. Initialize u vector with zeros (Frozen bits)
  ctx.u_vector.assign(N_POLAR, 0);

  // 2. Get Reliability Sequence
  // We need the K indices with the HIGHEST reliability.
  // In our sorted Q array (Low -> High), we take the last K entries.
  ctx.reliability_indices = get_reliability_sequence_N512();

  std::cout << "[STEP 2.1] Retrieved Reliability Sequence Q(0..511).\n";

  // 3. Map Info Bits
  std::cout << "[STEP 2.2] Mapping info bits to top " << K_INFO
            << " reliable channels.\n";

  // 5G mapping strategy: map message to the sorted reliable indices.
  int start_reliable_idx = N_POLAR - K_INFO; // Start of the "Good" region

  for (int i = 0; i < K_INFO; ++i) {
    // Get the specific index in u from the Q sequence
    int u_idx = ctx.reliability_indices[start_reliable_idx + i];

    // Place info bit
    ctx.u_vector[u_idx] = ctx.info_bits[i];
  }

  std::cout << "Info Mapping Summary:\n";
  std::cout << " - Total Channels (N): " << N_POLAR << "\n";
  std::cout << " - Frozen Bits (0):    " << (N_POLAR - K_INFO) << "\n";
  std::cout << " - Info Bits (1/0):    " << K_INFO << "\n";

  print_buffer(ctx.u_vector, "Vector 'u' (Pre-Transform)", 128);

  std::cout << "✓ STAGE 2 COMPLETE\n";
}

/**
 * @brief STAGE 3: POLAR TRANSFORM (ENCODING)
 *
 * Computes x = u * G_N
 * Uses the recursive butterfly structure (like FFT/FWHT) for O(N log N).
 */
void stage3_polar_transform(PolarContext &ctx) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 3: POLAR ENCODING (TRANSFORM G_N)                      ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Formula: x = u * G_N (Kronecker Product)\n";
  std::cout << "Algorithm: Recursive Butterfly XOR updates.\n\n";

  // Copy u to x to perform in-place transformation
  ctx.x_encoded = ctx.u_vector;

  int stages = (int)std::log2(N_POLAR);
  std::cout << "[STEP 3.1] Executing " << stages
            << " stages of XOR operations...\n";

  // Iterative implementation of the Plotkin construction / Arikan's Butterfly
  for (int stage = 0; stage < stages; ++stage) {
    int stride = 1 << stage;     // 1, 2, 4, 8...
    int group_size = stride * 2; // 2, 4, 8, 16...

    // Process blocks
    for (int i = 0; i < N_POLAR; i += group_size) {
      for (int j = 0; j < stride; ++j) {
        // Indices for the butterfly operation
        int idx1 = i + j;
        int idx2 = i + j + stride;

        // Save original values
        Bit u1 = ctx.x_encoded[idx1];
        Bit u2 = ctx.x_encoded[idx2];

        // Update Rule (XOR)
        ctx.x_encoded[idx1] = (u1 + u2) % 2;
        // ctx.x_encoded[idx2] remains u2
      }
    }
  }

  std::cout << "[STEP 3.2] Transform complete.\n";
  print_buffer(ctx.x_encoded, "Polar Encoded Vector 'x' (N=512)", 128);

  std::cout << "✓ STAGE 3 COMPLETE\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN & DEMO SCENARIOS
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════╗\n";
  std::cout << "║           5G NR POLAR ENCODER SIMULATOR (DCI PROCESSING)     "
               "     ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════╝\n";
  std::cout << " Demonstrating K=66 -> N=512 Channel Coding\n\n";

  // 1. SETUP
  // Input Info Bits (66 bits from Prompt)
  std::string input_str =
      "011111000101110010110101001000111011110011110111010101110110";

  // Pad to exactly 66 bits if the input string is short
  while (input_str.length() < 66)
    input_str += "0";

  PolarContext ctx;
  ctx.info_bits = parse_bits(input_str.substr(0, 66));

  std::cout << "INPUT INFO BITS (K=66):\n";
  print_buffer(ctx.info_bits, "Input Payload (DCI+CRC)", -1);

  // 2. MAPPING
  stage2_map_bits(ctx);

  // 3. TRANSFORM
  stage3_polar_transform(ctx);

  // 4. VERIFICATION OUTPUT
  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << " FINAL ENCODED OUTPUT (First 80 bits view)\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";

  std::cout << "Binary Output Stream:\n";
  for (int i = 0; i < 80 && i < N_POLAR; ++i) {
    std::cout << (int)ctx.x_encoded[i];
  }
  std::cout << "...\n\n";

  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << " KEY TAKEAWAYS\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";
  std::cout << "1. Channel Polarization: The transform creates synthesized "
               "channels\n";
  std::cout << "   where some are perfect (Capacity=1) and some are noise "
               "(Capacity=0).\n";
  std::cout << "2. Frozen Bits: We force 0 into the noisy positions so the "
               "receiver\n";
  std::cout << "   knows them in advance, aiding the decoding of the reliable "
               "bits.\n";
  std::cout
      << "3. Code Construction: Success depends on correctly identifying the\n";
  std::cout << "   reliable indices (Q-Sequence) for the specific channel "
               "conditions.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════\n";

  return 0;
}
