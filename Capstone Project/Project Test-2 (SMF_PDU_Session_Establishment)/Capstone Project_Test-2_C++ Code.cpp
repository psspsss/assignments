/*
 * ════════════════════════════════════════════════════════════════════════════
 * 5G Core Network - SMF PDU Session Establishment Simulator (Enhanced)
 * ════════════════════════════════════════════════════════════════════════════
 * 
 * OVERVIEW:
 * This simulator demonstrates complete PDU Session lifecycle management in a
 * 5G Core Network (5GC), implementing the Session Management Function (SMF) 
 * with full protocol stack simulation.
 * 
 * KEY FEATURES:
 * ─────────────────────────────────────────────────────────────────────────────
 * • Multiple PDU Session types (IPv4/IPv6/Ethernet/Unstructured)
 * • S-NSSAI (Network Slicing) and DNN-based session routing
 * • QoS Flow management with 5QI (5G QoS Identifier) mapping
 * • PFCP (Packet Forwarding Control Protocol) simulation with UPF
 * • GTP-U tunnel establishment (F-TEID allocation)
 * • PCF (Policy Control Function) integration for PCC rule enforcement
 * • AMF/UDM coordination for session authorization
 * • Enhanced error handling and validation
 * • Comprehensive logging and statistics tracking
 * 
 * TECHNICAL STANDARDS:
 * ─────────────────────────────────────────────────────────────────────────────
 * • 3GPP TS 23.502 - Procedures for the 5G System
 * • 3GPP TS 29.502 - Session Management Services
 * • 3GPP TS 29.244 - Interface between Control Plane and User Plane Nodes (PFCP)
 * • 3GPP TS 23.501 - System architecture for the 5G System
 * 
 * AUTHOR: 5G Core Network Training Lab
 * VERSION: 2.1 (Enhanced Edition)
 * DATE: 2025
 * ════════════════════════════════════════════════════════════════════════════
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>
#include <algorithm>
#include <ctime>
#include <stdexcept>
#include <optional>

// ═══════════════════════════════════════════════════════════════════════════
// UTILITY CLASSES - Logging and System Tools
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @class Logger
 * @brief Provides structured logging with timestamps for network function operations
 */
class Logger {
private:
    std::string getTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

public:
    void info(const std::string& module, const std::string& message) const {
        std::cout << "[" << getTimestamp() << "] [INFO] [" << module << "] " 
                  << message << std::endl;
    }

    void success(const std::string& module, const std::string& message) const {
        std::cout << "[" << getTimestamp() << "] [✓ SUCCESS] [" << module << "] " 
                  << message << std::endl;
    }

    void error(const std::string& module, const std::string& message) const {
        std::cout << "[" << getTimestamp() << "] [✗ ERROR] [" << module << "] " 
                  << message << std::endl;
    }

    void warning(const std::string& module, const std::string& message) const {
        std::cout << "[" << getTimestamp() << "] [⚠ WARNING] [" << module << "] " 
                  << message << std::endl;
    }

    void protocol(const std::string& interface, const std::string& direction, 
                  const std::string& message) const {
        std::cout << "[" << getTimestamp() << "] [" << interface << "] " 
                  << direction << " " << message << std::endl;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// 5G CORE NETWORK DATA STRUCTURES
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @enum PDUSessionType
 * @brief Defines the types of PDU sessions as per 3GPP TS 23.501
 */
enum class PDUSessionType {
    IPv4,           ///< IPv4 only
    IPv6,           ///< IPv6 only
    IPv4v6,         ///< Dual stack IPv4 and IPv6
    Ethernet,       ///< Ethernet type
    Unstructured    ///< Unstructured type (for specific use cases)
};

/**
 * @enum SSCMode
 * @brief Session and Service Continuity Mode as per 3GPP TS 23.501
 */
enum class SSCMode {
    SSC_MODE_1,  ///< Maintain UPF anchor point
    SSC_MODE_2,  ///< Release and re-establish with new UPF
    SSC_MODE_3   ///< Make-before-break handover
};

/**
 * @struct SNSSAI
 * @brief Single Network Slice Selection Assistance Information
 * Identifies a network slice as per 3GPP TS 23.003
 */
struct SNSSAI {
    uint8_t sst;           ///< Slice/Service Type (1-255)
    uint32_t sd;           ///< Slice Differentiator (24-bit, optional)
    
    std::string toString() const {
        std::stringstream ss;
        ss << "SST=" << static_cast<int>(sst);
        if (sd != 0) {
            ss << ", SD=0x" << std::hex << std::setfill('0') << std::setw(6) << sd;
        }
        return ss.str();
    }
    
    bool operator==(const SNSSAI& other) const {
        return sst == other.sst && sd == other.sd;
    }
};

/**
 * @struct QoSFlow
 * @brief Quality of Service Flow identifier and parameters
 * As per 3GPP TS 23.501 Section 5.7.1
 */
struct QoSFlow {
    uint8_t qfi;           ///< QoS Flow Identifier (1-63)
    uint8_t five_qi;       ///< 5G QoS Identifier (5QI)
    uint32_t gfbr;         ///< Guaranteed Flow Bit Rate (kbps)
    uint32_t mfbr;         ///< Maximum Flow Bit Rate (kbps)
    uint8_t priority;      ///< Priority level (1-127, lower is higher priority)
    uint16_t packet_delay; ///< Packet Delay Budget (ms)
    double packet_error;   ///< Packet Error Rate (PER)
    bool gfbr_present;     ///< Indicates if GFBR is applicable (GBR flows)
    
    /**
     * @brief Determines if this is a Guaranteed Bit Rate (GBR) flow
     */
    std::string getResourceType() const {
        // GBR 5QIs: 1, 2, 3, 4, 65, 66, 67, 75, etc.
        return (five_qi <= 4 || five_qi == 65 || five_qi == 66 || 
                five_qi == 67 || five_qi == 75) ? "GBR" : "Non-GBR";
    }
    
    /**
     * @brief Returns the service characteristics based on 5QI
     * As per 3GPP TS 23.501 Table 5.7.4-1
     */
    std::string getQoSCharacteristics() const {
        std::map<uint8_t, std::string> qci_map = {
            {1, "Conversational Voice (VoNR, VoLTE)"},
            {2, "Conversational Video (Live Streaming)"},
            {3, "Real-time Gaming, V2X Messages"},
            {4, "Non-Conversational Video (Buffered Streaming)"},
            {5, "IMS Signaling"},
            {6, "Video (Buffered), TCP-based (HTTP, FTP)"},
            {7, "Voice, Video (Live), Interactive Gaming"},
            {8, "Video (Buffered), TCP-based Premium"},
            {9, "Video (Buffered), TCP-based Standard"},
            {65, "Mission Critical PTT Voice"},
            {66, "Mission Critical Video"},
            {69, "Mission Critical Delay Sensitive Signaling"},
            {70, "Mission Critical Data"},
            {79, "V2X Messages"},
            {80, "Low Latency eMBB Applications"},
            {82, "Discrete Automation (Small Packets)"},
            {83, "Discrete Automation (Large Packets)"},
            {84, "Intelligent Transport Systems"},
            {85, "Electricity Distribution (High Voltage)"}
        };
        
        auto it = qci_map.find(five_qi);
        if (it != qci_map.end()) {
            return it->second;
        }
        return "Standard Data Transfer";
    }
    
    /**
     * @brief Validates QoS Flow parameters
     */
    bool isValid() const {
        return qfi >= 1 && qfi <= 63 && 
               five_qi >= 1 && five_qi <= 255 &&
               priority >= 1 && priority <= 127;
    }
};

/**
 * @struct FTEID
 * @brief Fully Qualified Tunnel Endpoint Identifier
 * Used in GTP-U tunneling (3GPP TS 29.281)
 */
struct FTEID {
    uint32_t teid;         ///< Tunnel Endpoint Identifier
    std::string ipv4;      ///< IPv4 address of tunnel endpoint
    std::string ipv6;      ///< IPv6 address of tunnel endpoint (optional)
    
    std::string toString() const {
        std::stringstream ss;
        ss << "TEID=0x" << std::hex << std::setfill('0') << std::setw(8) << teid 
           << std::dec << ", IPv4=" << ipv4;
        if (!ipv6.empty()) {
            ss << ", IPv6=" << ipv6;
        }
        return ss.str();
    }
};

/**
 * @struct PCCRule
 * @brief Policy and Charging Control Rule
 * Provided by PCF as per 3GPP TS 29.512
 */
struct PCCRule {
    std::string rule_id;           ///< Unique rule identifier
    uint8_t precedence;            ///< Rule precedence (lower value = higher priority)
    std::string flow_description;  ///< Packet filter information
    uint8_t qfi;                   ///< Associated QoS Flow Identifier
    bool active;                   ///< Rule activation status
    
    std::string toString() const {
        std::stringstream ss;
        ss << "Rule[" << rule_id << "] Precedence=" << static_cast<int>(precedence)
           << ", QFI=" << static_cast<int>(qfi) 
           << ", Flow=" << flow_description
           << ", Status=" << (active ? "ACTIVE" : "INACTIVE");
        return ss.str();
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// PDU SESSION CLASS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @class PDUSession
 * @brief Represents a PDU (Protocol Data Unit) Session
 * 
 * Manages the complete lifecycle of a PDU session including IP allocation,
 * QoS flows, PCC rules, and traffic statistics.
 */
class PDUSession {
private:
    // Session identifiers
    uint16_t pdu_session_id;
    PDUSessionType session_type;
    std::string dnn;                ///< Data Network Name
    SNSSAI snssai;                  ///< Network Slice identifier
    SSCMode ssc_mode;
    
    // UE addressing
    std::string ue_ipv4;
    std::string ue_ipv6;
    
    // Tunnel endpoints
    FTEID upf_fteid_n3;      ///< N3 interface (gNB to UPF)
    FTEID upf_fteid_n9;      ///< N9 interface (UPF to UPF for roaming)
    
    // QoS and Policy
    std::vector<QoSFlow> qos_flows;
    std::vector<PCCRule> pcc_rules;
    
    // Session state
    std::string state;
    uint64_t uplink_bytes;
    uint64_t downlink_bytes;
    std::chrono::system_clock::time_point establishment_time;

    // Random number generation
    std::random_device rd;
    std::mt19937 gen;

    /**
     * @brief Generates a random IPv4 address within a subnet
     */
    std::string generateIPv4(const std::string& subnet) {
        std::uniform_int_distribution<> dist(10, 250);
        std::stringstream ss;
        ss << subnet << "." << dist(gen);
        return ss.str();
    }

    /**
     * @brief Generates a random IPv6 address with a prefix
     */
    std::string generateIPv6(const std::string& prefix) {
        std::uniform_int_distribution<> dist(0, 65535);
        std::stringstream ss;
        ss << prefix << ":"
           << std::hex << std::setfill('0') << std::setw(4) << dist(gen) << ":"
           << std::setw(4) << dist(gen) << ":"
           << std::setw(4) << dist(gen) << ":"
           << std::setw(4) << dist(gen);
        return ss.str();
    }

    /**
     * @brief Generates a random Tunnel Endpoint Identifier (TEID)
     */
    uint32_t generateTEID() {
        std::uniform_int_distribution<uint32_t> dist(0x10000000, 0xFFFFFFFF);
        return dist(gen);
    }

public:
    /**
     * @brief Constructs a new PDU Session
     */
    PDUSession(uint16_t id, PDUSessionType type, const std::string& dnn_name, 
               const SNSSAI& slice, SSCMode mode) 
        : pdu_session_id(id), session_type(type), dnn(dnn_name), 
          snssai(slice), ssc_mode(mode), state("INACTIVE"),
          uplink_bytes(0), downlink_bytes(0), gen(rd()) {
        establishment_time = std::chrono::system_clock::now();
    }

    /**
     * @brief Allocates IP address(es) to the UE based on session type
     */
    void allocateIPAddress() {
        if (session_type == PDUSessionType::IPv4 || session_type == PDUSessionType::IPv4v6) {
            ue_ipv4 = generateIPv4("10.45.0");
        }
        if (session_type == PDUSessionType::IPv6 || session_type == PDUSessionType::IPv4v6) {
            ue_ipv6 = generateIPv6("2001:db8:5g:ue");
        }
    }

    /**
     * @brief Allocates F-TEID for UPF interfaces
     */
    void allocateUPFFTEID() {
        upf_fteid_n3.teid = generateTEID();
        upf_fteid_n3.ipv4 = generateIPv4("192.168.100");
        
        upf_fteid_n9.teid = generateTEID();
        upf_fteid_n9.ipv4 = generateIPv4("192.168.101");
    }

    /**
     * @brief Adds a QoS Flow to the session
     */
    void addQoSFlow(const QoSFlow& flow) {
        if (!flow.isValid()) {
            throw std::invalid_argument("Invalid QoS Flow parameters");
        }
        
        // Check for duplicate QFI
        for (const auto& existing : qos_flows) {
            if (existing.qfi == flow.qfi) {
                throw std::invalid_argument("QFI " + std::to_string(flow.qfi) + " already exists");
            }
        }
        
        qos_flows.push_back(flow);
    }

    /**
     * @brief Adds a PCC Rule to the session
     */
    void addPCCRule(const PCCRule& rule) {
        pcc_rules.push_back(rule);
    }

    /**
     * @brief Updates session state
     */
    void setState(const std::string& new_state) {
        state = new_state;
    }

    /**
     * @brief Updates traffic statistics
     */
    void updateTraffic(uint64_t ul, uint64_t dl) {
        uplink_bytes += ul;
        downlink_bytes += dl;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // GETTERS
    // ═══════════════════════════════════════════════════════════════════════
    
    uint16_t getSessionId() const { return pdu_session_id; }
    std::string getDNN() const { return dnn; }
    SNSSAI getSNSSAI() const { return snssai; }
    std::string getState() const { return state; }
    std::string getUEIPv4() const { return ue_ipv4; }
    std::string getUEIPv6() const { return ue_ipv6; }
    FTEID getUPFFTEIDN3() const { return upf_fteid_n3; }
    FTEID getUPFFTEIDN9() const { return upf_fteid_n9; }
    const std::vector<QoSFlow>& getQoSFlows() const { return qos_flows; }
    const std::vector<PCCRule>& getPCCRules() const { return pcc_rules; }
    uint64_t getUplinkBytes() const { return uplink_bytes; }
    uint64_t getDownlinkBytes() const { return downlink_bytes; }

    std::string getSessionTypeString() const {
        switch (session_type) {
            case PDUSessionType::IPv4: return "IPv4";
            case PDUSessionType::IPv6: return "IPv6";
            case PDUSessionType::IPv4v6: return "IPv4v6 (Dual Stack)";
            case PDUSessionType::Ethernet: return "Ethernet";
            case PDUSessionType::Unstructured: return "Unstructured";
            default: return "Unknown";
        }
    }

    std::string getSSCModeString() const {
        switch (ssc_mode) {
            case SSCMode::SSC_MODE_1: return "SSC Mode 1 (UPF Anchor Maintained)";
            case SSCMode::SSC_MODE_2: return "SSC Mode 2 (Release & Re-establish)";
            case SSCMode::SSC_MODE_3: return "SSC Mode 3 (Make-Before-Break)";
            default: return "Unknown";
        }
    }

    /**
     * @brief Calculates session duration
     */
    std::string getSessionDuration() const {
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - establishment_time);
        
        auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
        duration -= hours;
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
        duration -= minutes;
        auto seconds = duration;
        
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << hours.count() << ":"
           << std::setw(2) << minutes.count() << ":"
           << std::setw(2) << seconds.count();
        return ss.str();
    }

    /**
     * @brief Displays comprehensive session information
     */
    void displaySessionInfo() const {
        std::cout << "\n╔══════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║            PDU SESSION DETAILED INFORMATION                          ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ BASIC INFORMATION                                                    ║\n";
        std::cout << "╟──────────────────────────────────────────────────────────────────────╢\n";
        std::cout << "║ PDU Session ID       : " << std::setw(43) << std::left << pdu_session_id << "║\n";
        std::cout << "║ State                : " << std::setw(43) << state << "║\n";
        std::cout << "║ Session Type         : " << std::setw(43) << getSessionTypeString() << "║\n";
        std::cout << "║ DNN                  : " << std::setw(43) << dnn << "║\n";
        std::cout << "║ S-NSSAI              : " << std::setw(43) << snssai.toString() << "║\n";
        std::cout << "║ SSC Mode             : " << std::setw(43) << getSSCModeString() << "║\n";
        std::cout << "║ Session Duration     : " << std::setw(43) << getSessionDuration() << "║\n";
        
        std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ UE IP ADDRESSING                                                     ║\n";
        std::cout << "╟──────────────────────────────────────────────────────────────────────╢\n";
        if (!ue_ipv4.empty()) {
            std::cout << "║ UE IPv4 Address      : " << std::setw(43) << ue_ipv4 << "║\n";
        }
        if (!ue_ipv6.empty()) {
            std::cout << "║ UE IPv6 Address      : " << std::setw(43) << ue_ipv6 << "║\n";
        }
        
        std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ TUNNEL ENDPOINTS (GTP-U)                                             ║\n";
        std::cout << "╟──────────────────────────────────────────────────────────────────────╢\n";
        std::cout << "║ UPF N3 F-TEID        : " << std::setw(43) << upf_fteid_n3.toString() << "║\n";
        std::cout << "║ UPF N9 F-TEID        : " << std::setw(43) << upf_fteid_n9.toString() << "║\n";
        
        std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ QoS FLOWS (" << std::setw(2) << qos_flows.size() << ")                                                      ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
        
        for (size_t i = 0; i < qos_flows.size(); ++i) {
            const auto& flow = qos_flows[i];
            std::cout << "║ Flow #" << (i+1) << "                                                             ║\n";
            std::cout << "║   QFI: " << std::setw(2) << static_cast<int>(flow.qfi) 
                      << " | 5QI: " << std::setw(3) << static_cast<int>(flow.five_qi)
                      << " | Type: " << std::setw(8) << flow.getResourceType()
                      << " | Priority: " << std::setw(3) << static_cast<int>(flow.priority) << std::setw(15) << " " << "║\n";
            
            std::string chars = flow.getQoSCharacteristics();
            if (chars.length() > 60) {
                chars = chars.substr(0, 57) + "...";
            }
            std::cout << "║   Service: " << std::setw(56) << std::left << chars << "║\n";
            
            if (flow.gfbr_present) {
                std::cout << "║   GFBR: " << std::setw(7) << flow.gfbr << " kbps | MFBR: " 
                          << std::setw(7) << flow.mfbr << " kbps" << std::setw(22) << " " << "║\n";
            } else {
                std::cout << "║   Max Flow BR: " << std::setw(7) << flow.mfbr << " kbps" << std::setw(35) << " " << "║\n";
            }
            
            std::cout << "║   Packet Delay Budget: " << std::setw(4) << flow.packet_delay 
                      << " ms | PER: " << std::scientific << std::setprecision(1) 
                      << flow.packet_error << std::setw(20) << " " << "║\n" << std::fixed;
            
            if (i < qos_flows.size() - 1) {
                std::cout << "╟──────────────────────────────────────────────────────────────────────╢\n";
            }
        }
        
        std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ PCC RULES (" << std::setw(2) << pcc_rules.size() << ")                                                     ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
        
        if (pcc_rules.empty()) {
            std::cout << "║ No PCC rules configured                                              ║\n";
        } else {
            for (const auto& rule : pcc_rules) {
                std::string rule_str = rule.toString();
                if (rule_str.length() > 67) {
                    rule_str = rule_str.substr(0, 64) + "...";
                }
                std::cout << "║ " << std::setw(68) << std::left << rule_str << "║\n";
            }
        }
        
        std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ TRAFFIC STATISTICS                                                   ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
        
        // Format bytes with appropriate units
        auto formatBytes = [](uint64_t bytes) -> std::string {
            std::stringstream ss;
            if (bytes >= 1073741824) {  // GB
                ss << std::fixed << std::setprecision(2) << (bytes / 1073741824.0) << " GB";
            } else if (bytes >= 1048576) {  // MB
                ss << std::fixed << std::setprecision(2) << (bytes / 1048576.0) << " MB";
            } else if (bytes >= 1024) {  // KB
                ss << std::fixed << std::setprecision(2) << (bytes / 1024.0) << " KB";
            } else {
                ss << bytes << " bytes";
            }
            return ss.str();
        };
        
        std::cout << "║ Uplink              : " << std::setw(43) << formatBytes(uplink_bytes) << "║\n";
        std::cout << "║ Downlink            : " << std::setw(43) << formatBytes(downlink_bytes) << "║\n";
        std::cout << "║ Total               : " << std::setw(43) << formatBytes(uplink_bytes + downlink_bytes) << "║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n";
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// NETWORK FUNCTION SIMULATORS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @class UDMSimulator
 * @brief Unified Data Management function simulator
 * 
 * Handles subscription data retrieval and authentication as per 3GPP TS 29.503
 */
class UDMSimulator {
private:
    Logger& logger;

public:
    UDMSimulator(Logger& log) : logger(log) {}

    /**
     * @brief Retrieves subscription data for a given SUPI
     */
    bool getSubscriptionData(const std::string& supi, const SNSSAI& snssai, const std::string& dnn) {
        logger.protocol("N8 (SMF->UDM)", "→", "GET /subscription-data/" + supi + "/sm-data");
        logger.info("UDM", "Checking subscription authorization for SUPI=" + supi);
        logger.info("UDM", "Verifying S-NSSAI: " + snssai.toString() + ", DNN: " + dnn);
        
        // Simulate subscription database lookup
        logger.info("UDM", "Querying subscription database (UDSF)...");
        logger.info("UDM", "Validating DNN access permissions...");
        logger.info("UDM", "Validating Network Slice access permissions...");
        
        // Simulate successful authorization
        logger.success("UDM", "Subscription data retrieved - Authorization GRANTED");
        logger.info("UDM", "SMF allowed to proceed with PDU Session establishment");
        logger.protocol("N8 (UDM->SMF)", "←", "200 OK - Subscription Data");
        return true;
    }
    
    /**
     * @brief Updates session management subscription data
     */
    void updateSMSubscription(const std::string& supi, uint16_t pdu_session_id) {
        logger.info("UDM", "Updating SM subscription for SUPI=" + supi);
        logger.info("UDM", "Recording active PDU Session ID: " + std::to_string(pdu_session_id));
    }
};

/**
 * @class PCFSimulator
 * @brief Policy Control Function simulator
 * 
 * Provides PCC rules for policy enforcement as per 3GPP TS 29.512
 */
class PCFSimulator {
private:
    Logger& logger;

public:
    PCFSimulator(Logger& log) : logger(log) {}

    /**
     * @brief Generates PCC rules based on DNN and network slice
     */
    std::vector<PCCRule> getPCCRules(const std::string& dnn, const SNSSAI& snssai) {
        logger.protocol("N7 (SMF->PCF)", "→", "POST /sm-policies - Request PCC Rules");
        logger.info("PCF", "Generating PCC rules for DNN=" + dnn + ", S-NSSAI=" + snssai.toString());
        
        std::vector<PCCRule> rules;
        
        // Generate rules based on DNN
        if (dnn == "internet" || dnn == "ims") {
            // Default rule for general traffic
            PCCRule default_rule;
            default_rule.rule_id = "PCC-RULE-DEFAULT";
            default_rule.precedence = 100;
            default_rule.flow_description = "permit out ip from any to assigned";
            default_rule.qfi = 9;  // Default QFI
            default_rule.active = true;
            rules.push_back(default_rule);
            
            // IMS-specific rules
            if (dnn == "ims") {
                // SIP signaling rule
                PCCRule sip_rule;
                sip_rule.rule_id = "PCC-RULE-SIP";
                sip_rule.precedence = 10;
                sip_rule.flow_description = "permit in/out udp from any 5060 to assigned";
                sip_rule.qfi = 5;  // IMS Signaling
                sip_rule.active = true;
                rules.push_back(sip_rule);
                
                // RTP media rule
                PCCRule rtp_rule;
                rtp_rule.rule_id = "PCC-RULE-RTP";
                rtp_rule.precedence = 20;
                rtp_rule.flow_description = "permit in/out udp from any 16384-32767 to assigned";
                rtp_rule.qfi = 1;  // Conversational Voice
                rtp_rule.active = true;
                rules.push_back(rtp_rule);
                
                // RTCP control rule
                PCCRule rtcp_rule;
                rtcp_rule.rule_id = "PCC-RULE-RTCP";
                rtcp_rule.precedence = 30;
                rtcp_rule.flow_description = "permit in/out udp from any 16385-32768 to assigned";
                rtcp_rule.qfi = 1;
                rtcp_rule.active = true;
                rules.push_back(rtcp_rule);
            }
        }
        
        logger.success("PCF", "Generated " + std::to_string(rules.size()) + " PCC rules");
        logger.protocol("N7 (PCF->SMF)", "←", "201 Created - PCC Rules Delivered");
        
        for (const auto& rule : rules) {
            logger.info("PCF", "  ↳ " + rule.toString());
        }
        
        return rules;
    }

    /**
     * @brief Notifies about policy updates
     */
    void updatePCCRules(uint16_t pdu_session_id) {
        logger.protocol("N7 (PCF->SMF)", "←", "NOTIFY - Policy Update for PDU Session " + 
                       std::to_string(pdu_session_id));
        logger.info("PCF", "Policy update notification received");
        logger.info("PCF", "SMF should re-evaluate QoS and charging policies");
    }
};

/**
 * @class UPFSimulator
 * @brief User Plane Function simulator
 * 
 * Manages PFCP sessions and packet forwarding as per 3GPP TS 29.244
 */
class UPFSimulator {
private:
    Logger& logger;
    std::map<uint16_t, uint32_t> pfcp_sessions;  ///< PDU Session ID -> PFCP Session ID mapping
    std::map<uint16_t, std::vector<uint8_t>> session_qfis;  ///< Track QFIs per session

public:
    UPFSimulator(Logger& log) : logger(log) {}

    /**
     * @brief Establishes a PFCP session with the UPF
     */
    bool establishPFCPSession(uint16_t pdu_session_id, const FTEID& fteid) {
        logger.protocol("N4 (SMF->UPF)", "→", "PFCP Session Establishment Request");
        logger.info("PFCP", "Creating PFCP session for PDU Session ID: " + std::to_string(pdu_session_id));
        logger.info("PFCP", "UPF F-TEID (N3): " + fteid.toString());
        
        // Generate PFCP Session ID (SEID)
        uint32_t pfcp_session_id = 0x80000000 | pdu_session_id;
        pfcp_sessions[pdu_session_id] = pfcp_session_id;
        
        logger.info("UPF", "Allocating GTP-U tunnel resources for N3 interface");
        logger.info("UPF", "Programming Packet Detection Rules (PDRs) for downlink/uplink");
        logger.info("UPF", "Programming Forwarding Action Rules (FARs)");
        logger.info("UPF", "  • FAR 1: Uplink forwarding to Data Network");
        logger.info("UPF", "  • FAR 2: Downlink forwarding to gNB via N3");
        logger.info("UPF", "Programming QoS Enforcement Rules (QERs)");
        logger.info("UPF", "Programming Usage Reporting Rules (URRs) for charging");
        
        logger.success("UPF", "PFCP Session established - SEID: 0x" + 
                      std::to_string(pfcp_session_id));
        logger.protocol("N4 (UPF->SMF)", "←", "PFCP Session Establishment Response - SUCCESS");
        return true;
    }

    /**
     * @brief Modifies an existing PFCP session (e.g., adding QoS flows)
     */
    bool modifyPFCPSession(uint16_t pdu_session_id, const QoSFlow& flow) {
        auto it = pfcp_sessions.find(pdu_session_id);
        if (it == pfcp_sessions.end()) {
            logger.error("UPF", "PFCP session not found for PDU Session ID: " + 
                        std::to_string(pdu_session_id));
            return false;
        }
        
        logger.protocol("N4 (SMF->UPF)", "→", "PFCP Session Modification Request");
        logger.info("PFCP", "Modifying PFCP session for PDU Session ID: " + std::to_string(pdu_session_id));
        logger.info("PFCP", "Adding QoS Flow - QFI: " + std::to_string(flow.qfi) + 
                   ", 5QI: " + std::to_string(flow.five_qi));
        
        logger.info("UPF", "Creating new PDR for QFI " + std::to_string(flow.qfi));
        logger.info("UPF", "  • PDR ID: " + std::to_string(pdu_session_id * 100 + flow.qfi));
        logger.info("UPF", "  • QFI marking: " + std::to_string(flow.qfi));
        logger.info("UPF", "Updating QoS Enforcement Rules for new flow");
        logger.info("UPF", "Configuring traffic shaping:");
        logger.info("UPF", "  • Maximum Flow Bit Rate: " + std::to_string(flow.mfbr) + " kbps");
        
        if (flow.gfbr_present) {
            logger.info("UPF", "  • Guaranteed Flow Bit Rate: " + std::to_string(flow.gfbr) + " kbps");
            logger.info("UPF", "Reserving dedicated bandwidth for GBR flow");
        }
        
        logger.info("UPF", "Configuring QoS parameters:");
        logger.info("UPF", "  • Priority Level: " + std::to_string(flow.priority));
        logger.info("UPF", "  • Packet Delay Budget: " + std::to_string(flow.packet_delay) + " ms");
        
        // Track QFI
        session_qfis[pdu_session_id].push_back(flow.qfi);
        
        logger.success("UPF", "PFCP Session modified successfully");
        logger.success("UPF", "New QoS flow is now active and enforced");
        logger.protocol("N4 (UPF->SMF)", "←", "PFCP Session Modification Response - SUCCESS");
        return true;
    }

    /**
     * @brief Releases a PFCP session
     */
    void releasePFCPSession(uint16_t pdu_session_id) {
        auto it = pfcp_sessions.find(pdu_session_id);
        if (it == pfcp_sessions.end()) {
            logger.warning("UPF", "PFCP session not found for PDU Session ID: " + 
                          std::to_string(pdu_session_id));
            return;
        }
        
        logger.protocol("N4 (SMF->UPF)", "→", "PFCP Session Deletion Request");
        logger.info("PFCP", "Releasing PFCP session for PDU Session ID: " + std::to_string(pdu_session_id));
        logger.info("PFCP", "SEID: 0x" + std::to_string(it->second));
        
        logger.info("UPF", "Releasing GTP-U tunnel resources");
        logger.info("UPF", "Removing all Packet Detection Rules (PDRs)");
        logger.info("UPF", "Removing all Forwarding Action Rules (FARs)");
        logger.info("UPF", "Removing all QoS Enforcement Rules (QERs)");
        logger.info("UPF", "Removing all Usage Reporting Rules (URRs)");
        logger.info("UPF", "Flushing buffered packets for this session");
        
        pfcp_sessions.erase(it);
        session_qfis.erase(pdu_session_id);
        
        logger.success("UPF", "PFCP Session deleted successfully");
        logger.success("UPF", "All resources released");
        logger.protocol("N4 (UPF->SMF)", "←", "PFCP Session Deletion Response - SUCCESS");
    }
    
    /**
     * @brief Gets statistics for a PFCP session
     */
    void getSessionStats(uint16_t pdu_session_id) {
        auto it = pfcp_sessions.find(pdu_session_id);
        if (it == pfcp_sessions.end()) {
            logger.error("UPF", "PFCP session not found");
            return;
        }
        
        logger.info("UPF", "Session statistics for PDU Session " + std::to_string(pdu_session_id));
        logger.info("UPF", "  • Active QFIs: " + std::to_string(session_qfis[pdu_session_id].size()));
        logger.info("UPF", "  • PFCP SEID: 0x" + std::to_string(it->second));
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// SMF SIMULATOR - MAIN SESSION MANAGER
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @class SMFSimulator
 * @brief Session Management Function simulator
 * 
 * Main class orchestrating PDU session lifecycle management
 */
class SMFSimulator {
private:
    Logger logger;
    UDMSimulator udm;
    PCFSimulator pcf;
    UPFSimulator upf;
    std::map<uint16_t, std::unique_ptr<PDUSession>> sessions;
    uint16_t next_session_id;

    /**
     * @brief Creates a default Non-GBR QoS flow (QoS Flow ID 9, 5QI 9)
     */
    QoSFlow createDefaultQoSFlow() const {
        QoSFlow flow;
        flow.qfi = 9;
        flow.five_qi = 9;  // Default bearer
        flow.gfbr = 0;
        flow.mfbr = 100000;  // 100 Mbps max
        flow.priority = 80;
        flow.packet_delay = 300;  // 300 ms
        flow.packet_error = 1e-6;
        flow.gfbr_present = false;
        return flow;
    }

    /**
     * @brief Creates a Voice QoS flow (GBR, 5QI 1)
     */
    QoSFlow createVoiceQoSFlow() const {
        QoSFlow flow;
        flow.qfi = 1;
        flow.five_qi = 1;  // Conversational Voice
        flow.gfbr = 128;      // 128 kbps guaranteed
        flow.mfbr = 256;      // 256 kbps max
        flow.priority = 20;   // High priority
        flow.packet_delay = 100;  // 100 ms delay budget
        flow.packet_error = 1e-2;
        flow.gfbr_present = true;
        return flow;
    }

    /**
     * @brief Creates a Video QoS flow (GBR, 5QI 2)
     */
    QoSFlow createVideoQoSFlow() const {
        QoSFlow flow;
        flow.qfi = 2;
        flow.five_qi = 2;  // Conversational Video
        flow.gfbr = 4000;     // 4 Mbps guaranteed
        flow.mfbr = 10000;    // 10 Mbps max
        flow.priority = 40;
        flow.packet_delay = 150;  // 150 ms delay budget
        flow.packet_error = 1e-3;
        flow.gfbr_present = true;
        return flow;
    }
    
    /**
     * @brief Creates a Gaming QoS flow (GBR, 5QI 3)
     */
    QoSFlow createGamingQoSFlow() const {
        QoSFlow flow;
        flow.qfi = 3;
        flow.five_qi = 3;  // Real-time Gaming
        flow.gfbr = 5000;     // 5 Mbps guaranteed
        flow.mfbr = 15000;    // 15 Mbps max
        flow.priority = 30;
        flow.packet_delay = 50;   // 50 ms delay budget (low latency)
        flow.packet_error = 1e-3;
        flow.gfbr_present = true;
        return flow;
    }

public:
    SMFSimulator() : udm(logger), pcf(logger), upf(logger), next_session_id(1) {
        logger.info("SMF", "═══════════════════════════════════════════════════════════");
        logger.info("SMF", "  5G Core SMF Simulator initialized");
        logger.info("SMF", "  Version: 2.1 Enhanced Edition");
        logger.info("SMF", "  Standards: 3GPP TS 23.502, TS 29.502, TS 29.244");
        logger.info("SMF", "═══════════════════════════════════════════════════════════");
        logger.info("SMF", "Ready to handle PDU Session establishment requests");
    }

    /**
     * @brief Establishes a new PDU Session
     * 
     * Implements the complete PDU Session Establishment procedure as per
     * 3GPP TS 23.502 Section 4.3.2
     */
    bool establishPDUSession(PDUSessionType type, const std::string& dnn, 
                            const SNSSAI& snssai, SSCMode ssc_mode) {
        std::cout << "\n" << std::string(80, '=') << "\n";
        logger.info("SMF", "╔════════════════════════════════════════════════════════════╗");
        logger.info("SMF", "║       INITIATING PDU SESSION ESTABLISHMENT PROCEDURE       ║");
        logger.info("SMF", "╚════════════════════════════════════════════════════════════╝");
        
        uint16_t pdu_session_id = next_session_id++;
        std::string supi = "imsi-001010000000001";
        
        logger.info("SMF", "Request Parameters:");
        logger.info("SMF", "  • PDU Session ID: " + std::to_string(pdu_session_id));
        logger.info("SMF", "  • SUPI: " + supi);
        logger.info("SMF", "  • DNN: " + dnn);
        logger.info("SMF", "  • S-NSSAI: " + snssai.toString());
        
        try {
            // ═══════════════════════════════════════════════════════════════
            // STEP 1: UDM Subscription Authorization
            // ═══════════════════════════════════════════════════════════════
            logger.info("SMF", "\n[STEP 1/7] UDM Subscription Authorization");
            logger.info("SMF", "───────────────────────────────────────────");
            if (!udm.getSubscriptionData(supi, snssai, dnn)) {
                logger.error("SMF", "Subscription authorization failed");
                logger.error("SMF", "PDU Session establishment ABORTED");
                return false;
            }
            
            // ═══════════════════════════════════════════════════════════════
            // STEP 2: Create PDU Session Context
            // ═══════════════════════════════════════════════════════════════
            logger.info("SMF", "\n[STEP 2/7] Creating PDU Session Context");
            logger.info("SMF", "───────────────────────────────────────────");
            auto session = std::make_unique<PDUSession>(pdu_session_id, type, dnn, snssai, ssc_mode);
            logger.success("SMF", "PDU Session context created in memory");
            logger.info("SMF", "Session will use " + session->getSSCModeString());
            
            // ═══════════════════════════════════════════════════════════════
            // STEP 3: UPF Selection
            // ═══════════════════════════════════════════════════════════════
            logger.info("SMF", "\n[STEP 3/7] UPF Selection (N4 Interface)");
            logger.info("SMF", "───────────────────────────────────────────");
            logger.info("SMF", "Evaluating UPF candidates based on:");
            logger.info("SMF", "  • Network slice requirements (S-NSSAI)");
            logger.info("SMF", "  • DNN routing policies");
            logger.info("SMF", "  • Geographic proximity to UE");
            logger.info("SMF", "  • Current load and available capacity");
            logger.info("SMF", "  • Required service quality");
            logger.success("SMF", "Selected UPF: UPF-REGION-01");
            logger.info("SMF", "  • UPF Address: 192.168.100.1");
            logger.info("SMF", "  • Location: Core DC Region 1");
            logger.info("SMF", "  • Load: 45% (within capacity)");
            
            // ═══════════════════════════════════════════════════════════════
            // STEP 4: IP Address Allocation
            // ═══════════════════════════════════════════════════════════════
            logger.info("SMF", "\n[STEP 4/7] IP Address Allocation");
            logger.info("SMF", "───────────────────────────────────────────");
            logger.info("SMF", "Allocating IP address from pool for DNN: " + dnn);
            session->allocateIPAddress();
            session->allocateUPFFTEID();
            
            if (!session->getUEIPv4().empty()) {
                logger.success("SMF", "UE IPv4 allocated: " + session->getUEIPv4());
            }
            if (!session->getUEIPv6().empty()) {
                logger.success("SMF", "UE IPv6 allocated: " + session->getUEIPv6());
            }
            
            logger.info("SMF", "F-TEID allocation for GTP-U tunnels:");
            logger.info("SMF", "  • N3 Interface (gNB↔UPF): " + session->getUPFFTEIDN3().toString());
            logger.info("SMF", "  • N9 Interface (UPF↔UPF): " + session->getUPFFTEIDN9().toString());
            
            // ═══════════════════════════════════════════════════════════════
            // STEP 5: PCF Interaction (Policy Control)
            // ═══════════════════════════════════════════════════════════════
            logger.info("SMF", "\n[STEP 5/7] Policy Control Function (PCF) Interaction");
            logger.info("SMF", "───────────────────────────────────────────");
            logger.info("SMF", "Requesting PCC rules from PCF via N7 interface");
            auto pcc_rules = pcf.getPCCRules(dnn, snssai);
            
            for (const auto& rule : pcc_rules) {
                session->addPCCRule(rule);
            }
            logger.success("SMF", "PCC rules installed: " + std::to_string(pcc_rules.size()) + " rule(s)");
            
            // ═══════════════════════════════════════════════════════════════
            // STEP 6: Default QoS Flow Establishment
            // ═══════════════════════════════════════════════════════════════
            logger.info("SMF", "\n[STEP 6/7] QoS Flow Establishment");
            logger.info("SMF", "───────────────────────────────────────────");
            logger.info("SMF", "Creating default QoS flow for session");
            QoSFlow default_flow = createDefaultQoSFlow();
            session->addQoSFlow(default_flow);
            logger.success("SMF", "Default QoS Flow created:");
            logger.info("SMF", "  • QFI: " + std::to_string(default_flow.qfi));
            logger.info("SMF", "  • 5QI: " + std::to_string(default_flow.five_qi));
            logger.info("SMF", "  • Type: " + default_flow.getResourceType());
            logger.info("SMF", "  • Service: " + default_flow.getQoSCharacteristics());
            
            // ═══════════════════════════════════════════════════════════════
            // STEP 7: PFCP Session with UPF
            // ═══════════════════════════════════════════════════════════════
            logger.info("SMF", "\n[STEP 7/7] PFCP Session Establishment (N4 Interface)");
            logger.info("SMF", "───────────────────────────────────────────");
            logger.info("SMF", "Establishing PFCP session with selected UPF");
            if (!upf.establishPFCPSession(pdu_session_id, session->getUPFFTEIDN3())) {
                logger.error("SMF", "PFCP session establishment failed");
                logger.error("SMF", "PDU Session establishment ABORTED");
                return false;
            }
            
            // ═══════════════════════════════════════════════════════════════
            // Session Activation
            // ═══════════════════════════════════════════════════════════════
            session->setState("ACTIVE");
            udm.updateSMSubscription(supi, pdu_session_id);
            
            // Success notification
            logger.info("SMF", "\n");
            logger.success("SMF", "╔════════════════════════════════════════════════════════════╗");
            logger.success("SMF", "║   PDU SESSION ESTABLISHMENT COMPLETED SUCCESSFULLY!        ║");
            logger.success("SMF", "╚════════════════════════════════════════════════════════════╝");
            logger.info("SMF", "");
            logger.info("SMF", "Session Status:");
            logger.info("SMF", "  • PDU Session ID: " + std::to_string(pdu_session_id));
            logger.info("SMF", "  • State: ACTIVE");
            logger.info("SMF", "  • GTP-U Tunnel: ESTABLISHED");
            logger.info("SMF", "  • User Plane: READY FOR DATA TRANSFER");
            logger.info("SMF", "  • QoS Flows: " + std::to_string(session->getQoSFlows().size()));
            logger.info("SMF", "  • PCC Rules: " + std::to_string(session->getPCCRules().size()));
            std::cout << std::string(80, '=') << "\n";
            
            // Store session
            sessions[pdu_session_id] = std::move(session);
            
            return true;
            
        } catch (const std::exception& e) {
            logger.error("SMF", "Exception during PDU Session establishment: " + std::string(e.what()));
            logger.error("SMF", "PDU Session establishment FAILED");
            return false;
        }
    }

    /**
     * @brief Adds a new QoS flow to an existing PDU session
     */
    bool addQoSFlow(uint16_t pdu_session_id, const QoSFlow& flow) {
        auto it = sessions.find(pdu_session_id);
        if (it == sessions.end()) {
            logger.error("SMF", "PDU Session not found: " + std::to_string(pdu_session_id));
            return false;
        }

        std::cout << "\n" << std::string(80, '=') << "\n";
        logger.info("SMF", "╔════════════════════════════════════════════════════════════╗");
        logger.info("SMF", "║           QoS FLOW ADDITION PROCEDURE                      ║");
        logger.info("SMF", "╚════════════════════════════════════════════════════════════╝");
        
        logger.info("SMF", "Target PDU Session ID: " + std::to_string(pdu_session_id));
        logger.info("SMF", "New QoS Flow Parameters:");
        logger.info("SMF", "  • QFI: " + std::to_string(flow.qfi));
        logger.info("SMF", "  • 5QI: " + std::to_string(flow.five_qi));
        logger.info("SMF", "  • Type: " + flow.getResourceType());
        logger.info("SMF", "  • Service: " + flow.getQoSCharacteristics());
        logger.info("SMF", "  • Priority: " + std::to_string(flow.priority));
        
        try {
            it->second->addQoSFlow(flow);
            logger.success("SMF", "QoS flow added to session context");
            
            // Update UPF via PFCP
            logger.info("SMF", "Updating UPF with new QoS flow via N4 interface");
            if (!upf.modifyPFCPSession(pdu_session_id, flow)) {
                logger.error("SMF", "Failed to update UPF");
                return false;
            }
            
            logger.success("SMF", "╔════════════════════════════════════════════════════════════╗");
            logger.success("SMF", "║   QoS FLOW ADDED SUCCESSFULLY!                             ║");
            logger.success("SMF", "╚════════════════════════════════════════════════════════════╝");
            logger.info("SMF", "New QoS flow is active and ready for traffic");
            std::cout << std::string(80, '=') << "\n";
            
            return true;
            
        } catch (const std::exception& e) {
            logger.error("SMF", "Failed to add QoS flow: " + std::string(e.what()));
            return false;
        }
    }

    /**
     * @brief Displays detailed information for a specific session
     */
    void displaySession(uint16_t pdu_session_id) {
        auto it = sessions.find(pdu_session_id);
        if (it != sessions.end()) {
            it->second->displaySessionInfo();
        } else {
            logger.error("SMF", "PDU Session not found: " + std::to_string(pdu_session_id));
        }
    }

    /**
     * @brief Displays a summary of all active sessions
     */
    void displayAllSessions() {
        std::cout << "\n╔════════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                      ACTIVE PDU SESSIONS SUMMARY                       ║\n";
        std::cout << "╠════════════════════════════════════════════════════════════════════════╣\n";
        
        if (sessions.empty()) {
            std::cout << "║  No active sessions                                                    ║\n";
        } else {
            std::cout << "║ ID   │ DNN             │ State      │ Slice        │ Flows │ Type    ║\n";
            std::cout << "╟──────┼─────────────────┼────────────┼──────────────┼───────┼─────────╢\n";
            
            for (const auto& [id, session] : sessions) {
                std::string dnn_padded = session->getDNN();
                dnn_padded.resize(15, ' ');
                
                std::string state_padded = session->getState();
                state_padded.resize(10, ' ');
                
                std::string snssai_str = "SST=" + std::to_string(session->getSNSSAI().sst);
                snssai_str.resize(12, ' ');
                
                std::string type = session->getSessionTypeString();
                if (type.length() > 7) type = type.substr(0, 7);
                type.resize(7, ' ');
                
                std::cout << "║ " << std::setw(4) << std::left << id 
                          << " │ " << dnn_padded
                          << " │ " << state_padded
                          << " │ " << snssai_str
                          << " │ " << std::setw(5) << session->getQoSFlows().size()
                          << " │ " << type << " ║\n";
            }
        }
        
        std::cout << "╠════════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ Total Active Sessions: " << std::setw(48) << std::left << sessions.size() << "║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════════════╝\n";
    }

    /**
     * @brief Quick test: Establish an Internet PDU session
     */
    bool testInternetSession() {
        SNSSAI snssai{1, 0x000001};  // eMBB slice (SST=1)
        return establishPDUSession(PDUSessionType::IPv4, "internet", snssai, SSCMode::SSC_MODE_1);
    }

    /**
     * @brief Quick test: Establish an IMS PDU session with voice capability
     */
    bool testIMSSession() {
        SNSSAI snssai{5, 0x000005};  // IMS slice (SST=5)
        bool result = establishPDUSession(PDUSessionType::IPv4v6, "ims", snssai, SSCMode::SSC_MODE_2);
        
        if (result && !sessions.empty()) {
            uint16_t session_id = sessions.rbegin()->first;
            // Automatically add voice QoS flow for IMS
            logger.info("SMF", "\nAutomatically adding Voice QoS flow for IMS session...");
            addQoSFlow(session_id, createVoiceQoSFlow());
        }
        
        return result;
    }

    /**
     * @brief Adds a voice QoS flow to the most recent session
     */
    void addVoiceFlow() {
        if (sessions.empty()) {
            logger.error("SMF", "No active PDU sessions. Please establish a session first.");
            return;
        }
        uint16_t session_id = sessions.rbegin()->first;
        addQoSFlow(session_id, createVoiceQoSFlow());
    }

    /**
     * @brief Adds a video QoS flow to the most recent session
     */
    void addVideoFlow() {
        if (sessions.empty()) {
            logger.error("SMF", "No active PDU sessions. Please establish a session first.");
            return;
        }
        uint16_t session_id = sessions.rbegin()->first;
        addQoSFlow(session_id, createVideoQoSFlow());
    }
    
    /**
     * @brief Adds a gaming QoS flow to the most recent session
     */
    void addGamingFlow() {
        if (sessions.empty()) {
            logger.error("SMF", "No active PDU sessions. Please establish a session first.");
            return;
        }
        uint16_t session_id = sessions.rbegin()->first;
        addQoSFlow(session_id, createGamingQoSFlow());
    }

    /**
     * @brief Shows details of the most recently created session
     */
    void showCurrentSession() {
        if (sessions.empty()) {
            logger.error("SMF", "No active PDU sessions");
            return;
        }
        uint16_t session_id = sessions.rbegin()->first;
        displaySession(session_id);
    }

    /**
     * @brief Releases a PDU session
     */
    void releaseSession(uint16_t pdu_session_id) {
        auto it = sessions.find(pdu_session_id);
        if (it == sessions.end()) {
            logger.error("SMF", "PDU Session not found: " + std::to_string(pdu_session_id));
            return;
        }

        std::cout << "\n" << std::string(80, '=') << "\n";
        logger.info("SMF", "╔════════════════════════════════════════════════════════════╗");
        logger.info("SMF", "║         PDU SESSION RELEASE PROCEDURE                      ║");
        logger.info("SMF", "╚════════════════════════════════════════════════════════════╝");
        logger.info("SMF", "Initiating PDU Session Release for Session " + std::to_string(pdu_session_id));
        
        logger.info("SMF", "Step 1: Notifying UPF to release resources");
        upf.releasePFCPSession(pdu_session_id);
        
        logger.info("SMF", "Step 2: Removing session context from SMF");
        sessions.erase(it);
        
        logger.info("SMF", "Step 3: Notifying PCF about session termination");
        // In real implementation, would notify PCF
        
        logger.success("SMF", "╔════════════════════════════════════════════════════════════╗");
        logger.success("SMF", "║   PDU SESSION RELEASED SUCCESSFULLY                        ║");
        logger.success("SMF", "╚════════════════════════════════════════════════════════════╝");
        logger.info("SMF", "PDU Session " + std::to_string(pdu_session_id) + " is now terminated");
        std::cout << std::string(80, '=') << "\n";
    }
    
    /**
     * @brief Gets total number of active sessions
     */
    size_t getActiveSessionCount() const {
        return sessions.size();
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MAIN PROGRAM - USER INTERFACE
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Displays the main menu
 */
void displayMenu() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                                      ║\n";
    std::cout << "║           5G CORE NETWORK - SMF PDU SESSION SIMULATOR                ║\n";
    std::cout << "║                     Enhanced Edition v2.1                            ║\n";
    std::cout << "║                                                                      ║\n";
    std::cout << "║                   Reference: 3GPP TS 23.502                          ║\n";
    std::cout << "║                                                                      ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║                                                                      ║\n";
    std::cout << "║  PDU SESSION ESTABLISHMENT                                           ║\n";
    std::cout << "║  ─────────────────────────                                           ║\n";
    std::cout << "║    1. Establish Internet PDU Session (IPv4, DNN=internet)            ║\n";
    std::cout << "║    2. Establish IMS PDU Session (IPv4v6, DNN=ims)                    ║\n";
    std::cout << "║                                                                      ║\n";
    std::cout << "║  QoS FLOW MANAGEMENT                                                 ║\n";
    std::cout << "║  ───────────────────                                                 ║\n";
    std::cout << "║    3. Add Voice QoS Flow (5QI=1, GBR) to Current Session             ║\n";
    std::cout << "║    4. Add Video QoS Flow (5QI=2, GBR) to Current Session             ║\n";
    std::cout << "║    5. Add Gaming QoS Flow (5QI=3, GBR) to Current Session            ║\n";
    std::cout << "║                                                                      ║\n";
    std::cout << "║  SESSION INFORMATION                                                 ║\n";
    std::cout << "║  ───────────────────                                                 ║\n";
    std::cout << "║    6. Display Current PDU Session Details                            ║\n";
    std::cout << "║    7. Display All Active Sessions                                    ║\n";
    std::cout << "║                                                                      ║\n";
    std::cout << "║  SESSION RELEASE                                                     ║\n";
    std::cout << "║  ───────────────                                                     ║\n";
    std::cout << "║    8. Release Current PDU Session                                    ║\n";
    std::cout << "║                                                                      ║\n";
    std::cout << "║    0. Exit Simulator                                                 ║\n";
    std::cout << "║                                                                      ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\nSelect option: ";
}

/**
 * @brief Main entry point
 */
int main() {
    // Display banner
    std::cout << "\n";
    std::cout << "████████████████████████████████████████████████████████████████████████\n";
    std::cout << "█                                                                      █\n";
    std::cout << "█           5G CORE NETWORK TRAINING LAB                              █\n";
    std::cout << "█           SMF PDU SESSION ESTABLISHMENT SIMULATOR                   █\n";
    std::cout << "█                   Enhanced Edition v2.1                              █\n";
    std::cout << "█                                                                      █\n";
    std::cout << "█           Layer: 5G Core (SMF - Session Management Function)        █\n";
    std::cout << "█           Standards: 3GPP TS 23.502, TS 29.502, TS 29.244           █\n";
    std::cout << "█           Difficulty: Intermediate to Advanced                       █\n";
    std::cout << "█                                                                      █\n";
    std::cout << "█           Features:                                                  █\n";
    std::cout << "█           • Full PDU Session Lifecycle Management                    █\n";
    std::cout << "█           • PFCP Protocol Simulation (N4 Interface)                  █\n";
    std::cout << "█           • QoS Flow Management with 5QI Mapping                     █\n";
    std::cout << "█           • Network Slicing (S-NSSAI) Support                        █\n";
    std::cout << "█           • Policy Control (PCF) Integration                         █\n";
    std::cout << "█           • Enhanced Error Handling & Validation                     █\n";
    std::cout << "█                                                                      █\n";
    std::cout << "████████████████████████████████████████████████████████████████████████\n";

    // Initialize SMF Simulator
    SMFSimulator smf;

    int choice;
    bool running = true;

    // Main event loop
    while (running) {
        displayMenu();
        
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "\n[ERROR] Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice) {
            case 1:
                smf.testInternetSession();
                break;
                
            case 2:
                smf.testIMSSession();
                break;
                
            case 3:
                smf.addVoiceFlow();
                break;
                
            case 4:
                smf.addVideoFlow();
                break;
                
            case 5:
                smf.addGamingFlow();
                break;
                
            case 6:
                smf.showCurrentSession();
                break;
                
            case 7:
                smf.displayAllSessions();
                break;
                
            case 8: {
                if (smf.getActiveSessionCount() == 0) {
                    std::cout << "\n[ERROR] No active sessions to release.\n";
                    break;
                }
                std::cout << "Enter PDU Session ID to release: ";
                uint16_t session_id;
                if (std::cin >> session_id) {
                    smf.releaseSession(session_id);
                } else {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    std::cout << "\n[ERROR] Invalid session ID.\n";
                }
                break;
            }
                
            case 0:
                std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
                std::cout << "║                                                              ║\n";
                std::cout << "║  Thank you for using the 5G Core SMF Simulator!             ║\n";
                std::cout << "║  Training Lab Session Complete.                             ║\n";
                std::cout << "║                                                              ║\n";
                std::cout << "║  For more information, visit:                                ║\n";
                std::cout << "║  3GPP Specifications: https://www.3gpp.org/specifications    ║\n";
                std::cout << "║                                                              ║\n";
                std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
                running = false;
                break;
                
            default:
                std::cout << "\n[ERROR] Invalid option. Please select 0-8.\n";
        }
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * END OF FILE
 * ═══════════════════════════════════════════════════════════════════════════
 */
