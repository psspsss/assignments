/*
 * 5G NR Physical Layer - OFDM Modulation Simulator
 * Reference: 3GPP TS 38.211
 * Team-Based Implementation - 5 Developers
 * 
 * Module Distribution:
 * DEV 1: Numerology & Configuration Manager
 * DEV 2: Digital Modulation Engine (QPSK, 16-QAM, 64-QAM, 256-QAM)
 * DEV 3: Resource Grid & Mapping
 * DEV 4: IFFT/FFT Processing & Signal Generation
 * DEV 5: Cyclic Prefix & OFDM Symbol Assembly
 */

#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <map>
#include <string>
#include <random>
#include <algorithm>
#include <sstream>
#include <memory>

using namespace std;
using Complex = complex<double>;

// ============================================================================
// DEVELOPER 1: NUMEROLOGY & CONFIGURATION MANAGER
// Responsible for: 5G NR numerology, subcarrier spacing, frame structure
// ============================================================================

class NumerologyManager {
public:
    enum class NumerologyIndex { MU_0 = 0, MU_1 = 1, MU_2 = 2, MU_3 = 3, MU_4 = 4 };
    enum class CyclicPrefixType { NORMAL, EXTENDED };
    
    struct NRConfig {
        NumerologyIndex mu;
        int subcarrier_spacing_khz;
        int fft_size;
        int num_subcarriers;
        int num_symbols_per_slot;
        int num_slots_per_subframe;
        double slot_duration_ms;
        CyclicPrefixType cp_type;
        vector<int> cp_lengths;  // CP length for each symbol in samples
    };

private:
    NRConfig config;
    map<int, int> mu_to_scs = {{0, 15}, {1, 30}, {2, 60}, {3, 120}, {4, 240}};
    
    void calculateCPLengths() {
        // Normal CP lengths based on 3GPP TS 38.211
        // First symbol has longer CP
        int sampling_rate = config.fft_size * config.subcarrier_spacing_khz * 1000;
        
        if (config.cp_type == CyclicPrefixType::NORMAL) {
            config.cp_lengths.clear();
            // For mu=1 (30 kHz), typical CP lengths
            int first_cp = config.fft_size * 9 / 128;  // Longer CP for first symbol
            int normal_cp = config.fft_size * 9 / 128;  // Normal CP
            
            config.cp_lengths.push_back(first_cp);
            for (int i = 1; i < config.num_symbols_per_slot; i++) {
                config.cp_lengths.push_back(normal_cp);
            }
        } else {
            // Extended CP (used in specific scenarios)
            int ext_cp = config.fft_size / 4;
            for (int i = 0; i < config.num_symbols_per_slot; i++) {
                config.cp_lengths.push_back(ext_cp);
            }
        }
    }

public:
    NumerologyManager(NumerologyIndex mu = NumerologyIndex::MU_1, 
                     CyclicPrefixType cp = CyclicPrefixType::NORMAL) {
        config.mu = mu;
        config.cp_type = cp;
        config.subcarrier_spacing_khz = mu_to_scs[static_cast<int>(mu)];
        
        // FFT size selection based on bandwidth and numerology
        config.fft_size = 2048;  // Common for 20 MHz bandwidth
        config.num_subcarriers = 1200;  // 20 MHz bandwidth
        config.num_symbols_per_slot = (cp == CyclicPrefixType::NORMAL) ? 14 : 12;
        config.num_slots_per_subframe = 1 << static_cast<int>(mu);
        config.slot_duration_ms = 1.0 / config.num_slots_per_subframe;
        
        calculateCPLengths();
    }
    
    const NRConfig& getConfig() const { return config; }
    
    void displayConfiguration() const {
        auto now = chrono::system_clock::now();
        auto time = chrono::system_clock::to_time_t(now);
        
        cout << "\n" << string(80, '=') << endl;
        cout << "[DEV 1] NUMEROLOGY & CONFIGURATION MANAGER" << endl;
        cout << string(80, '=') << endl;
        cout << "Timestamp: " << ctime(&time);
        cout << "┌─────────────────────────────────────────────────────────┐" << endl;
        cout << "│ 5G NR Numerology Configuration (3GPP TS 38.211)        │" << endl;
        cout << "├─────────────────────────────────────────────────────────┤" << endl;
        cout << "│ Numerology Index (μ)      : " << setw(3) << static_cast<int>(config.mu) 
             << "                          │" << endl;
        cout << "│ Subcarrier Spacing        : " << setw(3) << config.subcarrier_spacing_khz 
             << " kHz                      │" << endl;
        cout << "│ FFT Size                  : " << setw(4) << config.fft_size 
             << " points                   │" << endl;
        cout << "│ Number of Subcarriers     : " << setw(4) << config.num_subcarriers 
             << " RBs                      │" << endl;
        cout << "│ Symbols per Slot          : " << setw(2) << config.num_symbols_per_slot 
             << " symbols                    │" << endl;
        cout << "│ Slots per Subframe        : " << setw(2) << config.num_slots_per_subframe 
             << " slots                      │" << endl;
        cout << "│ Slot Duration             : " << fixed << setprecision(3) 
             << config.slot_duration_ms << " ms                        │" << endl;
        cout << "│ Cyclic Prefix Type        : " 
             << (config.cp_type == CyclicPrefixType::NORMAL ? "NORMAL" : "EXTENDED") 
             << "                       │" << endl;
        cout << "└─────────────────────────────────────────────────────────┘" << endl;
        
        cout << "\n[INFO] CP Lengths per Symbol (samples):" << endl;
        for (size_t i = 0; i < config.cp_lengths.size(); i++) {
            cout << "  Symbol " << i << ": " << config.cp_lengths[i] << " samples";
            if (i == 0) cout << " (Extended for first symbol)";
            cout << endl;
        }
    }
};

// ============================================================================
// DEVELOPER 2: DIGITAL MODULATION ENGINE
// Responsible for: QPSK, 16-QAM, 64-QAM, 256-QAM constellation mapping
// ============================================================================

class ModulationEngine {
public:
    enum class ModulationType { QPSK, QAM16, QAM64, QAM256 };
    
private:
    ModulationType mod_type;
    int bits_per_symbol;
    vector<Complex> constellation;
    
    void generateConstellation() {
        constellation.clear();
        
        switch (mod_type) {
            case ModulationType::QPSK:
                bits_per_symbol = 2;
                generateQPSKConstellation();
                break;
            case ModulationType::QAM16:
                bits_per_symbol = 4;
                generateQAM16Constellation();
                break;
            case ModulationType::QAM64:
                bits_per_symbol = 6;
                generateQAM64Constellation();
                break;
            case ModulationType::QAM256:
                bits_per_symbol = 8;
                generateQAM256Constellation();
                break;
        }
    }
    
    void generateQPSKConstellation() {
        // QPSK: 4 constellation points
        // Normalized for unit average power
        double norm = 1.0 / sqrt(2.0);
        constellation = {
            Complex(norm, norm),    // 00
            Complex(norm, -norm),   // 01
            Complex(-norm, norm),   // 10
            Complex(-norm, -norm)   // 11
        };
    }
    
    void generateQAM16Constellation() {
        // 16-QAM: 16 constellation points
        // Gray-coded mapping, normalized for unit average power
        double norm = 1.0 / sqrt(10.0);
        for (int i = 0; i < 16; i++) {
            int re_bits = (i >> 2) & 0x03;  // I component bits
            int im_bits = i & 0x03;          // Q component bits
            
            double re = (2 * (re_bits >> 1) - 1) * (2 * (re_bits & 1) + 1);
            double im = (2 * (im_bits >> 1) - 1) * (2 * (im_bits & 1) + 1);
            
            constellation.push_back(Complex(re * norm, im * norm));
        }
    }
    
    void generateQAM64Constellation() {
        // 64-QAM: 64 constellation points
        double norm = 1.0 / sqrt(42.0);
        for (int i = 0; i < 64; i++) {
            int re_idx = (i >> 3) & 0x07;
            int im_idx = i & 0x07;
            
            double re = 2 * re_idx - 7;
            double im = 2 * im_idx - 7;
            
            constellation.push_back(Complex(re * norm, im * norm));
        }
    }
    
    void generateQAM256Constellation() {
        // 256-QAM: 256 constellation points
        double norm = 1.0 / sqrt(170.0);
        for (int i = 0; i < 256; i++) {
            int re_idx = (i >> 4) & 0x0F;
            int im_idx = i & 0x0F;
            
            double re = 2 * re_idx - 15;
            double im = 2 * im_idx - 15;
            
            constellation.push_back(Complex(re * norm, im * norm));
        }
    }

public:
    ModulationEngine(ModulationType type = ModulationType::QPSK) 
        : mod_type(type) {
        generateConstellation();
    }
    
    vector<Complex> modulate(const vector<int>& bits) {
        vector<Complex> symbols;
        
        for (size_t i = 0; i < bits.size(); i += bits_per_symbol) {
            if (i + bits_per_symbol > bits.size()) break;
            
            int symbol_idx = 0;
            for (int j = 0; j < bits_per_symbol; j++) {
                symbol_idx = (symbol_idx << 1) | bits[i + j];
            }
            
            symbols.push_back(constellation[symbol_idx]);
        }
        
        return symbols;
    }
    
    void displayConstellation() const {
        auto now = chrono::system_clock::now();
        auto time = chrono::system_clock::to_time_t(now);
        
        cout << "\n" << string(80, '=') << endl;
        cout << "[DEV 2] DIGITAL MODULATION ENGINE" << endl;
        cout << string(80, '=') << endl;
        cout << "Timestamp: " << ctime(&time);
        
        string mod_name;
        switch (mod_type) {
            case ModulationType::QPSK: mod_name = "QPSK"; break;
            case ModulationType::QAM16: mod_name = "16-QAM"; break;
            case ModulationType::QAM64: mod_name = "64-QAM"; break;
            case ModulationType::QAM256: mod_name = "256-QAM"; break;
        }
        
        cout << "┌─────────────────────────────────────────────────────────┐" << endl;
        cout << "│ Modulation Scheme: " << left << setw(33) << mod_name << "│" << endl;
        cout << "│ Bits per Symbol:   " << setw(33) << bits_per_symbol << "│" << endl;
        cout << "│ Constellation Size: " << setw(32) << constellation.size() << "│" << endl;
        cout << "└─────────────────────────────────────────────────────────┘" << endl;
        
        cout << "\n[INFO] Constellation Points (I + jQ):" << endl;
        int display_limit = min((int)constellation.size(), 16);
        for (int i = 0; i < display_limit; i++) {
            cout << "  Symbol " << setw(3) << i << ": " 
                 << fixed << setprecision(4)
                 << setw(8) << constellation[i].real() << " + j" 
                 << setw(8) << constellation[i].imag() << endl;
        }
        if (constellation.size() > 16) {
            cout << "  ... (" << constellation.size() - 16 << " more points)" << endl;
        }
        
        // Calculate and display average power
        double avg_power = 0.0;
        for (const auto& sym : constellation) {
            avg_power += norm(sym);
        }
        avg_power /= constellation.size();
        
        cout << "\n[METRIC] Average Symbol Power: " << fixed << setprecision(6) 
             << avg_power << " (normalized)" << endl;
    }
    
    int getBitsPerSymbol() const { return bits_per_symbol; }
    ModulationType getModulationType() const { return mod_type; }
};

// ============================================================================
// DEVELOPER 3: RESOURCE GRID & MAPPING
// Responsible for: Resource block allocation, grid structure, symbol mapping
// ============================================================================

class ResourceGridManager {
private:
    int num_subcarriers;
    int num_ofdm_symbols;
    vector<vector<Complex>> resource_grid;
    
    struct ResourceBlock {
        int start_subcarrier;
        int num_subcarriers;
        int start_symbol;
        int num_symbols;
    };
    
    vector<ResourceBlock> allocated_blocks;

public:
    ResourceGridManager(int subcarriers, int symbols) 
        : num_subcarriers(subcarriers), num_ofdm_symbols(symbols) {
        // Initialize resource grid with zeros
        resource_grid.resize(num_ofdm_symbols, vector<Complex>(num_subcarriers, Complex(0, 0)));
    }
    
    void mapSymbolsToGrid(const vector<Complex>& symbols, int start_sc = 0, int start_sym = 0) {
        int sym_idx = 0;
        
        for (int sym = start_sym; sym < num_ofdm_symbols && sym_idx < symbols.size(); sym++) {
            for (int sc = start_sc; sc < num_subcarriers && sym_idx < symbols.size(); sc++) {
                resource_grid[sym][sc] = symbols[sym_idx++];
            }
        }
        
        // Track allocation
        ResourceBlock rb;
        rb.start_subcarrier = start_sc;
        rb.num_subcarriers = num_subcarriers - start_sc;
        rb.start_symbol = start_sym;
        rb.num_symbols = num_ofdm_symbols - start_sym;
        allocated_blocks.push_back(rb);
    }
    
    vector<Complex> getOFDMSymbol(int symbol_index) const {
        if (symbol_index < 0 || symbol_index >= num_ofdm_symbols) {
            return vector<Complex>();
        }
        return resource_grid[symbol_index];
    }
    
    const vector<vector<Complex>>& getResourceGrid() const {
        return resource_grid;
    }
    
    void displayResourceGrid() const {
        auto now = chrono::system_clock::now();
        auto time = chrono::system_clock::to_time_t(now);
        
        cout << "\n" << string(80, '=') << endl;
        cout << "[DEV 3] RESOURCE GRID & MAPPING" << endl;
        cout << string(80, '=') << endl;
        cout << "Timestamp: " << ctime(&time);
        
        cout << "┌─────────────────────────────────────────────────────────┐" << endl;
        cout << "│ Resource Grid Configuration                            │" << endl;
        cout << "├─────────────────────────────────────────────────────────┤" << endl;
        cout << "│ Grid Dimensions: " << num_ofdm_symbols << " symbols × " 
             << num_subcarriers << " subcarriers      │" << endl;
        cout << "│ Total Resource Elements: " << setw(28) 
             << (num_ofdm_symbols * num_subcarriers) << "│" << endl;
        cout << "│ Allocated Resource Blocks: " << setw(26) 
             << allocated_blocks.size() << "│" << endl;
        cout << "└─────────────────────────────────────────────────────────┘" << endl;
        
        // Calculate resource utilization
        int occupied_res = 0;
        for (const auto& row : resource_grid) {
            for (const auto& element : row) {
                if (abs(element) > 1e-10) occupied_res++;
            }
        }
        
        double utilization = 100.0 * occupied_res / (num_ofdm_symbols * num_subcarriers);
        
        cout << "\n[METRIC] Resource Utilization: " << fixed << setprecision(2) 
             << utilization << "% (" << occupied_res << "/" 
             << (num_ofdm_symbols * num_subcarriers) << " REs)" << endl;
        
        // Display sample of grid
        cout << "\n[INFO] Sample Resource Grid (First 8×8):" << endl;
        cout << "       ";
        for (int sc = 0; sc < min(8, num_subcarriers); sc++) {
            cout << "SC" << setw(2) << sc << "     ";
        }
        cout << endl;
        
        for (int sym = 0; sym < min(8, num_ofdm_symbols); sym++) {
            cout << "Sym" << sym << ": ";
            for (int sc = 0; sc < min(8, num_subcarriers); sc++) {
                Complex val = resource_grid[sym][sc];
                if (abs(val) > 1e-10) {
                    cout << "█ ";
                } else {
                    cout << "· ";
                }
                cout << "      ";
            }
            cout << endl;
        }
    }
};

// ============================================================================
// DEVELOPER 4: IFFT/FFT PROCESSING & SIGNAL GENERATION
// Responsible for: IFFT implementation, frequency-to-time conversion
// ============================================================================

class FFTProcessor {
private:
    int fft_size;
    const double PI = 3.14159265358979323846;
    
    // Cooley-Tukey FFT algorithm
    void fft_recursive(vector<Complex>& x, bool inverse) {
        int N = x.size();
        if (N <= 1) return;
        
        // Divide
        vector<Complex> even(N/2), odd(N/2);
        for (int i = 0; i < N/2; i++) {
            even[i] = x[i*2];
            odd[i] = x[i*2 + 1];
        }
        
        // Conquer
        fft_recursive(even, inverse);
        fft_recursive(odd, inverse);
        
        // Combine
        double angle = 2.0 * PI / N * (inverse ? 1 : -1);
        Complex w(1), wn(cos(angle), sin(angle));
        
        for (int k = 0; k < N/2; k++) {
            Complex t = w * odd[k];
            x[k] = even[k] + t;
            x[k + N/2] = even[k] - t;
            w *= wn;
        }
    }

public:
    FFTProcessor(int size) : fft_size(size) {
        // Ensure FFT size is power of 2
        int power = 1;
        while (power < size) power *= 2;
        fft_size = power;
    }
    
    vector<Complex> ifft(const vector<Complex>& freq_domain) {
        vector<Complex> padded = freq_domain;
        padded.resize(fft_size, Complex(0, 0));
        
        fft_recursive(padded, true);
        
        // Normalize
        for (auto& val : padded) {
            val /= static_cast<double>(fft_size);
        }
        
        return padded;
    }
    
    vector<Complex> fft(const vector<Complex>& time_domain) {
        vector<Complex> padded = time_domain;
        padded.resize(fft_size, Complex(0, 0));
        
        fft_recursive(padded, false);
        
        return padded;
    }
    
    void displayProcessing(const vector<Complex>& input, const vector<Complex>& output) const {
        auto now = chrono::system_clock::now();
        auto time = chrono::system_clock::to_time_t(now);
        
        cout << "\n" << string(80, '=') << endl;
        cout << "[DEV 4] IFFT/FFT PROCESSING & SIGNAL GENERATION" << endl;
        cout << string(80, '=') << endl;
        cout << "Timestamp: " << ctime(&time);
        
        cout << "┌─────────────────────────────────────────────────────────┐" << endl;
        cout << "│ IFFT Processing Configuration                          │" << endl;
        cout << "├─────────────────────────────────────────────────────────┤" << endl;
        cout << "│ FFT Size:          " << setw(35) << fft_size << "│" << endl;
        cout << "│ Input Size:        " << setw(35) << input.size() << "│" << endl;
        cout << "│ Output Size:       " << setw(35) << output.size() << "│" << endl;
        cout << "└─────────────────────────────────────────────────────────┘" << endl;
        
        // Calculate signal power
        double input_power = 0.0, output_power = 0.0;
        for (const auto& val : input) input_power += norm(val);
        for (const auto& val : output) output_power += norm(val);
        
        input_power /= input.size();
        output_power /= output.size();
        
        cout << "\n[METRIC] Signal Power Analysis:" << endl;
        cout << "  Input Power (Freq Domain):  " << fixed << setprecision(6) 
             << input_power << endl;
        cout << "  Output Power (Time Domain): " << fixed << setprecision(6) 
             << output_power << endl;
        cout << "  Power Conservation:         " << fixed << setprecision(2)
             << (output_power / input_power * 100.0) << "%" << endl;
        
        // Display sample values
        cout << "\n[INFO] Time Domain Samples (First 8):" << endl;
        for (int i = 0; i < min(8, (int)output.size()); i++) {
            cout << "  Sample " << setw(4) << i << ": " 
                 << fixed << setprecision(6)
                 << setw(10) << output[i].real() << " + j" 
                 << setw(10) << output[i].imag() << endl;
        }
    }
    
    int getFFTSize() const { return fft_size; }
};

// ============================================================================
// DEVELOPER 5: CYCLIC PREFIX & OFDM SYMBOL ASSEMBLY
// Responsible for: CP addition, OFDM symbol formation, final time-domain signal
// ============================================================================

class CyclicPrefixProcessor {
private:
    vector<int> cp_lengths;
    
public:
    CyclicPrefixProcessor(const vector<int>& cp_lens) : cp_lengths(cp_lens) {}
    
    vector<Complex> addCyclicPrefix(const vector<Complex>& ofdm_symbol, int symbol_index) {
        if (symbol_index >= cp_lengths.size()) {
            symbol_index = cp_lengths.size() - 1;
        }
        
        int cp_len = cp_lengths[symbol_index];
        vector<Complex> symbol_with_cp;
        
        // Copy last cp_len samples to the beginning
        int start = ofdm_symbol.size() - cp_len;
        for (int i = start; i < ofdm_symbol.size(); i++) {
            symbol_with_cp.push_back(ofdm_symbol[i]);
        }
        
        // Copy entire OFDM symbol
        for (const auto& sample : ofdm_symbol) {
            symbol_with_cp.push_back(sample);
        }
        
        return symbol_with_cp;
    }
    
    vector<Complex> assembleSlot(const vector<vector<Complex>>& ofdm_symbols) {
        vector<Complex> slot_signal;
        
        for (size_t i = 0; i < ofdm_symbols.size(); i++) {
            vector<Complex> symbol_with_cp = addCyclicPrefix(ofdm_symbols[i], i);
            slot_signal.insert(slot_signal.end(), symbol_with_cp.begin(), symbol_with_cp.end());
        }
        
        return slot_signal;
    }
    
    void displayCPProcessing(int num_symbols) const {
        auto now = chrono::system_clock::now();
        auto time = chrono::system_clock::to_time_t(now);
        
        cout << "\n" << string(80, '=') << endl;
        cout << "[DEV 5] CYCLIC PREFIX & OFDM SYMBOL ASSEMBLY" << endl;
        cout << string(80, '=') << endl;
        cout << "Timestamp: " << ctime(&time);
        
        cout << "┌─────────────────────────────────────────────────────────┐" << endl;
        cout << "│ Cyclic Prefix Configuration                            │" << endl;
        cout << "├─────────────────────────────────────────────────────────┤" << endl;
        cout << "│ Number of OFDM Symbols: " << setw(32) << num_symbols << "│" << endl;
        cout << "│ CP Type:                " << setw(32) << "NORMAL" << "│" << endl;
        cout << "└─────────────────────────────────────────────────────────┘" << endl;
        
        cout << "\n[INFO] Cyclic Prefix Length per Symbol:" << endl;
        int total_cp_samples = 0;
        for (size_t i = 0; i < cp_lengths.size() && i < num_symbols; i++) {
            cout << "  Symbol " << setw(2) << i << ": " << setw(4) << cp_lengths[i] 
                 << " samples";
            if (i == 0) cout << " (Extended CP)";
            cout << endl;
            total_cp_samples += cp_lengths[i];
        }
        
        cout << "\n[METRIC] CP Overhead Analysis:" << endl;
        cout << "  Total CP Samples: " << total_cp_samples << endl;
        
        // Calculate overhead percentage
        int useful_samples = 2048 * min((int)cp_lengths.size(), num_symbols);
        double overhead = 100.0 * total_cp_samples / (total_cp_samples + useful_samples);
        cout << "  CP Overhead:      " << fixed << setprecision(2) << overhead << "%" << endl;
    }
};

// ============================================================================
// MAIN SYSTEM ORCHESTRATOR
// Integrates all 5 developer modules
// ============================================================================

class OFDMSimulator {
private:
    NumerologyManager num_mgr;
    unique_ptr<ModulationEngine> mod_engine;
    unique_ptr<ResourceGridManager> grid_mgr;
    unique_ptr<FFTProcessor> fft_proc;
    unique_ptr<CyclicPrefixProcessor> cp_proc;
    
    vector<int> generateRandomBits(int num_bits) {
        vector<int> bits;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 1);
        
        for (int i = 0; i < num_bits; i++) {
            bits.push_back(dis(gen));
        }
        
        return bits;
    }

public:
    OFDMSimulator(NumerologyManager::NumerologyIndex mu = NumerologyManager::NumerologyIndex::MU_1,
                  ModulationEngine::ModulationType mod = ModulationEngine::ModulationType::QPSK) {
        num_mgr = NumerologyManager(mu);
        mod_engine = make_unique<ModulationEngine>(mod);
        
        auto config = num_mgr.getConfig();
        grid_mgr = make_unique<ResourceGridManager>(config.num_subcarriers, 
                                                     config.num_symbols_per_slot);
        fft_proc = make_unique<FFTProcessor>(config.fft_size);
        cp_proc = make_unique<CyclicPrefixProcessor>(config.cp_lengths);
    }
    
    void runFullChainSimulation() {
        cout << "\n" << string(80, '*') << endl;
        cout << "**     5G NR OFDM COMPLETE TRANSMISSION CHAIN SIMULATION     **" << endl;
        cout << string(80, '*') << endl;
        
        // Step 1: Display Configuration
        num_mgr.displayConfiguration();
        
        // Step 2: Display Modulation
        mod_engine->displayConstellation();
        
        // Step 3: Generate random data
        int num_bits = 1000;
        cout << "\n[STEP 1] Generating " << num_bits << " random bits..." << endl;
        vector<int> data_bits = generateRandomBits(num_bits);
        
        // Step 4: Modulate
        cout << "[STEP 2] Modulating bits to symbols..." << endl;
        vector<Complex> modulated_symbols = mod_engine->modulate(data_bits);
        cout << "         Generated " << modulated_symbols.size() << " modulated symbols" << endl;
        
        // Step 5: Map to resource grid
        cout << "[STEP 3] Mapping symbols to resource grid..." << endl;
        grid_mgr->mapSymbolsToGrid(modulated_symbols);
        grid_mgr->displayResourceGrid();
        
        // Step 6: Perform IFFT for each OFDM symbol
        cout << "\n[STEP 4] Performing IFFT processing..." << endl;
        vector<vector<Complex>> time_domain_symbols;
        auto config = num_mgr.getConfig();
        
        for (int i = 0; i < config.num_symbols_per_slot; i++) {
            vector<Complex> freq_symbol = grid_mgr->getOFDMSymbol(i);
            vector<Complex> time_symbol = fft_proc->ifft(freq_symbol);
            time_domain_symbols.push_back(time_symbol);
        }
        
        fft_proc->displayProcessing(grid_mgr->getOFDMSymbol(0), time_domain_symbols[0]);
        
        // Step 7: Add cyclic prefix
        cout << "\n[STEP 5] Adding cyclic prefix..." << endl;
        cp_proc->displayCPProcessing(time_domain_symbols.size());
        
        vector<Complex> final_signal = cp_proc->assembleSlot(time_domain_symbols);
        
        // Final statistics
        cout << "\n" << string(80, '=') << endl;
        cout << "TRANSMISSION CHAIN COMPLETE - FINAL STATISTICS" << endl;
        cout << string(80, '=') << endl;
        cout << "┌─────────────────────────────────────────────────────────┐" << endl;
        cout << "│ Transmission Summary                                   │" << endl;
        cout << "├─────────────────────────────────────────────────────────┤" << endl;
        cout << "│ Input Data Bits:        " << setw(32) << num_bits << "│" << endl;
        cout << "│ Modulated Symbols:      " << setw(32) << modulated_symbols.size() << "│" << endl;
        cout << "│ OFDM Symbols Generated: " << setw(32) << time_domain_symbols.size() << "│" << endl;
        cout << "│ Final Signal Samples:   " << setw(32) << final_signal.size() << "│" << endl;
        cout << "│ Bits per Symbol:        " << setw(32) << mod_engine->getBitsPerSymbol() << "│" << endl;
        
        double spectral_efficiency = (double)num_bits / (config.num_subcarriers * config.num_symbols_per_slot);
        cout << "│ Spectral Efficiency:    " << setw(27) << fixed << setprecision(4) 
             << spectral_efficiency << " b/s/Hz│" << endl;
        cout << "└─────────────────────────────────────────────────────────┘" << endl;
        
        // Calculate signal statistics
        double avg_power = 0.0, peak_power = 0.0;
        for (const auto& sample : final_signal) {
            double power = norm(sample);
            avg_power += power;
            peak_power = max(peak_power, power);
        }
        avg_power /= final_signal.size();
        double papr_db = 10 * log10(peak_power / avg_power);
        
        cout << "\n[PERFORMANCE METRICS]" << endl;
        cout << "  Average Signal Power: " << fixed << setprecision(6) << avg_power << endl;
        cout << "  Peak Signal Power:    " << fixed << setprecision(6) << peak_power << endl;
        cout << "  PAPR:                 " << fixed << setprecision(2) << papr_db << " dB" << endl;
        
        cout << "\n[SUCCESS] ✓ Complete OFDM transmission chain executed successfully!" << endl;
    }
};

// ============================================================================
// MAIN FUNCTION - INTERACTIVE MENU
// ============================================================================

void displayMainMenu() {
    cout << "\n" << string(80, '=') << endl;
    cout << "       5G NR PHYSICAL LAYER - OFDM MODULATION SIMULATOR" << endl;
    cout << "                  Team-Based Implementation" << endl;
    cout << string(80, '=') << endl;
    cout << "\nDEVELOPER MODULE OPTIONS:" << endl;
    cout << "  1. [DEV 1] Display Numerology Configuration (μ=0,1,2,3,4)" << endl;
    cout << "  2. [DEV 2] QPSK Modulation & Constellation" << endl;
    cout << "  3. [DEV 2] 16-QAM Modulation & Constellation" << endl;
    cout << "  4. [DEV 2] 64-QAM Modulation & Constellation" << endl;
    cout << "  5. [DEV 2] 256-QAM Modulation & Constellation" << endl;
    cout << "  6. [DEV 3] Resource Grid Mapping Demonstration" << endl;
    cout << "  7. [DEV 4] IFFT Processing Demonstration" << endl;
    cout << "  8. [DEV 5] Cyclic Prefix Addition Demonstration" << endl;
    cout << "\nFULL SYSTEM SIMULATIONS:" << endl;
    cout << "  9. Complete OFDM Chain (QPSK, μ=1, 30kHz SCS)" << endl;
    cout << " 10. Complete OFDM Chain (16-QAM, μ=1, 30kHz SCS)" << endl;
    cout << " 11. Complete OFDM Chain (64-QAM, μ=2, 60kHz SCS)" << endl;
    cout << " 12. Performance Comparison (All Modulations)" << endl;
    cout << "  0. Exit" << endl;
    cout << string(80, '=') << endl;
    cout << "Enter your choice: ";
}

void runPerformanceComparison() {
    cout << "\n" << string(80, '*') << endl;
    cout << "**        PERFORMANCE COMPARISON - ALL MODULATION SCHEMES       **" << endl;
    cout << string(80, '*') << endl;
    
    vector<ModulationEngine::ModulationType> mod_types = {
        ModulationEngine::ModulationType::QPSK,
        ModulationEngine::ModulationType::QAM16,
        ModulationEngine::ModulationType::QAM64,
        ModulationEngine::ModulationType::QAM256
    };
    
    vector<string> mod_names = {"QPSK", "16-QAM", "64-QAM", "256-QAM"};
    
    cout << "\n┌───────────┬────────────┬─────────────┬──────────────────┐" << endl;
    cout << "│ Modulation│ Bits/Symbol│ Constellation│ Spectral Efficiency│" << endl;
    cout << "│  Scheme   │            │    Size     │    (b/s/Hz)        │" << endl;
    cout << "├───────────┼────────────┼─────────────┼──────────────────┤" << endl;
    
    for (size_t i = 0; i < mod_types.size(); i++) {
        ModulationEngine engine(mod_types[i]);
        int bps = engine.getBitsPerSymbol();
        int const_size = (1 << bps);
        double spec_eff = bps * 0.85;  // Typical coding rate adjustment
        
        cout << "│ " << left << setw(10) << mod_names[i]
             << "│ " << setw(11) << bps
             << "│ " << setw(12) << const_size
             << "│ " << setw(17) << fixed << setprecision(2) << spec_eff
             << "│" << endl;
    }
    
    cout << "└───────────┴────────────┴─────────────┴──────────────────┘" << endl;
    
    cout << "\n[ANALYSIS] Trade-offs:" << endl;
    cout << "  • Higher-order modulation → Higher data rates" << endl;
    cout << "  • Higher-order modulation → Lower noise tolerance" << endl;
    cout << "  • QPSK: Most robust, suitable for cell edge users" << endl;
    cout << "  • 256-QAM: Highest throughput, requires excellent SNR" << endl;
}

int main() {
    int choice;
    
    while (true) {
        displayMainMenu();
        cin >> choice;
        
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Invalid input. Please enter a number." << endl;
            continue;
        }
        
        switch (choice) {
            case 0:
                cout << "\n[INFO] Exiting simulator. Goodbye!" << endl;
                return 0;
                
            case 1: {
                int mu_choice;
                cout << "\nSelect Numerology Index (0-4): ";
                cin >> mu_choice;
                NumerologyManager mgr(static_cast<NumerologyManager::NumerologyIndex>(mu_choice));
                mgr.displayConfiguration();
                break;
            }
            
            case 2: {
                ModulationEngine engine(ModulationEngine::ModulationType::QPSK);
                engine.displayConstellation();
                break;
            }
            
            case 3: {
                ModulationEngine engine(ModulationEngine::ModulationType::QAM16);
                engine.displayConstellation();
                break;
            }
            
            case 4: {
                ModulationEngine engine(ModulationEngine::ModulationType::QAM64);
                engine.displayConstellation();
                break;
            }
            
            case 5: {
                ModulationEngine engine(ModulationEngine::ModulationType::QAM256);
                engine.displayConstellation();
                break;
            }
            
            case 6: {
                ResourceGridManager grid(1200, 14);
                vector<Complex> test_symbols(100, Complex(1.0, 0.5));
                grid.mapSymbolsToGrid(test_symbols);
                grid.displayResourceGrid();
                break;
            }
            
            case 7: {
                FFTProcessor fft(2048);
                vector<Complex> freq_data(1200, Complex(1.0, 0.0));
                vector<Complex> time_data = fft.ifft(freq_data);
                fft.displayProcessing(freq_data, time_data);
                break;
            }
            
            case 8: {
                vector<int> cp_lens = {144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144};
                CyclicPrefixProcessor cp(cp_lens);
                cp.displayCPProcessing(14);
                break;
            }
            
            case 9: {
                OFDMSimulator sim(NumerologyManager::NumerologyIndex::MU_1,
                                ModulationEngine::ModulationType::QPSK);
                sim.runFullChainSimulation();
                break;
            }
            
            case 10: {
                OFDMSimulator sim(NumerologyManager::NumerologyIndex::MU_1,
                                ModulationEngine::ModulationType::QAM16);
                sim.runFullChainSimulation();
                break;
            }
            
            case 11: {
                OFDMSimulator sim(NumerologyManager::NumerologyIndex::MU_2,
                                ModulationEngine::ModulationType::QAM64);
                sim.runFullChainSimulation();
                break;
            }
            
            case 12: {
                runPerformanceComparison();
                break;
            }
            
            default:
                cout << "\n[ERROR] Invalid choice. Please select 0-12." << endl;
        }
        
        cout << "\nPress Enter to continue...";
        cin.ignore();
        cin.get();
    }
    
    return 0;
}
