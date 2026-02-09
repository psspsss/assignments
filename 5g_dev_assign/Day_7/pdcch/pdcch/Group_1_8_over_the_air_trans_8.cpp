// ═══════════════════════════════════════════════════════════════════════════
// FILE HEADER
// ═══════════════════════════════════════════════════════════════════════════
/*
 * ╔═════════════════════════════════════════════════════════════════════════╗
 * ║  5G NR OFDM TRANSMISSION & BEAMFORMING EMULATOR                         ║
 * ║  Implementation Reference: 3GPP TS 38.211 / TS 38.214                   ║
 * ╚═════════════════════════════════════════════════════════════════════════╝
 *
 * DESCRIPTION:
 * This program demonstrates the physical layer transmission chain of a 5G NR
 * gNodeB. It processes frequency-domain resource grid data through the OFDM
 * modulation stages (IFFT + Cyclic Prefix) and applies beamforming weights
 * defined by TCI states before simulating over-the-air propagation to a UE.
 *
 * TECHNICAL SCOPE:
 * - IFFT (Inverse Fast Fourier Transform) Implementation
 * - Cyclic Prefix (CP) Addition for ISI protection
 * - Digital Beamforming (Precoding) based on TCI States
 * - AWGN Channel Simulation
 *
 * IMPLEMENTATION STAGES:
 * 1. Frequency Domain Input Generation (IQ Samples)
 * 2. OFDM Modulation (IFFT Transformation)
 * 3. Cyclic Prefix Insertion
 * 4. Beamforming & Transmission (TCI/QCL application)
 * 5. Channel Propagation & UE Reception
 *
 * AUTHOR: AI C++ Technical Generator
 * DATE:   2023-10-27
 * STANDARDS:
 * - 3GPP TS 38.211 Section 5.3 (OFDM Baseband Signal Generation)
 * - 3GPP TS 38.214 Section 5.1.5 (Antenna ports & TCI)
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDES & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════

#include <algorithm>
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════

// Use std::complex for I/Q samples (In-Phase / Quadrature)
using Complex = std::complex<double>;

// Mathematical Constants
const double PI = 3.14159265358979323846;

/**
 * @brief Configuration for the OFDM Numerology and Grid.
 * Scaled down for educational visualization (N_FFT=32 instead of 2048/4096).
 */
struct OFDMConfig {
  int fft_size;           ///< N_FFT (e.g., 32 for demo, 4096 for FR2)
  int cp_length;          ///< Cyclic Prefix length in samples
  int subcarrier_spacing; ///< SCS in kHz (e.g., 15, 30, 120)

  // Derived Timing info
  double sampling_rate() const {
    return fft_size * subcarrier_spacing * 1000.0;
  }
};

/**
 * @brief Transmission Configuration Indicator (TCI) State.
 * Defines Quasi-Co-Location (QCL) source and beamforming weights.
 */
struct TCIState {
  int tci_id;
  std::string qcl_type;      ///< e.g., "TypeD" (Spatial Rx Param)
  std::string reference_sig; ///< e.g., "CSI-RS #3"
  Complex beam_weight;       ///< Complex precoding weight
};

/**
 * @brief Represents a time-domain OFDM symbol.
 */
struct TimeDomainSymbol {
  std::vector<Complex> samples; // Time domain IQ samples
  bool has_cp;                  // Flag if CP is attached
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Visualizes the magnitude of a signal using ASCII bars.
 *
 * @param signal Vector of complex samples
 * @param label Description of the signal
 */
void plot_signal_magnitude(const std::vector<Complex> &signal,
                           const std::string &label) {
  std::cout << "\n--- " << label << " (Magnitude Plot) ---\n";
  std::cout << "Idx | Magnitude | Visualization\n";
  std::cout << "----|-----------|------------------------------\n";

  // Determine scaling factor
  double max_mag = 0.0;
  for (const auto &s : signal)
    max_mag = std::max(max_mag, std::abs(s));
  if (max_mag < 1e-9)
    max_mag = 1.0;

  int limit =
      std::min((int)signal.size(), 16); // Only show first 16 for brevity

  for (int i = 0; i < limit; ++i) {
    double mag = std::abs(signal[i]);
    int bars = static_cast<int>((mag / max_mag) * 20); // 20 char width

    std::cout << std::setw(3) << i << " | " << std::fixed
              << std::setprecision(3) << mag << "     | ";
    for (int b = 0; b < bars; ++b)
      std::cout << "█";
    if (bars == 0 && mag > 0.001)
      std::cout << ".";
    std::cout << "\n";
  }
  if (signal.size() > 16)
    std::cout << "...\n";
}

/**
 * @brief Prints complex number in readable format (I + jQ).
 */
void print_complex(const Complex &c) {
  std::cout << "(" << std::fixed << std::setprecision(2) << c.real()
            << (c.imag() >= 0 ? "+" : "") << c.imag() << "j)";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: STAGE 1 - INPUT GENERATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Generates synthetic frequency domain data (QPSK symbols).
 * Simulates the output of the Resource Mapper.
 *
 * @param cfg OFDM Configuration
 * @param active_subcarriers Number of subcarriers carrying data
 * @return std::vector<Complex> Frequency domain vector (Length = FFT Size)
 */
std::vector<Complex> stage1_generate_frequency_data(const OFDMConfig &cfg,
                                                    int active_subcarriers) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 1: FREQUENCY DOMAIN INPUT GENERATION                   "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";

  std::vector<Complex> freq_domain(cfg.fft_size, Complex(0, 0));

  std::cout << "[STEP 1.1] Configuring Resource Grid\n";
  std::cout << "  > FFT Size: " << cfg.fft_size << "\n";
  std::cout << "  > Active Subcarriers: " << active_subcarriers << "\n";
  std::cout << "  > Modulation: QPSK (Simulated)\n";

  // QPSK Constellation points: (+/- 1 +/- 1j) / sqrt(2)
  double scale = 1.0 / std::sqrt(2.0);
  std::vector<Complex> qpsk_points = {
      {scale, scale}, {scale, -scale}, {-scale, scale}, {-scale, -scale}};

  // Fill the central subcarriers (excluding DC usually, but keeping simple)
  int start_idx = (cfg.fft_size - active_subcarriers) / 2;

  std::cout << "[STEP 1.2] Mapping modulation symbols to subcarriers...\n";
  for (int i = 0; i < active_subcarriers; ++i) {
    // Simple pattern: rotate through QPSK points
    freq_domain[start_idx + i] = qpsk_points[i % 4];
  }

  // Visualize a slice
  std::cout << "  > Data Slice (Start Index " << start_idx << "): ";
  for (int i = 0; i < 4; ++i) {
    print_complex(freq_domain[start_idx + i]);
    std::cout << " ";
  }
  std::cout << "...\n";

  std::cout << "\n✓ STAGE 1 COMPLETE: Resource Grid Loaded.\n";
  return freq_domain;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: STAGE 2 - IFFT PROCESSING
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Performs Inverse Discrete Fourier Transform (IDFT).
 * WARNING: Uses O(N^2) IDFT for educational clarity. Production uses O(N log N)
 * FFT.
 *
 * Formula: x[n] = (1/N) * Σ(X[k] * e^(j * 2π * k * n / N))
 */
std::vector<Complex>
stage2_perform_ifft(const std::vector<Complex> &freq_input) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 2: OFDM MODULATION (IFFT)                              "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";
  std::cout << "Standard Reference: 3GPP TS 38.211 Section 5.3.1\n";

  int N = freq_input.size();
  std::vector<Complex> time_output(N);

  std::cout << "[STEP 2.1] Computing IDFT (Converting Frequency → Time)...\n";
  std::cout
      << "  > Formula: x[n] = (1/N) * Σ(X[k] * exp(j * 2π * k * n / N))\n";
  std::cout << "  > Processing " << N << " samples...\n";

  for (int n = 0; n < N; ++n) {
    Complex sum(0, 0);
    for (int k = 0; k < N; ++k) {
      double angle = (2.0 * PI * k * n) / N;
      // Euler's formula: exp(jx) = cos(x) + j*sin(x)
      Complex exponential(std::cos(angle), std::sin(angle));
      sum += freq_input[k] * exponential;
    }
    time_output[n] = sum / static_cast<double>(N); // Normalization
  }

  plot_signal_magnitude(time_output, "Time Domain Signal (Pre-CP)");

  std::cout << "\n✓ STAGE 2 COMPLETE: Baseband OFDM Symbol Generated.\n";
  return time_output;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5: STAGE 3 - CYCLIC PREFIX INSERTION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Prepends the Cyclic Prefix to the symbol.
 *
 * Copies the last 'cp_len' samples to the front.
 * Protects against Inter-Symbol Interference (ISI) caused by multipath delay
 * spread.
 */
TimeDomainSymbol
stage3_add_cyclic_prefix(const std::vector<Complex> &time_signal, int cp_len) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 3: CYCLIC PREFIX (CP) INSERTION                        "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";

  int N = time_signal.size();
  if (cp_len >= N) {
    std::cerr << "CRITICAL ERROR: CP length exceeds symbol length.\n";
    return {time_signal, false};
  }

  TimeDomainSymbol result;
  result.has_cp = true;

  // Reserve space: CP length + Symbol length
  result.samples.reserve(N + cp_len);

  std::cout << "[STEP 3.1] Extracting Tail Samples...\n";
  // 1. Copy last 'cp_len' samples (The Tail)
  std::cout << "  > Copying last " << cp_len << " samples from index "
            << (N - cp_len) << " to " << (N - 1) << "\n";
  for (int i = 0; i < cp_len; ++i) {
    result.samples.push_back(time_signal[N - cp_len + i]);
  }

  // 2. Copy the original symbol body
  std::cout << "[STEP 3.2] Appending Symbol Body...\n";
  result.samples.insert(result.samples.end(), time_signal.begin(),
                        time_signal.end());

  std::cout << "  > Original Size: " << N << " samples\n";
  std::cout << "  > New Size:      " << result.samples.size()
            << " samples (CP + Symbol)\n";

  std::cout << "\n✓ STAGE 3 COMPLETE: Guard Interval Added.\n";
  return result;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 6: STAGE 4 - BEAMFORMING & TRANSMISSION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Applies Beamforming Weights based on TCI state.
 * Simulates transmission from gNB Antenna Ports.
 *
 * @param symbol Input Time Domain Symbol
 * @param tci The TCI configuration defining the beam
 */
std::vector<Complex>
stage4_beamform_and_transmit(const TimeDomainSymbol &symbol,
                             const TCIState &tci) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 4: BEAMFORMING & RF TRANSMISSION                       "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";
  std::cout << "Standard Reference: TS 38.214 (QCL & TCI)\n";

  std::cout << "[STEP 4.1] Loading TCI State Configuration...\n";
  std::cout << "  > TCI State ID: " << tci.tci_id << "\n";
  std::cout << "  > QCL Type:     " << tci.qcl_type << " (Spatial Params)\n";
  std::cout << "  > Source RS:    " << tci.reference_sig << "\n";
  std::cout << "  > Beam Weight:  ";
  print_complex(tci.beam_weight);
  std::cout << "\n";

  std::vector<Complex> rf_signal;
  rf_signal.reserve(symbol.samples.size());

  // Apply simple scalar beamforming (Precoding)
  // In MIMO, this would be a matrix multiplication: y = W * x
  std::cout << "[STEP 4.2] Applying Precoding Weights (W * x)...\n";

  for (const auto &samp : symbol.samples) {
    rf_signal.push_back(samp * tci.beam_weight);
  }

  std::cout << "  > Beamforming applied to " << rf_signal.size()
            << " samples.\n";
  std::cout << "  > Signal passed to DAC and RF Front End.\n";

  std::cout << "\n✓ STAGE 4 COMPLETE: Signal Radiated.\n";
  return rf_signal;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 7: STAGE 5 - CHANNEL & RECEPTION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Simulates wireless channel propagation and UE reception.
 * Adds Path Loss attenuation and AWGN (Additive White Gaussian Noise).
 */
void stage5_channel_propagation(const std::vector<Complex> &tx_signal,
                                int ue_id) {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════════"
               "═════════╗\n";
  std::cout << "║ STAGE 5: CHANNEL PROPAGATION -> UE RECEPTION                 "
               "         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═════════╝\n";

  std::cout << "Target: UE #" << ue_id << "\n";

  // Simulation parameters
  double path_loss_factor = 0.5; // Signal attenuates by 50%
  double noise_floor = 0.05;     // Noise magnitude

  std::cout << "[STEP 5.1] Simulating Air Interface...\n";
  std::cout << "  > Path Loss Factor: " << path_loss_factor << "\n";
  std::cout << "  > Adding AWGN (Noise)...\n";

  std::vector<Complex> rx_signal;
  rx_signal.reserve(tx_signal.size());

  // Random Number Gen for Noise
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0.0, noise_floor);

  for (const auto &tx_samp : tx_signal) {
    // Apply Path Loss
    Complex attenuated = tx_samp * path_loss_factor;

    // Add Noise
    Complex noise(distribution(generator), distribution(generator));
    rx_signal.push_back(attenuated + noise);
  }

  plot_signal_magnitude(rx_signal, "Received Signal at UE Antenna");

  std::cout << "[STEP 5.2] Reception Summary\n";
  std::cout << "  > Rx Samples: " << rx_signal.size() << "\n";
  std::cout << "  > Ready for Synchronization and CP Removal.\n";

  std::cout << "\n✓ STAGE 5 COMPLETE: Transmission Successful.\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// DEMONSTRATION SCENARIOS
// ═══════════════════════════════════════════════════════════════════════════

void demo_normal_transmission() {
  std::cout << "\n============================================================="
               "==========\n";
  std::cout << "SCENARIO A: NORMAL TRANSMISSION TO UE#7 (QCL-TypeD)\n";
  std::cout << "==============================================================="
               "========\n";

  // 1. Configuration
  OFDMConfig config;
  config.fft_size = 32;           // Small FFT for demo visibility
  config.cp_length = 8;           // Extended CP for robustness
  config.subcarrier_spacing = 30; // 30 kHz SCS

  // 2. Beamforming Config (TCI State)
  TCIState tci;
  tci.tci_id = 1;
  tci.qcl_type = "QCL-TypeD";
  tci.reference_sig = "CSI-RS #3";
  // Beam directed at 45 degrees (simulated complex weight)
  // 1.0 magnitude, 45 deg phase = 0.707 + 0.707j
  tci.beam_weight = Complex(0.707, 0.707);

  // 3. Execution Pipeline
  // Step 1: Input Data
  auto freq_data = stage1_generate_frequency_data(
      config, 12); // 12 active subcarriers (1 RB)

  // Step 2: IFFT
  auto time_data = stage2_perform_ifft(freq_data);

  // Step 3: Cyclic Prefix
  auto symbol_with_cp = stage3_add_cyclic_prefix(time_data, config.cp_length);

  // Step 4: Beamforming
  auto rf_signal = stage4_beamform_and_transmit(symbol_with_cp, tci);

  // Step 5: Channel
  stage5_channel_propagation(rf_signal, 7);
}

void demo_beam_failure() {
  std::cout << "\n============================================================="
               "==========\n";
  std::cout << "SCENARIO B: BEAM MISALIGNMENT / WEAK SIGNAL\n";
  std::cout << "==============================================================="
               "========\n";

  OFDMConfig config;
  config.fft_size = 32;
  config.cp_length = 4;
  config.subcarrier_spacing = 15;

  // Weak Beam weight (misaligned side lobe)
  TCIState tci_weak;
  tci_weak.tci_id = 99;
  tci_weak.qcl_type = "Unknown";
  tci_weak.reference_sig = "SSB Index 0";
  tci_weak.beam_weight = Complex(0.1, 0.0); // Very weak gain

  auto freq_data = stage1_generate_frequency_data(config, 8);
  auto time_data = stage2_perform_ifft(freq_data);
  auto symbol_with_cp = stage3_add_cyclic_prefix(time_data, config.cp_length);
  auto rf_signal = stage4_beamform_and_transmit(symbol_with_cp, tci_weak);
  stage5_channel_propagation(rf_signal, 7);
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << R"(
    ###################################################################
    #                                                                 #
    #    5G NR OVER-THE-AIR (OTA) TRANSMISSION SIMULATOR              #
    #         OFDM | IFFT | CYCLIC PREFIX | BEAMFORMING               #
    #                                                                 #
    ###################################################################
    )" << "\n";

  std::cout << "TECHNICAL NOTES:\n";
  std::cout << "1. This simulation uses a scaled-down FFT size (N=32) for "
               "visual clarity.\n";
  std::cout << "   Production 5G uses N=512 to N=4096 depending on Bandwidth "
               "Part (BWP).\n";
  std::cout << "2. Beamforming is applied digitally (Precoding) in the "
               "Frequency or Time domain.\n";
  std::cout << "   Here we simulate Time Domain weighting for simplicity.\n";
  std::cout << "3. TCI (Transmission Configuration Indicator) links the "
               "PDCCH/PDSCH to a\n";
  std::cout << "   specific Reference Signal (CSI-RS) for spatial filtering "
               "(Beamforming).\n\n";

  try {
    demo_normal_transmission();
    demo_beam_failure();
  } catch (const std::exception &e) {
    std::cerr << "SYSTEM ERROR: " << e.what() << "\n";
    return 1;
  }

  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "FINAL SUMMARY:\n";
  std::cout << "✓ Frequency Data Converted to Time Domain (IFFT).\n";
  std::cout << "✓ Cyclic Prefix Added (Tail copied to Head).\n";
  std::cout << "✓ Beamforming Weights Applied (TCI: QCL-TypeD).\n";
  std::cout << "✓ Signal Propagated to UE#7.\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  return 0;
}
