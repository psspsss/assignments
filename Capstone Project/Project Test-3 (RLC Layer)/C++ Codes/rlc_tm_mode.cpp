/*
 * 5G NR RLC Layer - TM (Transparent Mode) Simulator
 * Reference: 3GPP TS 38.322
 * 
 * TM Mode: Simplest RLC mode with no segmentation, ARQ, or reassembly.
 * Direct pass-through of SDUs to MAC layer.
 */

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <random>

using namespace std;

// ============================================================================
// SIMPLE LOGGING SYSTEM
// ============================================================================
class Logger {
private:
    static chrono::steady_clock::time_point start_time;
    static bool initialized;

    static string getSimTime() {
        if (!initialized) {
            start_time = chrono::steady_clock::now();
            initialized = true;
        }
        auto now = chrono::steady_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(now - start_time);
        return to_string(duration.count()) + "ms";
    }

public:
    static void log(const string& level, const string& message) {
        cout << "[" << setw(8) << getSimTime() << "] [" << level << "] " << message << endl;
    }
};

chrono::steady_clock::time_point Logger::start_time;
bool Logger::initialized = false;

// ============================================================================
// RLC TM MODE IMPLEMENTATION
// ============================================================================
class RLC_TM {
private:
    int node_id;
    int bearer_id;
    int sdu_count;
    int total_bytes_tx;
    int total_bytes_rx;

public:
    RLC_TM(int node, int bearer) 
        : node_id(node), bearer_id(bearer), sdu_count(0), 
          total_bytes_tx(0), total_bytes_rx(0) {
        Logger::log("INFO", "RLC TM Mode initialized for Node=" + to_string(node_id) + 
                    ", Bearer=" + to_string(bearer_id));
    }

    // Transmit SDU (direct pass-through)
    void transmitSDU(const vector<uint8_t>& sdu_data) {
        sdu_count++;
        total_bytes_tx += sdu_data.size();
        
        Logger::log("TX", "TM-SDU #" + to_string(sdu_count) + 
                    " | Size=" + to_string(sdu_data.size()) + " bytes" +
                    " | Node=" + to_string(node_id));
        
        // In TM mode, SDU = PDU (no header, no segmentation)
        transmitPDU(sdu_data);
    }

    // Transmit PDU to MAC layer
    void transmitPDU(const vector<uint8_t>& pdu_data) {
        Logger::log("TX→MAC", "TM-PDU transmitted | Size=" + to_string(pdu_data.size()) + 
                    " bytes (No RLC header in TM mode)");
    }

    // Receive PDU from MAC layer
    void receivePDU(const vector<uint8_t>& pdu_data) {
        total_bytes_rx += pdu_data.size();
        
        Logger::log("RX←MAC", "TM-PDU received | Size=" + to_string(pdu_data.size()) + " bytes");
        
        // Direct delivery to upper layer (no reassembly)
        deliverSDU(pdu_data);
    }

    // Deliver SDU to upper layer (PDCP)
    void deliverSDU(const vector<uint8_t>& sdu_data) {
        Logger::log("RX→PDCP", "TM-SDU delivered to PDCP | Size=" + 
                    to_string(sdu_data.size()) + " bytes");
    }

    void displayStatistics() {
        cout << "\n" << string(60, '=') << endl;
        cout << "        RLC TM MODE STATISTICS" << endl;
        cout << string(60, '=') << endl;
        cout << "  Node ID:              " << node_id << endl;
        cout << "  Bearer ID:            " << bearer_id << endl;
        cout << "  SDUs Transmitted:     " << sdu_count << endl;
        cout << "  Total Bytes TX:       " << total_bytes_tx << " bytes" << endl;
        cout << "  Total Bytes RX:       " << total_bytes_rx << " bytes" << endl;
        cout << "\n  TM Mode Characteristics:" << endl;
        cout << "    ✓ No segmentation or reassembly" << endl;
        cout << "    ✓ No RLC headers added" << endl;
        cout << "    ✓ No ARQ (no retransmissions)" << endl;
        cout << "    ✓ Direct SDU ↔ PDU mapping" << endl;
        cout << "    ✓ Lowest overhead and latency" << endl;
        cout << string(60, '=') << endl;
    }
};

// ============================================================================
// TEST SCENARIO
// ============================================================================
void runTMTest() {
    cout << "\n" << string(60, '*') << endl;
    cout << "   5G NR RLC - TRANSPARENT MODE (TM) TEST" << endl;
    cout << string(60, '*') << endl;
    
    Logger::log("INFO", "Starting RLC TM Mode Test Scenario");
    
    // Create RLC TM entity
    RLC_TM rlc_tm(1, 5);
    
    // Generate test data
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(100, 1000);
    
    cout << "\n[TEST 1] Transmitting SDUs in TM Mode..." << endl;
    
    // Transmit multiple SDUs
    for (int i = 0; i < 5; i++) {
        int size = dis(gen);
        vector<uint8_t> sdu_data(size, 0xAA + i);
        rlc_tm.transmitSDU(sdu_data);
    }
    
    cout << "\n[TEST 2] Receiving PDUs in TM Mode..." << endl;
    
    // Simulate reception
    for (int i = 0; i < 5; i++) {
        int size = dis(gen);
        vector<uint8_t> pdu_data(size, 0xBB + i);
        rlc_tm.receivePDU(pdu_data);
    }
    
    // Display statistics
    rlc_tm.displayStatistics();
    
    // Mode comparison
    cout << "\n[INFO] TM Mode Use Cases:" << endl;
    cout << "  • BCCH (Broadcast Control Channel)" << endl;
    cout << "  • PCCH (Paging Control Channel)" << endl;
    cout << "  • Very low latency requirements" << endl;
    cout << "  • No need for reliability (ARQ)" << endl;
    
    Logger::log("SUCCESS", "✓ RLC TM Mode test completed successfully!");
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================
int main() {
    cout << "\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║   5G NR RLC LAYER - TRANSPARENT MODE (TM) SIMULATOR   ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << endl;
    
    runTMTest();
    
    cout << "\nPress Enter to exit...";
    cin.get();
    
    return 0;
}
