// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/**
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR PDSCH IMPLEMENTATION: OFDM MODULATION (IFFT + CP)                ║
 * ║  Reference: 3GPP TS 38.211 Section 5.3                                  ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This code implements the Waveform Generation stage.
 * 1. IFFT: Transforms frequency domain subcarriers to time domain samples.
 * 2. CP Addition: Copies the tail of the symbol to the beginning.
 *
 * PARAMETERS (Simplified for Demo):
 * - N_fft: 128 (FFT Size)
 * - N_cp:  10  (Cyclic Prefix Length)
 * - SCS:   15 kHz logic assumed
 *
 * ALGORITHM:
 * Uses a basic recursive Cooley-Tukey IFFT algorithm for demonstration.
 * In production, this would use optimized libraries like FFTW or Intel MKL.
 *
 * AUTHOR: Technical Documentation Bot
 * DATE:   2026-02-06
 */

#include <algorithm>
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// TYPES & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

using Complex = std::complex<double>;
using TimeSequence = std::vector<Complex>;
using FreqGrid = std::vector<std::vector<Complex>>; // [k][l]

const double PI = 3.14159265358979323846;

// Configuration for the Demo
const int N_FFT = 128;      // FFT Size (Power of 2)
const int N_CP = 10;        // Cyclic Prefix samples
const int N_ACTIVE_SC = 72; // Active subcarriers (e.g., 6 RBs)

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: IFFT ENGINE
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Reorders vector for FFT (Bit Reversal Permutation)
 */
void bit_reverse_copy(const std::vector<Complex> &input,
                      std::vector<Complex> &output) {
  int n = input.size();
  int bits = std::log2(n);

  for (int i = 0; i < n; i++) {
    int rev = 0;
    int val = i;
    for (int j = 0; j < bits; j++) {
      rev = (rev << 1) | (val & 1);
      val >>= 1;
    }
    output[rev] = input[i];
  }
}

/**
 * @brief Performs Inverse Fast Fourier Transform (Cooley-Tukey)
 *
 * Math: X[n] = (1/N) * sum(x[k] * exp(j*2*pi*n*k/N))
 */
void perform_ifft(std::vector<Complex> &buffer) {
  int n = buffer.size();
  if (n <= 1)
    return;

  // 1. Bit Reversal (Iterative in-place optimization omitted for recursive
  // clarity,
  //    using simple recursive split here)

  // NOTE: For brevity in this demo, we implement a recursive O(N log N) logic.
  // For IFFT, we conjugate input, perform FFT, conjugate output, and scale.

  // Step A: Conjugate Input
  for (auto &val : buffer)
    val = std::conj(val);

  // Step B: Forward FFT (Recursive)
  // We'll define a lambda or helper for the core FFT recursion
  auto recursive_fft = [](auto &&self, std::vector<Complex> &a) -> void {
    int size = a.size();
    if (size <= 1)
      return;

    std::vector<Complex> even(size / 2);
    std::vector<Complex> odd(size / 2);

    for (int i = 0; i < size / 2; i++) {
      even[i] = a[i * 2];
      odd[i] = a[i * 2 + 1];
    }

    self(self, even);
    self(self, odd);

    for (int k = 0; k < size / 2; k++) {
      Complex t = std::polar(1.0, -2.0 * PI * k / size) * odd[k];
      a[k] = even[k] + t;
      a[k + size / 2] = even[k] - t;
    }
  };

  recursive_fft(recursive_fft, buffer);

  // Step C: Conjugate Output and Scale by 1/N
  double scale = 1.0 / n;
  for (auto &val : buffer) {
    val = std::conj(val) * scale;
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: CYCLIC PREFIX (CP)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Adds Cyclic Prefix to the time domain symbol.
 *
 * Operation: Take last N_cp samples and prepend them to the front.
 * Input:  [0, 1, 2 ... 127]
 * Output: [118...127, 0, 1, 2 ... 127]
 */
TimeSequence add_cyclic_prefix(const TimeSequence &ifft_out, int cp_len) {
  TimeSequence symbol_with_cp;
  symbol_with_cp.reserve(ifft_out.size() + cp_len);

  // 1. Copy Tail (CP)
  int start_idx = ifft_out.size() - cp_len;
  symbol_with_cp.insert(symbol_with_cp.end(), ifft_out.begin() + start_idx,
                        ifft_out.end());

  // 2. Copy Body (Original IFFT)
  symbol_with_cp.insert(symbol_with_cp.end(), ifft_out.begin(), ifft_out.end());

  return symbol_with_cp;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: OFDM GENERATOR
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Process one OFDM Symbol from Frequency Domain to Time Domain
 */
TimeSequence process_ofdm_symbol(const std::vector<Complex> &freq_data) {
  std::cout << "  -> Processing Symbol... ";

  // 1. Prepare FFT Buffer (Pad with Zeros if input < N_FFT)
  std::vector<Complex> fft_buffer(N_FFT, Complex(0, 0));

  // Map Subcarriers to FFT Bins
  // Standard mapping: DC at index 0 (or center depending on implementation).
  // Here we map 0..N_active to 0..N_active for simplicity.
  // (In reality, we perform an FFT Shift).
  for (size_t i = 0; i < freq_data.size() && i < N_FFT; i++) {
    fft_buffer[i] = freq_data[i];
  }

  // 2. Perform IFFT
  perform_ifft(fft_buffer);

  // 3. Add Cyclic Prefix
  TimeSequence final_symbol = add_cyclic_prefix(fft_buffer, N_CP);

  std::cout << "Done. (Size: " << final_symbol.size() << " samples)\n";
  return final_symbol;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: VISUALIZATION
// ═══════════════════════════════════════════════════════════════════════════

void print_signal_stats(const TimeSequence &seq) {
  std::cout << "\n[SIGNAL ANALYSIS]\n";
  std::cout << "Total Samples: " << seq.size() << "\n";

  // Check CP Property: Start of symbol should match End of symbol
  bool cp_match = true;
  for (int i = 0; i < N_CP; i++) {
    // CP is at index [i]
    // Tail is at index [N_CP + (N_FFT - N_CP) + i]? No.
    // CP is index 0..9. Body is 10..137.
    // Tail of Body is index (10 + 128 - 10) = 128 to 137.

    Complex cp_val = seq[i];
    Complex tail_val = seq[N_FFT + i]; // Offset by FFT size

    // Allow tiny floating point error
    if (std::abs(cp_val - tail_val) > 1e-9)
      cp_match = false;
  }

  if (cp_match)
    std::cout << "✓ CP INTEGRITY CHECK: PASS (Prefix matches Body Tail)\n";
  else
    std::cout << "X CP INTEGRITY CHECK: FAIL\n";

  std::cout << "First 5 Samples (Time Domain):\n";
  for (int i = 0; i < 5; i++) {
    std::cout << " [" << i << "]: " << seq[i].real() << " + " << seq[i].imag()
              << "j\n";
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE: OFDM MODULATION (IFFT + CP)                           ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "Configuration: FFT=" << N_FFT << ", CP=" << N_CP << "\n\n";

  // 1. Generate Mock Frequency Domain Data (One OFDM Symbol)
  // Let's create a subcarrier pattern (e.g., all 1s)
  std::vector<Complex> subcarriers(N_ACTIVE_SC, Complex(1.0, 0.0));

  // Add some "Data" variation
  for (int i = 0; i < N_ACTIVE_SC; i++) {
    if (i % 2 == 0)
      subcarriers[i] = Complex(-1.0, 0.0);
  }

  // 2. Process
  TimeSequence waveform = process_ofdm_symbol(subcarriers);

  // 3. Analyze
  print_signal_stats(waveform);

  std::cout
      << "\n---------------------------------------------------------------\n";
  std::cout
      << "FINAL OUTPUT: This vector 'waveform' is sent to the DAC/Antenna.\n";
  std::cout
      << "---------------------------------------------------------------\n";

  return 0;
}
