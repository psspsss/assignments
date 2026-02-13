/*
 * 5G NR Physical Layer - Channel Coding Simulator
 * Reference: 3GPP TS 38.212
 * Team-Based Implementation - 5 Developers
 * 
 * Module Distribution:
 * DEV 1: CRC Attachment (CRC24A, CRC16, CRC6)
 * DEV 2: LDPC Encoder (Base Graph 1/2)
 * DEV 3: Polar Encoder (Control Channels)
 * DEV 4: Rate Matching & HARQ
 * DEV 5: Code Block Segmentation
 */

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace std;

// ============================================================================
// DEVELOPER 1: CRC ATTACHMENT
// Responsible for: CRC polynomials, CRC calculation and attachment
// ============================================================================

class CRCProcessor {
public:
    enum CRCType { CRC24A, CRC24B, CRC16, CRC11, CRC6 };
    
private:
    // CRC polynomials (3GPP TS 38.212 Section 5.1)
    const uint32_t CRC24A_POLY = 0x1864CFB;  // x^24 + x^23 + x^18 + ...
    const uint32_t CRC24B_POLY = 0x1800063;  
    const uint32_t CRC16_POLY = 0x11021;
    const uint32_t CRC11_POLY = 0xE21;
    const uint32_t CRC6_POLY = 0x61;
    
public:
    vector<uint8_t> attachCRC(const vector<uint8_t>& data, CRCType type) {
        vector<uint8_t> result = data;
        vector<uint8_t> crc = calculateCRC(data, type);
        result.insert(result.end(), crc.begin(), crc.end());
        return result;
    }
    
    vector<uint8_t> calculateCRC(const vector<uint8_t>& data, CRCType type) {
        int crc_len = getCRCLength(type);
        uint32_t poly = getPolynomial(type);
        
        vector<uint8_t> padded = data;
        padded.resize(data.size() + crc_len, 0);
        
        for (size_t i = 0; i < data.size(); i++) {
            if (padded[i]) {
                for (int j = 0; j < crc_len; j++) {
                    if ((poly >> (crc_len - 1 - j)) & 1) {
                        padded[i + j] ^= 1;
                    }
                }
            }
        }
        
        return vector<uint8_t>(padded.end() - crc_len, padded.end());
    }
    
    bool verifyCRC(const vector<uint8_t>& data_with_crc, CRCType type) {
        int crc_len = getCRCLength(type);
        vector<uint8_t> data(data_with_crc.begin(), data_with_crc.end() - crc_len);
        vector<uint8_t> received_crc(data_with_crc.end() - crc_len, data_with_crc.end());
        vector<uint8_t> calculated_crc = calculateCRC(data, type);
        return received_crc == calculated_crc;
    }
    
    void displayCRCInfo(const vector<uint8_t>& data, CRCType type) {
        auto now = chrono::system_clock::now();
        auto time_t = chrono::system_clock::to_time_t(now);
        
        cout << "\n" << string(80, '=') << endl;
        cout << "[DEV 1] CRC ATTACHMENT MODULE" << endl;
        cout << string(80, '=') << endl;
        cout << "Timestamp: " << ctime(&time_t);
        
        string type_name = getCRCName(type);
        int crc_len = getCRCLength(type);
        
        cout << "CRC Type:           " << type_name << endl;
        cout << "CRC Length:         " << crc_len << " bits" << endl;
        cout << "Input Data Length:  " << data.size() << " bits" << endl;
        cout << "Output Length:      " << (data.size() + crc_len) << " bits" << endl;
        
        vector<uint8_t> crc = calculateCRC(data, type);
        cout << "CRC Value:          ";
        for (auto bit : crc) cout << (int)bit;
        cout << endl;
    }

private:
    int getCRCLength(CRCType type) {
        switch(type) {
            case CRC24A: case CRC24B: return 24;
            case CRC16: return 16;
            case CRC11: return 11;
            case CRC6: return 6;
            default: return 24;
        }
    }
    
    uint32_t getPolynomial(CRCType type) {
        switch(type) {
            case CRC24A: return CRC24A_POLY;
            case CRC24B: return CRC24B_POLY;
            case CRC16: return CRC16_POLY;
            case CRC11: return CRC11_POLY;
            case CRC6: return CRC6_POLY;
            default: return CRC24A_POLY;
        }
    }
    
    string getCRCName(CRCType type) {
        switch(type) {
            case CRC24A: return "CRC-24A";
            case CRC24B: return "CRC-24B";
            case CRC16: return "CRC-16";
            case CRC11: return "CRC-11";
            case CRC6: return "CRC-6";
            default: return "CRC-24A";
        }
    }
};

// ============================================================================
// DEVELOPER 2: LDPC ENCODER
// Responsible for: LDPC encoding with Base Graph selection (BG1/BG2)
// ============================================================================

class LDPCEncoder {
public:
    enum BaseGraph { BG1, BG2 };
    
private:
    // LDPC parameters (3GPP TS 38.212 Section 5.3.2)
    static const int BG1_KB = 22;  // Information columns
    static const int BG1_N = 66;   // Total columns (including parity)
    static const int BG2_KB = 10;
    static const int BG2_N = 50;
    
    BaseGraph selected_bg;
    int lifting_size;
    
public:
    LDPCEncoder() : selected_bg(BG1), lifting_size(384) {}
    
    BaseGraph selectBaseGraph(int tbs, int code_rate_x100) {
        // Base Graph selection (3GPP TS 38.212 Section 5.2.2)
        if (tbs <= 292 || (tbs <= 3824 && code_rate_x100 <= 67)) {
            selected_bg = BG2;
        } else {
            selected_bg = BG1;
        }
        return selected_bg;
    }
    
    vector<uint8_t> encode(const vector<uint8_t>& info_bits) {
        int kb = (selected_bg == BG1) ? BG1_KB : BG2_KB;
        int n = (selected_bg == BG1) ? BG1_N : BG2_N;
        
        // Simplified LDPC encoding (systematic)
        vector<uint8_t> codeword = info_bits;
        
        // Calculate parity bits (simplified - actual uses parity check matrix)
        int num_parity = (n - kb) * lifting_size;
        vector<uint8_t> parity(num_parity, 0);
        
        // Simple parity generation for demonstration
        for (size_t i = 0; i < info_bits.size(); i++) {
            parity[i % num_parity] ^= info_bits[i];
        }
        
        codeword.insert(codeword.end(), parity.begin(), parity.end());
        return codeword;
    }
    
    void displayLDPCInfo(int tbs) {
        auto now = chrono::system_clock::now();
        auto time_t = chrono::system_clock::to_time_t(now);
        
        cout << "\n" << string(80, '=') << endl;
        cout << "[DEV 2] LDPC ENCODER MODULE" << endl;
        cout << string(80, '=') << endl;
        cout << "Timestamp: " << ctime(&time_t);
        
        string bg_name = (selected_bg == BG1) ? "Base Graph 1" : "Base Graph 2";
        int kb = (selected_bg == BG1) ? BG1_KB : BG2_KB;
        int n = (selected_bg == BG1) ? BG1_N : BG2_N;
        
        cout << "Selected Base Graph: " << bg_name << endl;
        cout << "Transport Block Size: " << tbs << " bits" << endl;
        cout << "Information Columns:  " << kb << endl;
        cout << "Total Columns:        " << n << endl;
        cout << "Lifting Size:         " << lifting_size << endl;
        cout << "Code Rate:            " << fixed << setprecision(3) 
             << (double)kb/n << endl;
    }
    
    BaseGraph getBaseGraph() const { return selected_bg; }
};

// ============================================================================
// DEVELOPER 3: POLAR ENCODER
// Responsible for: Polar coding for control channels with frozen bits
// ============================================================================

class PolarEncoder {
private:
    int n_max;  // Maximum code length (power of 2)
    vector<int> frozen_bits;
    
public:
    PolarEncoder() : n_max(512) {}
    
    vector<uint8_t> encode(const vector<uint8_t>& info_bits, int E) {
        // Determine code length N (nearest power of 2 >= E)
        int N = 1;
        while (N < E) N *= 2;
        
        // Generate frozen bit positions (simplified)
        generateFrozenBits(info_bits.size(), N);
        
        // Polar encoding using generator matrix
        vector<uint8_t> u(N, 0);  // Input to polar encoder
        
        // Place information bits in non-frozen positions
        int info_idx = 0;
        for (int i = 0; i < N; i++) {
            if (find(frozen_bits.begin(), frozen_bits.end(), i) == frozen_bits.end()) {
                if (info_idx < info_bits.size()) {
                    u[i] = info_bits[info_idx++];
                }
            }
        }
        
        // Polar transform (simplified Arikan encoding)
        vector<uint8_t> encoded = polarTransform(u);
        
        // Rate matching to E bits
        encoded.resize(E);
        return encoded;
    }
    
    void displayPolarInfo(int K, int E) {
        auto now = chrono::system_clock::now();
        auto time_t = chrono::system_clock::to_time_t(now);
        
        cout << "\n" << string(80, '=') << endl;
        cout << "[DEV 3] POLAR ENCODER MODULE" << endl;
        cout << string(80, '=') << endl;
        cout << "Timestamp: " << ctime(&time_t);
        
        int N = 1;
        while (N < E) N *= 2;
        
        cout << "Information Bits (K): " << K << endl;
        cout << "Code Length (N):      " << N << endl;
        cout << "Output Length (E):    " << E << endl;
        cout << "Code Rate:            " << fixed << setprecision(3) 
             << (double)K/E << endl;
        cout << "Number of Frozen Bits: " << frozen_bits.size() << endl;
        
        cout << "Frozen Bit Positions: ";
        for (int i = 0; i < min(10, (int)frozen_bits.size()); i++) {
            cout << frozen_bits[i] << " ";
        }
        if (frozen_bits.size() > 10) cout << "...";
        cout << endl;
    }

private:
    void generateFrozenBits(int K, int N) {
        frozen_bits.clear();
        // Simplified: freeze first (N-K) positions
        for (int i = 0; i < N - K; i++) {
            frozen_bits.push_back(i);
        }
    }
    
    vector<uint8_t> polarTransform(const vector<uint8_t>& u) {
        int n = u.size();
        if (n == 1) return u;
        
        vector<uint8_t> result(n);
        
        // Simplified polar transform (Kronecker product based)
        for (int stage = 1; stage < n; stage *= 2) {
            for (int i = 0; i < n; i += 2 * stage) {
                for (int j = 0; j < stage; j++) {
                    uint8_t u1 = (i + j < n) ? u[i + j] : 0;
                    uint8_t u2 = (i + j + stage < n) ? u[i + j + stage] : 0;
                    result[i + j] = u1 ^ u2;
                    result[i + j + stage] = u2;
                }
            }
        }
        
        return result;
    }
};

// ============================================================================
// DEVELOPER 4: RATE MATCHING & HARQ
// Responsible for: Rate matching, redundancy versions, HARQ combining
// ============================================================================

class RateMatchingEngine {
public:
    enum RedundancyVersion { RV0 = 0, RV1 = 1, RV2 = 2, RV3 = 3 };
    
private:
    RedundancyVersion current_rv;
    
public:
    RateMatchingEngine() : current_rv(RV0) {}
    
    vector<uint8_t> rateMatch(const vector<uint8_t>& coded_bits, int E, 
                              RedundancyVersion rv) {
        current_rv = rv;
        vector<uint8_t> output;
        
        // Interleaving (simplified sub-block interleaving)
        vector<uint8_t> interleaved = subBlockInterleave(coded_bits);
        
        // Circular buffer and bit selection based on RV
        int k0 = calculateK0(coded_bits.size(), rv);
        
        for (int i = 0; i < E; i++) {
            int idx = (k0 + i) % interleaved.size();
            output.push_back(interleaved[idx]);
        }
        
        return output;
    }
    
    void displayRateMatchingInfo(int input_len, int output_len, RedundancyVersion rv) {
        auto now = chrono::system_clock::now();
        auto time_t = chrono::system_clock::to_time_t(now);
        
        cout << "\n" << string(80, '=') << endl;
        cout << "[DEV 4] RATE MATCHING & HARQ MODULE" << endl;
        cout << string(80, '=') << endl;
        cout << "Timestamp: " << ctime(&time_t);
        
        cout << "Input Length (coded): " << input_len << " bits" << endl;
        cout << "Output Length (E):    " << output_len << " bits" << endl;
        cout << "Redundancy Version:   RV" << (int)rv << endl;
        cout << "Starting Position k0: " << calculateK0(input_len, rv) << endl;
        
        double code_rate = (double)output_len / input_len;
        cout << "Effective Code Rate:  " << fixed << setprecision(3) 
             << code_rate << endl;
        
        if (code_rate > 1.0) {
            cout << "Mode: Repetition (rate > 1)" << endl;
        } else {
            cout << "Mode: Puncturing (rate < 1)" << endl;
        }
    }

private:
    vector<uint8_t> subBlockInterleave(const vector<uint8_t>& bits) {
        // Simplified interleaving for demonstration
        return bits;
    }
    
    int calculateK0(int N, RedundancyVersion rv) {
        // Starting position calculation based on RV (3GPP TS 38.212)
        int k0_table[4] = {0, (int)(0.25 * N), (int)(0.5 * N), (int)(0.75 * N)};
        return k0_table[rv];
    }
};

// ============================================================================
// DEVELOPER 5: CODE BLOCK SEGMENTATION
// Responsible for: Segmentation of transport blocks into code blocks
// ============================================================================

class CodeBlockSegmenter {
private:
    static const int MAX_CB_SIZE_LDPC = 8448;  // Maximum code block size
    
public:
    struct CodeBlock {
        int cb_index;
        vector<uint8_t> data;
        int filler_bits;
    };
    
    vector<CodeBlock> segment(const vector<uint8_t>& transport_block) {
        vector<CodeBlock> code_blocks;
        int B = transport_block.size();
        
        // Calculate number of code blocks (3GPP TS 38.212 Section 5.2.2)
        int C = 1;  // Number of code blocks
        int K_dash = MAX_CB_SIZE_LDPC;
        
        if (B > MAX_CB_SIZE_LDPC) {
            C = (int)ceil((double)B / (MAX_CB_SIZE_LDPC - 24));  // -24 for CRC
            K_dash = (int)ceil((double)(B + 24 * C) / C);
        } else {
            K_dash = B;
        }
        
        // Calculate filler bits
        int total_bits = C * K_dash;
        int filler_bits = total_bits - B;
        
        // Segment transport block
        int offset = 0;
        for (int i = 0; i < C; i++) {
            CodeBlock cb;
            cb.cb_index = i;
            cb.filler_bits = (i == 0) ? filler_bits : 0;
            
            int cb_size = K_dash;
            if (i == C - 1) {
                cb_size = B - offset;
            } else {
                cb_size = K_dash - ((i == 0) ? filler_bits : 0);
            }
            
            cb.data = vector<uint8_t>(
                transport_block.begin() + offset,
                transport_block.begin() + offset + cb_size
            );
            
            offset += cb_size;
            code_blocks.push_back(cb);
        }
        
        return code_blocks;
    }
    
    void displaySegmentationInfo(int tbs, const vector<CodeBlock>& blocks) {
        auto now = chrono::system_clock::now();
        auto time_t = chrono::system_clock::to_time_t(now);
        
        cout << "\n" << string(80, '=') << endl;
        cout << "[DEV 5] CODE BLOCK SEGMENTATION MODULE" << endl;
        cout << string(80, '=') << endl;
        cout << "Timestamp: " << ctime(&time_t);
        
        cout << "Transport Block Size: " << tbs << " bits" << endl;
        cout << "Number of Code Blocks: " << blocks.size() << endl;
        cout << "Max CB Size:          " << MAX_CB_SIZE_LDPC << " bits" << endl;
        
        int total_filler = 0;
        for (const auto& cb : blocks) {
            total_filler += cb.filler_bits;
        }
        cout << "Total Filler Bits:    " << total_filler << endl;
        
        cout << "\nCode Block Details:" << endl;
        for (const auto& cb : blocks) {
            cout << "  CB " << cb.cb_index << ": " << cb.data.size() 
                 << " bits";
            if (cb.filler_bits > 0) {
                cout << " (+" << cb.filler_bits << " filler)";
            }
            cout << endl;
        }
    }
};

// ============================================================================
// MAIN SYSTEM ORCHESTRATOR
// Integrates all 5 developer modules
// ============================================================================

class ChannelCodingSimulator {
private:
    CRCProcessor crc_proc;
    LDPCEncoder ldpc_enc;
    PolarEncoder polar_enc;
    RateMatchingEngine rate_match;
    CodeBlockSegmenter cb_seg;
    
public:
    void runLDPCTest() {
        cout << "\n" << string(80, '*') << endl;
        cout << "**        LDPC CODING COMPLETE CHAIN TEST             **" << endl;
        cout << string(80, '*') << endl;
        
        // Generate test data
        int tbs = 5000;  // Transport block size
        vector<uint8_t> transport_block = generateRandomBits(tbs);
        
        cout << "\n[STEP 1] Code Block Segmentation..." << endl;
        auto code_blocks = cb_seg.segment(transport_block);
        cb_seg.displaySegmentationInfo(tbs, code_blocks);
        
        cout << "\n[STEP 2] CRC Attachment..." << endl;
        auto cb_with_crc = crc_proc.attachCRC(code_blocks[0].data, 
                                               CRCProcessor::CRC24A);
        crc_proc.displayCRCInfo(code_blocks[0].data, CRCProcessor::CRC24A);
        
        cout << "\n[STEP 3] LDPC Encoding..." << endl;
        ldpc_enc.selectBaseGraph(tbs, 50);  // 50% code rate
        auto ldpc_coded = ldpc_enc.encode(cb_with_crc);
        ldpc_enc.displayLDPCInfo(tbs);
        
        cout << "\n[STEP 4] Rate Matching..." << endl;
        int E = 10000;  // Output length
        auto rate_matched = rate_match.rateMatch(ldpc_coded, E, 
                                                  RateMatchingEngine::RV0);
        rate_match.displayRateMatchingInfo(ldpc_coded.size(), E, 
                                           RateMatchingEngine::RV0);
        
        cout << "\n[SUCCESS] ✓ LDPC coding chain completed successfully!" << endl;
    }
    
    void runPolarTest() {
        cout << "\n" << string(80, '*') << endl;
        cout << "**        POLAR CODING TEST (CONTROL CHANNEL)        **" << endl;
        cout << string(80, '*') << endl;
        
        int K = 100;  // Information bits
        int E = 256;  // Coded output length
        
        vector<uint8_t> info_bits = generateRandomBits(K);
        
        cout << "\n[STEP 1] CRC Attachment (CRC-11 for DCI)..." << endl;
        auto bits_with_crc = crc_proc.attachCRC(info_bits, CRCProcessor::CRC11);
        crc_proc.displayCRCInfo(info_bits, CRCProcessor::CRC11);
        
        cout << "\n[STEP 2] Polar Encoding..." << endl;
        auto polar_coded = polar_enc.encode(bits_with_crc, E);
        polar_enc.displayPolarInfo(bits_with_crc.size(), E);
        
        cout << "\n[SUCCESS] ✓ Polar coding completed successfully!" << endl;
    }
    
    void runFullChain() {
        cout << "\n" << string(80, '*') << endl;
        cout << "**    COMPLETE CHANNEL CODING CHAIN (PDSCH)          **" << endl;
        cout << string(80, '*') << endl;
        
        int tbs = 3000;
        vector<uint8_t> transport_block = generateRandomBits(tbs);
        
        cout << "\n[CHAIN] Processing transport block through full coding chain..." << endl;
        
        // Segmentation
        auto code_blocks = cb_seg.segment(transport_block);
        cout << "  → Segmented into " << code_blocks.size() << " code blocks" << endl;
        
        // Process each code block
        for (const auto& cb : code_blocks) {
            // CRC
            auto with_crc = crc_proc.attachCRC(cb.data, CRCProcessor::CRC24A);
            cout << "  → CB" << cb.cb_index << ": CRC-24A attached" << endl;
            
            // LDPC
            ldpc_enc.selectBaseGraph(cb.data.size(), 67);
            auto coded = ldpc_enc.encode(with_crc);
            cout << "  → CB" << cb.cb_index << ": LDPC encoded (BG" 
                 << (ldpc_enc.getBaseGraph() == LDPCEncoder::BG1 ? "1" : "2") 
                 << ")" << endl;
            
            // Rate matching
            int E = (int)(coded.size() * 0.8);  // 80% rate matching
            auto matched = rate_match.rateMatch(coded, E, RateMatchingEngine::RV0);
            cout << "  → CB" << cb.cb_index << ": Rate matched to " 
                 << E << " bits" << endl;
        }
        
        cb_seg.displaySegmentationInfo(tbs, code_blocks);
        ldpc_enc.displayLDPCInfo(tbs);
        
        cout << "\n[SUCCESS] ✓ Complete coding chain executed successfully!" << endl;
    }

private:
    vector<uint8_t> generateRandomBits(int n) {
        vector<uint8_t> bits(n);
        for (int i = 0; i < n; i++) {
            bits[i] = rand() % 2;
        }
        return bits;
    }
};

// ============================================================================
// MAIN FUNCTION - INTERACTIVE MENU
// ============================================================================

void displayMainMenu() {
    cout << "\n" << string(80, '=') << endl;
    cout << "    5G NR PHYSICAL LAYER - CHANNEL CODING SIMULATOR" << endl;
    cout << "                Team-Based Implementation" << endl;
    cout << string(80, '=') << endl;
    cout << "\nDEVELOPER MODULE OPTIONS:" << endl;
    cout << "  1. [DEV 1] Test CRC Attachment" << endl;
    cout << "  2. [DEV 2] Test LDPC Encoder" << endl;
    cout << "  3. [DEV 3] Test Polar Encoder" << endl;
    cout << "  4. [DEV 4] Test Rate Matching" << endl;
    cout << "  5. [DEV 5] Test Code Block Segmentation" << endl;
    cout << "\nFULL SYSTEM SIMULATIONS:" << endl;
    cout << "  6. Complete LDPC Coding Chain (PDSCH)" << endl;
    cout << "  7. Complete Polar Coding Test (Control)" << endl;
    cout << "  8. Full Coding Chain with All Modules" << endl;
    cout << "  0. Exit" << endl;
    cout << string(80, '=') << endl;
    cout << "Enter your choice: ";
}

int main() {
    ChannelCodingSimulator sim;
    int choice;
    
    while (true) {
        displayMainMenu();
        cin >> choice;
        
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Invalid input." << endl;
            continue;
        }
        
        switch (choice) {
            case 0:
                cout << "\n[INFO] Exiting simulator." << endl;
                return 0;
                
            case 1: {
                CRCProcessor crc;
                auto data = vector<uint8_t>(100, 1);
                crc.displayCRCInfo(data, CRCProcessor::CRC24A);
                break;
            }
            
            case 2: {
                LDPCEncoder ldpc;
                ldpc.selectBaseGraph(5000, 50);
                ldpc.displayLDPCInfo(5000);
                break;
            }
            
            case 3: {
                PolarEncoder polar;
                polar.displayPolarInfo(100, 256);
                break;
            }
            
            case 4: {
                RateMatchingEngine rm;
                auto bits = vector<uint8_t>(1000, 1);
                rm.displayRateMatchingInfo(1000, 800, RateMatchingEngine::RV0);
                break;
            }
            
            case 5: {
                CodeBlockSegmenter seg;
                auto tb = vector<uint8_t>(10000, 1);
                auto blocks = seg.segment(tb);
                seg.displaySegmentationInfo(10000, blocks);
                break;
            }
            
            case 6:
                sim.runLDPCTest();
                break;
                
            case 7:
                sim.runPolarTest();
                break;
                
            case 8:
                sim.runFullChain();
                break;
                
            default:
                cout << "\n[ERROR] Invalid choice." << endl;
        }
        
        cout << "\nPress Enter to continue...";
        cin.ignore();
        cin.get();
    }
    
    return 0;
}
