/**
 * ═══════════════════════════════════════════════════════════════════════════
 * 5G NR PDCP INTEGRITY PROTECTION - COMPLETE IMPLEMENTATION (Stages 1-5)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Standard Compliance: 3GPP TS 38.323 (PDCP), TS 33.501 (Security), TS 35.223
 * (NIA2)
 *
 * This implementation demonstrates the complete PDCP integrity protection flow:
 *
 *   STAGE 1: PDU Input & Parameter Setup
 *   STAGE 2: Integrity Algorithm Initialization (NIA2 - AES-CMAC)
 *   STAGE 3: MAC-I Computation (Message Authentication Code)
 *   STAGE 4: MAC-I Attachment to PDU
 *   STAGE 5: Integrity Verification at Receiver
 *
 * Author: 5G Telecom Developer
 * Date: February 2026
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPE DEFINITIONS & CONSTANTS (3GPP TS 38.323)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief PDCP Security Parameters as per 3GPP TS 33.501
 */
struct IntegrityContext {
  uint8_t integrity_key[16]; // 128-bit IK (Integrity Key) - K_RRCint or K_UPint
  uint32_t count;            // 32-bit COUNT = HFN || PDCP_SN
  uint8_t bearer;            // 5-bit Radio Bearer Identity (0-31)
  uint8_t direction;         // 1-bit: 0=Uplink (UE→gNB), 1=Downlink (gNB→UE)
};

/**
 * @brief PDCP Protocol Data Unit Structure with Integrity
 */
struct PDCP_PDU_Integrity {
  uint8_t header[2];       // PDCP Header (D/C, PDCP SN, etc.)
  uint8_t *data;           // PDCP SDU (user data)
  uint32_t data_length;    // Length of data in bytes
  uint32_t mac_i;          // 32-bit Message Authentication Code for Integrity
  bool has_mac_i;          // Flag indicating if MAC-I is present
  bool integrity_verified; // Verification status flag
};

/**
 * @brief Integrity Algorithm Selection (3GPP TS 33.501)
 */
enum IntegrityAlgorithm {
  NIA0 = 0, // Null integrity (no protection)
  NIA1 = 1, // SNOW 3G based
  NIA2 = 2, // AES-CMAC based (128-bit) - Used in this implementation
  NIA3 = 3  // ZUC based
};

/**
 * @brief Radio Bearer Type Classification
 */
enum BearerType {
  SRB = 0, // Signaling Radio Bearer (always has integrity)
  DRB = 1, // Data Radio Bearer (integrity optional, configurable)
  MRB = 2  // MBS Radio Bearer (no integrity)
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Print hex dump of byte array (for debugging and visualization)
 */
void print_hex(const std::string &label, const uint8_t *data, size_t length) {
  std::cout << label << " (length=" << length << " bytes):\n  0x";
  for (size_t i = 0; i < length; i++) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(data[i]);
    if ((i + 1) % 32 == 0 && i + 1 < length)
      std::cout << "\n  0x";
  }
  std::cout << std::dec << "\n";
}

/**
 * @brief Print binary representation of byte (for bit-level analysis)
 */
void print_binary(const std::string &label, uint8_t byte) {
  std::cout << label << ": ";
  for (int i = 7; i >= 0; i--) {
    std::cout << ((byte >> i) & 1);
  }
  std::cout << "b\n";
}

/**
 * @brief Print 32-bit value in binary (for COUNT/MAC-I analysis)
 */
void print_binary_32(const std::string &label, uint32_t value) {
  std::cout << label << ":\n  ";
  for (int i = 31; i >= 0; i--) {
    std::cout << ((value >> i) & 1);
    if (i % 8 == 0 && i > 0)
      std::cout << " ";
  }
  std::cout << "b\n";
}

/**
 * @brief Convert string to hex byte array
 */
void string_to_bytes(const std::string &str, uint8_t *output) {
  for (size_t i = 0; i < str.length(); i++) {
    output[i] = static_cast<uint8_t>(str[i]);
  }
}

/**
 * @brief Convert hex byte array to string
 */
std::string bytes_to_string(const uint8_t *data, size_t length) {
  std::string result;
  for (size_t i = 0; i < length; i++) {
    if (data[i] == 0)
      break; // Stop at null terminator
    result += static_cast<char>(data[i]);
  }
  return result;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: SIMPLIFIED AES-CMAC IMPLEMENTATION (NIA2)
// ═══════════════════════════════════════════════════════════════════════════
// NOTE: This is a SIMPLIFIED AES-CMAC for educational purposes.
// Production code MUST use OpenSSL, mbedTLS, or hardware-accelerated crypto!
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Simplified AES block encryption (DEMONSTRATION ONLY - NOT SECURE!)
 *
 * In real 5G implementations, this MUST be replaced with:
 * - OpenSSL: CMAC_Init() with EVP_aes_128_cbc()
 * - mbedTLS: mbedtls_cipher_cmac()
 * - Intel AES-NI: Hardware-accelerated AES instructions
 *
 * @warning This is a pseudo-AES for demonstration. DO NOT use in production!
 */
void simplified_aes_block_encrypt(const uint8_t *key, const uint8_t *input,
                                  uint8_t *output) {
  // This is a SIMPLIFIED substitution-permutation network for demonstration
  // Real AES uses: SubBytes, ShiftRows, MixColumns, AddRoundKey over 10 rounds

  for (int i = 0; i < 16; i++) {
    // Pseudo-encryption: combines input with key using mixing functions
    uint8_t mixed = input[i] ^ key[i];
    mixed = ((mixed << 1) | (mixed >> 7)) ^ key[(i + 7) % 16];
    mixed = (mixed ^ 0x63) + key[(i + 3) % 16];
    mixed = ((mixed << 3) | (mixed >> 5)) ^ key[(i + 11) % 16];
    output[i] = mixed;
  }
}

/**
 * @brief Simplified AES-CMAC computation (DEMONSTRATION ONLY)
 *
 * Real CMAC involves:
 * 1. Subkey generation (K1, K2 from encrypting zero block)
 * 2. Message padding
 * 3. CBC-MAC with final block XOR with subkey
 *
 * This simplified version demonstrates the concept but is NOT cryptographically
 * secure.
 *
 * @param key         128-bit integrity key
 * @param message     Message to authenticate
 * @param msg_length  Message length in bytes
 * @param mac        [out] 128-bit MAC output (32-bit MAC-I extracted from this)
 */
void simplified_aes_cmac(const uint8_t *key, const uint8_t *message,
                         size_t msg_length, uint8_t mac[16]) {
  std::cout << "  [AES-CMAC] WARNING: Using simplified CMAC (for demo only)\n";
  std::cout << "  [AES-CMAC] Production code must use OpenSSL/mbedTLS!\n\n";

  // Initialize MAC to zero
  uint8_t state[16] = {0};

  // Process message in 16-byte blocks (CBC-MAC style)
  size_t num_blocks = (msg_length + 15) / 16;

  for (size_t block = 0; block < num_blocks; block++) {
    uint8_t input_block[16] = {0};
    size_t bytes_to_copy =
        (block == num_blocks - 1) ? (msg_length - block * 16) : 16;

    // Copy message block
    memcpy(input_block, message + block * 16, bytes_to_copy);

    // XOR with previous state (CBC chaining)
    for (int i = 0; i < 16; i++) {
      input_block[i] ^= state[i];
    }

    // Encrypt the block
    simplified_aes_block_encrypt(key, input_block, state);

    std::cout << "  [CMAC] Block " << block << " processed:\n";
    std::cout << "    Input:  0x";
    for (int i = 0; i < 16; i++) {
      std::cout << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(input_block[i]);
    }
    std::cout << "\n    Output: 0x";
    for (int i = 0; i < 16; i++) {
      std::cout << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(state[i]);
    }
    std::cout << std::dec << "\n\n";
  }

  // Final MAC is the last state
  memcpy(mac, state, 16);
}

/**
 * @brief Compute NIA2 MAC-I (32-bit) from 128-bit AES-CMAC output
 *
 * As per 3GPP TS 35.223, the 32-bit MAC-I is extracted from the
 * 128-bit CMAC output (typically the most significant 32 bits).
 *
 * @param cmac_output  128-bit CMAC output
 * @return            32-bit MAC-I
 */
uint32_t extract_mac_i(const uint8_t cmac_output[16]) {
  // Extract first 32 bits (bytes 0-3) as MAC-I
  uint32_t mac_i = 0;
  mac_i = (static_cast<uint32_t>(cmac_output[0]) << 24) |
          (static_cast<uint32_t>(cmac_output[1]) << 16) |
          (static_cast<uint32_t>(cmac_output[2]) << 8) |
          (static_cast<uint32_t>(cmac_output[3]));
  return mac_i;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: STAGE 1 - PDU INPUT & PARAMETER SETUP
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 1: Prepare PDCP PDU for integrity protection
 *
 * This stage receives the PDCP PDU (BEFORE ciphering) and extracts all
 * necessary parameters for the integrity protection operation according to
 * 3GPP TS 38.323 Section 5.9.
 *
 * Steps in Stage 1:
 * 1. Parse incoming PDCP Service Data Unit (SDU)
 * 2. Construct PDCP header (D/C bit, PDCP SN)
 * 3. Construct 32-bit COUNT value (COUNT = HFN || PDCP_SN)
 * 4. Identify BEARER identity
 * 5. Determine DIRECTION (uplink/downlink)
 * 6. Validate PDU structure and prepare for MAC-I computation
 *
 * @param plaintext     User data (PDCP SDU)
 * @param length        Length of plaintext in bytes
 * @param int_ctx       Integrity context with parameters
 * @param pdu          [out] Constructed PDCP PDU structure
 * @param bearer_type   Type of radio bearer (SRB/DRB/MRB)
 *
 * @return 0 on success, negative error code on failure
 *
 * @note 3GPP TS 38.323 Section 5.9: "The data unit that is integrity protected
 *       is the PDU header and the data part of the PDU before ciphering"
 */
int stage1_prepare_pdu_for_integrity(const std::string &plaintext,
                                     IntegrityContext &int_ctx,
                                     PDCP_PDU_Integrity &pdu,
                                     BearerType bearer_type) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 1: PDU INPUT & PARAMETER SETUP FOR INTEGRITY          ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "3GPP Reference: TS 38.323 Section 5.9 (Integrity Protection)\n";
  std::cout
      << "               TS 33.501 Section 6.9 (Integrity Algorithms)\n\n";

  // Step 1.1: Determine if integrity protection is required
  std::cout << "[STEP 1.1] Integrity Protection Applicability Check\n";
  std::cout << "  → Bearer Type: ";
  switch (bearer_type) {
  case SRB:
    std::cout << "SRB (Signaling Radio Bearer)\n";
    std::cout << "  → Integrity Protection: ALWAYS APPLIED (mandatory)\n";
    pdu.has_mac_i = true;
    break;
  case DRB:
    std::cout << "DRB (Data Radio Bearer)\n";
    std::cout << "  → Integrity Protection: CONFIGURABLE (optional)\n";
    std::cout << "  → Configuration status: ENABLED for this DRB\n";
    pdu.has_mac_i = true;
    break;
  case MRB:
    std::cout << "MRB (MBS Radio Bearer)\n";
    std::cout << "  → Integrity Protection: NOT APPLIED (per spec)\n";
    pdu.has_mac_i = false;
    std::cout << "\n⚠ STAGE 1 SKIPPED: No integrity for MRBs\n";
    return 0;
  }
  std::cout << "\n";

  // Step 1.2: Parse PDCP SDU (user plaintext data)
  std::cout << "[STEP 1.2] Parsing PDCP Service Data Unit (SDU)\n";
  std::cout << "  → Plaintext message: \"" << plaintext << "\"\n";
  std::cout << "  → Message length: " << plaintext.length() << " bytes\n";
  std::cout << "  → Note: This is BEFORE ciphering (as per spec)\n\n";

  // Step 1.3: Construct PDCP Header
  // PDCP header format (Data PDU with 12-bit SN):
  //   Bit 0:     D/C bit (1 = Data PDU, 0 = Control PDU)
  //   Bits 1-3:  Reserved (R bits, set to 0)
  //   Bits 4-15: PDCP Sequence Number (SN)
  std::cout << "[STEP 1.3] Constructing PDCP Header\n";
  std::cout << "  Header Format (12-bit SN variant):\n";
  std::cout << "  ┌─────────────────────────────────────────────┐\n";
  std::cout << "  │ Bit 0    : D/C (1=Data, 0=Control)          │\n";
  std::cout << "  │ Bits 1-3 : Reserved (R, set to 0)           │\n";
  std::cout << "  │ Bits 4-15: PDCP SN (12 bits)                │\n";
  std::cout << "  └─────────────────────────────────────────────┘\n\n";

  uint16_t pdcp_sn = int_ctx.count & 0x0FFF; // Extract 12-bit SN from COUNT
  pdu.header[0] = 0x80 | ((pdcp_sn >> 8) & 0x0F); // D/C=1 (Data), SN[11:8]
  pdu.header[1] = pdcp_sn & 0xFF;                 // SN[7:0]

  std::cout << "  → PDCP Sequence Number (SN): " << pdcp_sn << " (12-bit)\n";
  std::cout << "  → D/C bit: 1 (Data PDU)\n";
  std::cout << "  → Header byte[0]: 0x" << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<int>(pdu.header[0]) << " = 0b";
  for (int i = 7; i >= 0; i--)
    std::cout << ((pdu.header[0] >> i) & 1);
  std::cout << "\n  → Header byte[1]: 0x" << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<int>(pdu.header[1]) << " = 0b";
  for (int i = 7; i >= 0; i--)
    std::cout << ((pdu.header[1] >> i) & 1);
  std::cout << std::dec << "\n\n";

  // Step 1.4: Construct 32-bit COUNT value
  // COUNT = Hyper Frame Number (HFN) || PDCP_SN
  // For 12-bit SN: COUNT[31:12] = HFN, COUNT[11:0] = PDCP_SN
  std::cout << "[STEP 1.4] Constructing 32-bit COUNT Parameter\n";
  std::cout << "  COUNT Structure:\n";
  std::cout << "  ┌──────────────────────────────────────┐\n";
  std::cout << "  │ Bits [31:12] : HFN (20 bits)         │\n";
  std::cout << "  │ Bits [11:0]  : PDCP SN (12 bits)     │\n";
  std::cout << "  └──────────────────────────────────────┘\n\n";

  uint32_t hfn = int_ctx.count >> 12; // Extract HFN (upper 20 bits)
  std::cout << "  → COUNT value: 0x" << std::hex << std::setw(8)
            << std::setfill('0') << int_ctx.count << std::dec << " ("
            << int_ctx.count << ")\n";
  std::cout << "  → HFN (Hyper Frame Number): " << hfn << " (0x" << std::hex
            << hfn << std::dec << ")\n";
  std::cout << "  → PDCP SN: " << pdcp_sn << " (0x" << std::hex << pdcp_sn
            << std::dec << ")\n";
  std::cout << "  → Formula: COUNT = (HFN << 12) | PDCP_SN\n";

  print_binary_32("  → COUNT (binary)", int_ctx.count);
  std::cout << "    └─HFN─────────────────┘└─PDCP SN──┘\n\n";

  // Step 1.5: Extract Bearer and Direction
  std::cout << "[STEP 1.5] Identifying Bearer and Direction Parameters\n";
  std::cout << "  → BEARER ID: " << static_cast<int>(int_ctx.bearer)
            << " (5-bit value, range: 0-31)\n";
  std::cout << "  → BEARER calculation: RB identity - 1 (per TS 33.501)\n";
  std::cout << "  → DIRECTION: " << static_cast<int>(int_ctx.direction) << " (";
  if (int_ctx.direction == 0) {
    std::cout << "Uplink: UE → gNB)\n";
  } else {
    std::cout << "Downlink: gNB → UE)\n";
  }
  std::cout << "  → DIRECTION is set as per TS 33.501 Section 6.9.3\n\n";

  // Step 1.6: Allocate and copy plaintext data
  std::cout << "[STEP 1.6] Allocating PDU Data Buffer\n";
  pdu.data_length = plaintext.length();
  pdu.data = new uint8_t[pdu.data_length];
  string_to_bytes(plaintext, pdu.data);
  pdu.integrity_verified = false;

  print_hex("  → Plaintext data (hex)", pdu.data, pdu.data_length);

  // Step 1.7: Display complete input parameters for integrity algorithm
  std::cout << "\n[STAGE 1 SUMMARY] Input Parameters for NIA2 Algorithm:\n";
  std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
  std::cout << "  │ Parameter       │ Value                               │\n";
  std::cout << "  ├──────────────────────────────────────────────────────┤\n";
  std::cout << "  │ COUNT           │ 0x" << std::hex << std::setw(8)
            << std::setfill('0') << int_ctx.count << std::dec
            << " (32-bit)         │\n";
  std::cout << "  │ BEARER          │ " << std::setw(2)
            << static_cast<int>(int_ctx.bearer)
            << " (5-bit)                             │\n";
  std::cout << "  │ DIRECTION       │ " << static_cast<int>(int_ctx.direction)
            << " (" << (int_ctx.direction == 0 ? "UL" : "DL")
            << ")                                   │\n";
  std::cout << "  │ MESSAGE LENGTH  │ " << std::setw(4)
            << ((2 + pdu.data_length) * 8)
            << " bits (Header+Data)          │\n";
  std::cout << "  │ BEARER TYPE     │ "
            << (bearer_type == SRB ? "SRB"
                                   : (bearer_type == DRB ? "DRB" : "MRB"))
            << "                                  │\n";
  std::cout << "  └──────────────────────────────────────────────────────┘\n";

  std::cout << "\n[IMPORTANT] Integrity Protection Timing:\n";
  std::cout << "  ✓ Integrity protection is performed BEFORE ciphering\n";
  std::cout << "  ✓ MAC-I is computed over PDU header + plaintext data\n";
  std::cout << "  ✓ MAC-I itself is then ciphered along with data\n";
  std::cout << "  ✓ This ensures both integrity and confidentiality\n";

  std::cout << "\n✓ STAGE 1 COMPLETE: PDU prepared for integrity protection\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5: STAGE 2 - INTEGRITY ALGORITHM INITIALIZATION (NIA2)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 2: Initialize NIA2 integrity algorithm
 *
 * NIA2 uses AES-CMAC (Cipher-based Message Authentication Code) as specified
 * in 3GPP TS 35.223. This stage prepares the integrity algorithm with the
 * proper key and parameters.
 *
 * NIA2 Input Parameters (as per TS 35.223 Section 4):
 *   - IK: 128-bit Integrity Key (K_RRCint or K_UPint)
 *   - COUNT: 32-bit counter value
 *   - BEARER: 5-bit bearer identity
 *   - DIRECTION: 1-bit direction indicator
 *   - MESSAGE: Variable length message (Header + Data)
 *
 * @param int_ctx       Integrity context with key
 * @param pdu           PDCP PDU to be integrity protected
 * @param message      [out] Constructed message for MAC-I computation
 * @param msg_length   [out] Length of constructed message
 *
 * @return 0 on success
 *
 * @note 3GPP TS 35.223 Section 4: Algorithm specification for 128-NIA2
 */
int stage2_initialize_integrity_algorithm(const IntegrityContext &int_ctx,
                                          const PDCP_PDU_Integrity &pdu,
                                          uint8_t **message,
                                          size_t &msg_length) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 2: INTEGRITY ALGORITHM INITIALIZATION (NIA2 AES-CMAC) ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "3GPP Reference: TS 35.223 Section 4 (128-NIA2 Algorithm)\n";
  std::cout << "               RFC 4493 (AES-CMAC Algorithm Specification)\n\n";

  // Step 2.1: Display integrity algorithm information
  std::cout << "[STEP 2.1] Integrity Algorithm: 128-NIA2 (AES-128 CMAC)\n";
  std::cout << "  → Algorithm Type: CMAC (Cipher-based MAC)\n";
  std::cout << "  → Key size: 128 bits (16 bytes)\n";
  std::cout << "  → Output size: 32 bits (4 bytes) - MAC-I\n";
  std::cout << "  → Security: Provides message authentication and integrity\n";
  std::cout << "  → Attacks prevented: Modification, forgery, replay (via "
               "COUNT)\n\n";

  // Step 2.2: Load integrity key (IK)
  std::cout << "[STEP 2.2] Loading 128-bit Integrity Key (IK)\n";
  print_hex("  → Integrity Key (IK)", int_ctx.integrity_key, 16);
  std::cout << "  → Key derivation: IK = KDF(K_gNB, algorithm-ID, ...)\n";
  std::cout << "  → For Control Plane (SRB): IK = K_RRCint\n";
  std::cout << "  → For User Plane (DRB): IK = K_UPint\n";
  std::cout << "  → Key is derived per TS 33.501 Annex A (KDF)\n\n";

  // Step 2.3: Construct input message for MAC-I computation
  std::cout << "[STEP 2.3] Constructing Message for MAC-I Computation\n";
  std::cout << "  Message Structure (as per TS 35.223):\n";
  std::cout << "  ┌─────────────────────────────────────────────────────┐\n";
  std::cout << "  │ Bytes [0-3]   : COUNT (32 bits, big-endian)         │\n";
  std::cout << "  │ Byte  [4]     : BEARER || DIRECTION || 0 (padding)  │\n";
  std::cout << "  │ Bytes [5-7]   : Zero padding                        │\n";
  std::cout << "  │ Bytes [8-...]  : PDCP PDU (Header + Data)           │\n";
  std::cout << "  └─────────────────────────────────────────────────────┘\n\n";

  // Calculate message length: 8 bytes (COUNT+BEARER+DIR+padding) + PDU
  msg_length = 8 + 2 + pdu.data_length; // 8 + header(2) + data
  *message = new uint8_t[msg_length];
  memset(*message, 0, msg_length);

  // Bytes 0-3: COUNT (32 bits, big-endian)
  (*message)[0] = (int_ctx.count >> 24) & 0xFF;
  (*message)[1] = (int_ctx.count >> 16) & 0xFF;
  (*message)[2] = (int_ctx.count >> 8) & 0xFF;
  (*message)[3] = int_ctx.count & 0xFF;

  // Byte 4: BEARER (5 bits) || DIRECTION (1 bit) || 0 (2 bits padding)
  (*message)[4] = (int_ctx.bearer << 3) | (int_ctx.direction << 2);

  // Bytes 5-7: Zero padding
  (*message)[5] = 0x00;
  (*message)[6] = 0x00;
  (*message)[7] = 0x00;

  // Bytes 8-9: PDCP Header
  (*message)[8] = pdu.header[0];
  (*message)[9] = pdu.header[1];

  // Bytes 10+: PDCP Data (plaintext, before ciphering)
  memcpy(*message + 10, pdu.data, pdu.data_length);

  std::cout << "  [STEP 2.3.1] Message Component Breakdown:\n";
  std::cout << "    Bytes 0-3 (COUNT):   0x";
  for (int i = 0; i < 4; i++) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>((*message)[i]);
  }
  std::cout << std::dec << "\n";

  std::cout << "    Byte 4 (BEARER|DIR): 0x" << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<int>((*message)[4]) << std::dec
            << " = 0b";
  for (int i = 7; i >= 0; i--)
    std::cout << (((*message)[4] >> i) & 1);
  std::cout << "\n";
  std::cout << "                         └─BEARER─┘└D┘└─0─┘\n";
  std::cout << "                          (5 bits)  (1)(2 bits)\n\n";

  std::cout << "    Bytes 5-7 (Padding): 0x000000\n";
  std::cout << "    Bytes 8-9 (Header):  0x" << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<int>((*message)[8])
            << std::setw(2) << std::setfill('0')
            << static_cast<int>((*message)[9]) << std::dec << "\n";
  std::cout << "    Bytes 10+ (Data):    \""
            << bytes_to_string(pdu.data, pdu.data_length) << "\"\n\n";

  print_hex("  → Complete Message for MAC-I", *message, msg_length);

  std::cout << "\n[STEP 2.4] Message Length Calculation\n";
  std::cout << "  → COUNT + Padding: 8 bytes\n";
  std::cout << "  → PDCP Header: 2 bytes\n";
  std::cout << "  → PDCP Data: " << pdu.data_length << " bytes\n";
  std::cout << "  → Total message length: " << msg_length << " bytes ("
            << (msg_length * 8) << " bits)\n";

  std::cout
      << "\n✓ STAGE 2 COMPLETE: NIA2 algorithm initialized with message\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 6: STAGE 3 - MAC-I COMPUTATION (AES-CMAC)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 3: Compute MAC-I using AES-CMAC
 *
 * This stage performs the actual integrity protection computation:
 * 1. Apply AES-CMAC algorithm to the message
 * 2. Generate 128-bit CMAC output
 * 3. Extract 32-bit MAC-I from CMAC output
 *
 * Process (as per RFC 4493 and TS 35.223):
 * - Generate subkeys K1 and K2 from encrypting zero block
 * - Divide message into 16-byte blocks
 * - Apply CBC-MAC with AES encryption
 * - XOR final block with appropriate subkey
 * - Extract first 32 bits as MAC-I
 *
 * @param int_ctx   Integrity context with key
 * @param message   Message prepared in Stage 2
 * @param msg_length Message length
 * @param mac_i    [out] 32-bit MAC-I value
 *
 * @return 0 on success
 *
 * @note The MAC-I provides integrity protection but NOT confidentiality
 */
int stage3_compute_mac_i(const IntegrityContext &int_ctx,
                         const uint8_t *message, size_t msg_length,
                         uint32_t &mac_i) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 3: MAC-I COMPUTATION (AES-CMAC ALGORITHM)             ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "3GPP Reference: TS 35.223 Section 4 (MAC-I generation)\n";
  std::cout << "               RFC 4493 (AES-CMAC specification)\n\n";

  // Step 3.1: Display MAC-I computation overview
  std::cout << "[STEP 3.1] MAC-I Computation Overview\n";
  std::cout << "  AES-CMAC Process:\n";
  std::cout << "  1. Generate subkeys K1, K2 from AES(IK, 0^128)\n";
  std::cout << "  2. Divide message into 128-bit blocks\n";
  std::cout << "  3. Apply CBC-MAC (Cipher Block Chaining)\n";
  std::cout << "  4. XOR final block with K1 or K2 (depending on padding)\n";
  std::cout << "  5. Extract first 32 bits as MAC-I\n\n";

  std::cout << "  → Input message length: " << msg_length << " bytes ("
            << (msg_length * 8) << " bits)\n";
  std::cout << "  → Number of AES blocks: " << ((msg_length + 15) / 16)
            << " (rounded up to 16-byte blocks)\n\n";

  // Step 3.2: Compute AES-CMAC (128-bit output)
  std::cout << "[STEP 3.2] Computing AES-CMAC\n";
  uint8_t cmac_output[16];
  simplified_aes_cmac(int_ctx.integrity_key, message, msg_length, cmac_output);

  std::cout << "  [STEP 3.2.1] Full 128-bit CMAC Output:\n";
  print_hex("    → CMAC-128", cmac_output, 16);

  // Step 3.3: Extract 32-bit MAC-I
  std::cout << "\n[STEP 3.3] Extracting 32-bit MAC-I from CMAC Output\n";
  std::cout << "  MAC-I Extraction Method (per TS 35.223):\n";
  std::cout << "  → Take most significant 32 bits (first 4 bytes)\n";
  std::cout << "  → Alternatively: MAC-I = CMAC[0-31]\n\n";

  mac_i = extract_mac_i(cmac_output);

  std::cout << "  [STEP 3.3.1] MAC-I Byte Extraction:\n";
  std::cout << "    Byte 0: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cmac_output[0]) << " (bits [31:24])\n";
  std::cout << "    Byte 1: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cmac_output[1]) << " (bits [23:16])\n";
  std::cout << "    Byte 2: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cmac_output[2]) << " (bits [15:8])\n";
  std::cout << "    Byte 3: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cmac_output[3]) << std::dec
            << " (bits [7:0])\n\n";

  std::cout << "  [STEP 3.3.2] Complete MAC-I Value:\n";
  std::cout << "    → MAC-I (hex): 0x" << std::hex << std::setw(8)
            << std::setfill('0') << mac_i << std::dec << "\n";
  std::cout << "    → MAC-I (decimal): " << mac_i << "\n";

  print_binary_32("    → MAC-I (binary)", mac_i);

  // Step 3.4: Display MAC-I properties
  std::cout << "\n[STAGE 3 SUMMARY] MAC-I Properties:\n";
  std::cout << "  ✓ MAC-I is deterministic (same inputs → same MAC-I)\n";
  std::cout << "  ✓ MAC-I is bound to COUNT (prevents replay attacks)\n";
  std::cout
      << "  ✓ MAC-I is bound to BEARER and DIRECTION (context-specific)\n";
  std::cout << "  ✓ MAC-I covers both header and data (complete protection)\n";
  std::cout << "  ✓ Any modification changes MAC-I (tamper detection)\n";
  std::cout << "  ✓ Cannot be forged without knowledge of IK (security)\n";

  std::cout << "\n✓ STAGE 3 COMPLETE: MAC-I successfully computed\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 7: STAGE 4 - MAC-I ATTACHMENT TO PDU
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 4: Attach MAC-I to PDCP PDU
 *
 * This stage constructs the complete PDCP PDU with integrity protection:
 *
 * PDCP Data PDU format (with integrity):
 *   +----------------+------------------+------------------+
 *   | PDCP Header    | PDCP Data        | MAC-I            |
 *   | (2 bytes)      | (variable)       | (4 bytes)        |
 *   +----------------+------------------+------------------+
 *
 * Important notes:
 * - MAC-I is appended AFTER the data
 * - MAC-I will be ciphered along with data in the next stage
 * - For SRBs, MAC-I is always present (even if integrity not configured,
 *   it's padded with zeros)
 *
 * @param pdu       PDCP PDU structure
 * @param mac_i     Computed MAC-I value
 * @param output_pdu [out] Complete PDU with MAC-I
 * @param output_length [out] Total PDU length
 *
 * @return 0 on success
 *
 * @note 3GPP TS 38.323 Section 6.2: PDCP Data PDU format
 *       Section 6.3.4: MAC-I field description
 */
int stage4_attach_mac_i(PDCP_PDU_Integrity &pdu, uint32_t mac_i,
                        uint8_t *output_pdu, size_t &output_length) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 4: MAC-I ATTACHMENT TO PDU                            ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "3GPP Reference: TS 38.323 Section 6.2 (PDCP Data PDU format)\n";
  std::cout << "               Section 6.3.4 (MAC-I field)\n\n";

  // Step 4.1: Display PDU structure
  std::cout << "[STEP 4.1] PDCP Data PDU Structure with MAC-I\n";
  std::cout << "  Complete PDU Format:\n";
  std::cout
      << "  ┌────────────────────────────────────────────────────────────┐\n";
  std::cout
      << "  │ PDCP Header  │ PDCP Data (Plaintext) │ MAC-I              │\n";
  std::cout
      << "  │ (2 bytes)    │ (variable length)     │ (4 bytes)          │\n";
  std::cout
      << "  └────────────────────────────────────────────────────────────┘\n";
  std::cout << "  │←─ Will be ciphered in next stage ─────────────────────→│\n";
  std::cout << "    (Header NOT ciphered)  (Data + MAC-I ARE ciphered)\n\n";

  // Step 4.2: Construct PDU components
  std::cout << "[STEP 4.2] PDU Component Assembly\n";

  // Copy header (2 bytes)
  output_pdu[0] = pdu.header[0];
  output_pdu[1] = pdu.header[1];

  std::cout << "  [4.2.1] PDCP Header (Plaintext):\n";
  std::cout << "    Byte 0: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(pdu.header[0]) << " (D/C + SN[11:8])\n";
  std::cout << "    Byte 1: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(pdu.header[1]) << std::dec << " (SN[7:0])\n\n";

  // Copy data (variable length)
  memcpy(output_pdu + 2, pdu.data, pdu.data_length);

  std::cout << "  [4.2.2] PDCP Data (Plaintext, before ciphering):\n";
  std::cout << "    Length: " << pdu.data_length << " bytes\n";
  print_hex("    Data", pdu.data, pdu.data_length);

  // Append MAC-I (4 bytes, big-endian)
  output_pdu[2 + pdu.data_length] = (mac_i >> 24) & 0xFF;
  output_pdu[2 + pdu.data_length + 1] = (mac_i >> 16) & 0xFF;
  output_pdu[2 + pdu.data_length + 2] = (mac_i >> 8) & 0xFF;
  output_pdu[2 + pdu.data_length + 3] = mac_i & 0xFF;

  std::cout << "\n  [4.2.3] MAC-I (32-bit, big-endian):\n";
  std::cout << "    Byte 0: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << ((mac_i >> 24) & 0xFF) << " (MSB)\n";
  std::cout << "    Byte 1: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << ((mac_i >> 16) & 0xFF) << "\n";
  std::cout << "    Byte 2: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << ((mac_i >> 8) & 0xFF) << "\n";
  std::cout << "    Byte 3: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << (mac_i & 0xFF) << std::dec << " (LSB)\n";
  std::cout << "    Full MAC-I: 0x" << std::hex << std::setw(8)
            << std::setfill('0') << mac_i << std::dec << "\n\n";

  // Calculate total length
  output_length = 2 + pdu.data_length + 4; // header + data + MAC-I

  // Update PDU structure
  pdu.mac_i = mac_i;
  pdu.has_mac_i = true;

  std::cout << "[STEP 4.3] Complete PDU Details\n";
  print_hex("  → Complete PDU (before ciphering)", output_pdu, output_length);

  std::cout << "\n  PDU Size Breakdown:\n";
  std::cout << "  ┌──────────────────────────────────────┐\n";
  std::cout << "  │ Component    │ Size (bytes)          │\n";
  std::cout << "  ├──────────────────────────────────────┤\n";
  std::cout << "  │ Header       │ 2                     │\n";
  std::cout << "  │ Data         │ " << std::setw(2) << pdu.data_length
            << "                    │\n";
  std::cout << "  │ MAC-I        │ 4                     │\n";
  std::cout << "  ├──────────────────────────────────────┤\n";
  std::cout << "  │ TOTAL        │ " << std::setw(2) << output_length
            << "                    │\n";
  std::cout << "  └──────────────────────────────────────┘\n";

  std::cout << "\n[STEP 4.4] Next Processing Steps\n";
  std::cout
      << "  ⚠ IMPORTANT: Order of operations per TS 38.323 Section 5.2.1\n";
  std::cout << "  1. ✓ Integrity protection (MAC-I) - COMPLETED\n";
  std::cout << "  2. → Ciphering - NEXT STEP\n";
  std::cout << "     - Header: NOT ciphered (needed for routing)\n";
  std::cout << "     - Data + MAC-I: CIPHERED together\n";
  std::cout << "  3. → Transmission to lower layers\n";

  std::cout
      << "\n✓ STAGE 4 COMPLETE: MAC-I attached, PDU ready for ciphering\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 8: STAGE 5 - INTEGRITY VERIFICATION AT RECEIVER
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 5: Verify integrity at receiver
 *
 * The receiver performs the following steps:
 * 1. Receive PDU (after deciphering)
 * 2. Extract received MAC-I from PDU
 * 3. Compute expected MAC-I (X-MAC) using same inputs as transmitter
 * 4. Compare received MAC-I with computed X-MAC
 * 5. Accept PDU if MAC-I == X-MAC, discard if different
 *
 * As per TS 38.323 Section 5.9:
 * "the UE computes the value of the MAC-I field and at reception it verifies
 *  the integrity of the PDCP Data PDU by calculating the X-MAC based on the
 *  input parameters. If the calculated X-MAC corresponds to the received MAC-I,
 *  integrity protection is verified successfully."
 *
 * @param received_pdu  Received PDU (after deciphering)
 * @param pdu_length    Total PDU length
 * @param int_ctx       Receiver's integrity context (synchronized)
 * @param is_valid     [out] Verification result
 *
 * @return 0 on success (regardless of verification result)
 *
 * @note If verification fails, PDU is discarded and upper layers are notified
 */
int stage5_verify_integrity(const uint8_t *received_pdu, size_t pdu_length,
                            IntegrityContext &int_ctx, bool &is_valid) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 5: INTEGRITY VERIFICATION AT RECEIVER                 ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout
      << "3GPP Reference: TS 38.323 Section 5.2.2.1 (Receive operation)\n";
  std::cout << "               Section 5.9 (Integrity verification)\n\n";

  // Step 5.1: Extract received MAC-I from PDU
  std::cout << "[STEP 5.1] Extract Received MAC-I from PDU\n";
  std::cout << "  → Total PDU length: " << pdu_length << " bytes\n";
  std::cout << "  → MAC-I location: Last 4 bytes of PDU\n";
  std::cout << "  → MAC-I offset: " << (pdu_length - 4) << "\n\n";

  if (pdu_length < 6) { // minimum: 2 (header) + 0 (data) + 4 (MAC-I)
    std::cout << "  ✗ ERROR: PDU too short to contain MAC-I\n";
    is_valid = false;
    return -1;
  }

  uint32_t received_mac_i = 0;
  received_mac_i = (static_cast<uint32_t>(received_pdu[pdu_length - 4]) << 24) |
                   (static_cast<uint32_t>(received_pdu[pdu_length - 3]) << 16) |
                   (static_cast<uint32_t>(received_pdu[pdu_length - 2]) << 8) |
                   (static_cast<uint32_t>(received_pdu[pdu_length - 1]));

  std::cout << "  [STEP 5.1.1] Extracted MAC-I Bytes:\n";
  std::cout << "    Byte 0: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(received_pdu[pdu_length - 4]) << "\n";
  std::cout << "    Byte 1: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(received_pdu[pdu_length - 3]) << "\n";
  std::cout << "    Byte 2: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(received_pdu[pdu_length - 2]) << "\n";
  std::cout << "    Byte 3: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(received_pdu[pdu_length - 1]) << std::dec
            << "\n\n";

  std::cout << "  → Received MAC-I: 0x" << std::hex << std::setw(8)
            << std::setfill('0') << received_mac_i << std::dec << " ("
            << received_mac_i << ")\n";

  // Step 5.2: Extract PDCP header and determine COUNT
  std::cout << "\n[STEP 5.2] Extract PDCP Header and Reconstruct COUNT\n";
  uint16_t received_sn = ((received_pdu[0] & 0x0F) << 8) | received_pdu[1];
  std::cout << "  → Received PDCP SN: " << received_sn << "\n";

  // In real implementation, receiver maintains HFN state and reconstructs COUNT
  // For this demo, we use the synchronized COUNT from context
  std::cout << "  → COUNT (synchronized): 0x" << std::hex << std::setw(8)
            << std::setfill('0') << int_ctx.count << std::dec << " ("
            << int_ctx.count << ")\n";
  std::cout << "  → Note: In practice, COUNT is reconstructed from HFN state\n";
  std::cout << "         (see TS 38.323 Section 5.2.2.1 for algorithm)\n\n";

  // Step 5.3: Reconstruct message for X-MAC computation
  std::cout << "[STEP 5.3] Reconstruct Message for X-MAC Computation\n";
  std::cout << "  → Message components must match transmitter exactly:\n";
  std::cout << "    - COUNT (from reconstructed value)\n";
  std::cout << "    - BEARER (from security context)\n";
  std::cout << "    - DIRECTION (from security context)\n";
  std::cout << "    - PDCP Header (from received PDU)\n";
  std::cout << "    - PDCP Data (from received PDU, excluding MAC-I)\n\n";

  size_t data_length = pdu_length - 6; // Total - header(2) - MAC-I(4)
  size_t msg_length = 8 + 2 + data_length;
  uint8_t *verification_message = new uint8_t[msg_length];
  memset(verification_message, 0, msg_length);

  // Construct message same as transmitter
  verification_message[0] = (int_ctx.count >> 24) & 0xFF;
  verification_message[1] = (int_ctx.count >> 16) & 0xFF;
  verification_message[2] = (int_ctx.count >> 8) & 0xFF;
  verification_message[3] = int_ctx.count & 0xFF;
  verification_message[4] = (int_ctx.bearer << 3) | (int_ctx.direction << 2);
  verification_message[5] = 0x00;
  verification_message[6] = 0x00;
  verification_message[7] = 0x00;
  verification_message[8] = received_pdu[0]; // Header
  verification_message[9] = received_pdu[1];
  memcpy(verification_message + 10, received_pdu + 2, data_length); // Data only

  print_hex("  → Verification Message", verification_message, msg_length);

  // Step 5.4: Compute X-MAC (expected MAC-I)
  std::cout << "\n[STEP 5.4] Compute X-MAC (Expected MAC-I)\n";
  std::cout << "  → Using same NIA2 algorithm as transmitter\n";
  std::cout << "  → Same integrity key (IK)\n";
  std::cout << "  → Same input parameters (COUNT, BEARER, DIRECTION)\n";
  std::cout << "  → Same message content (Header + Data)\n\n";

  uint8_t cmac_output[16];
  simplified_aes_cmac(int_ctx.integrity_key, verification_message, msg_length,
                      cmac_output);

  uint32_t computed_mac_i = extract_mac_i(cmac_output);

  std::cout << "  → Computed X-MAC: 0x" << std::hex << std::setw(8)
            << std::setfill('0') << computed_mac_i << std::dec << " ("
            << computed_mac_i << ")\n";

  // Step 5.5: Compare MAC-I with X-MAC
  std::cout
      << "\n[STEP 5.5] Integrity Verification: Compare MAC-I with X-MAC\n";
  std::cout << "  ┌────────────────────────────────────────────────────┐\n";
  std::cout << "  │ Parameter          │ Value                         │\n";
  std::cout << "  ├────────────────────────────────────────────────────┤\n";
  std::cout << "  │ Received MAC-I     │ 0x" << std::hex << std::setw(8)
            << std::setfill('0') << received_mac_i << std::dec
            << "              │\n";
  std::cout << "  │ Computed X-MAC     │ 0x" << std::hex << std::setw(8)
            << std::setfill('0') << computed_mac_i << std::dec
            << "              │\n";
  std::cout << "  └────────────────────────────────────────────────────┘\n\n";

  is_valid = (received_mac_i == computed_mac_i);

  std::cout << "  [STEP 5.5.1] Bit-by-Bit Comparison:\n";
  print_binary_32("    Received MAC-I", received_mac_i);
  print_binary_32("    Computed X-MAC", computed_mac_i);

  if (is_valid) {
    std::cout << "\n  ┌────────────────────────────────────────────────┐\n";
    std::cout << "  │ ✓ INTEGRITY VERIFICATION: SUCCESS              │\n";
    std::cout << "  │                                                │\n";
    std::cout << "  │ MAC-I == X-MAC                                 │\n";
    std::cout << "  │ → PDU has NOT been modified                    │\n";
    std::cout << "  │ → PDU is authentic                             │\n";
    std::cout << "  │ → PDU is accepted for further processing       │\n";
    std::cout << "  └────────────────────────────────────────────────┘\n";
  } else {
    std::cout << "\n  ┌────────────────────────────────────────────────┐\n";
    std::cout << "  │ ✗ INTEGRITY VERIFICATION: FAILURE              │\n";
    std::cout << "  │                                                │\n";
    std::cout << "  │ MAC-I ≠ X-MAC                                  │\n";
    std::cout << "  │ → PDU has been MODIFIED or CORRUPTED           │\n";
    std::cout << "  │ → Possible attack or transmission error        │\n";
    std::cout << "  │ → PDU is DISCARDED (not delivered to upper)    │\n";
    std::cout << "  │ → Upper layer is NOTIFIED of failure           │\n";
    std::cout << "  └────────────────────────────────────────────────┘\n";

    std::cout << "\n  [SECURITY INCIDENT] Per TS 38.323 Section 5.2.2.1:\n";
    std::cout << "    \"if integrity verification fails:\n";
    std::cout << "     - indicate the integrity verification failure to upper "
                 "layer;\n";
    std::cout << "     - discard the PDCP Data PDU and consider it as not "
                 "received\"\n";
  }

  // Step 5.6: Display security analysis
  std::cout << "\n[STAGE 5 SUMMARY] Security Properties Verified:\n";
  if (is_valid) {
    std::cout << "  ✓ Message Authentication: Verified (PDU from legitimate "
                 "sender)\n";
    std::cout << "  ✓ Message Integrity: Verified (PDU not modified in "
                 "transit)\n";
    std::cout << "  ✓ Replay Protection: Enforced (via COUNT value)\n";
    std::cout << "  ✓ Context Binding: Verified (correct BEARER and "
                 "DIRECTION)\n";
  } else {
    std::cout << "  ✗ Message Authentication: FAILED\n";
    std::cout << "  ✗ Message Integrity: COMPROMISED\n";
    std::cout << "  ⚠ Possible causes:\n";
    std::cout << "    - Intentional modification (attack)\n";
    std::cout << "    - Bit errors in transmission\n";
    std::cout << "    - COUNT desynchronization\n";
    std::cout << "    - Wrong integrity key used\n";
  }

  delete[] verification_message;

  std::cout << "\n✓ STAGE 5 COMPLETE: Integrity verification "
            << (is_valid ? "PASSED" : "FAILED") << "\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 9: DEMONSTRATION SCENARIOS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Demonstrate complete integrity protection flow
 */
void demonstrate_integrity_protection() {
  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "  SCENARIO 1: SUCCESSFUL INTEGRITY PROTECTION & VERIFICATION\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  // Setup
  std::string message = "Critical 5G signaling message - must be protected!";
  IntegrityContext int_ctx;

  // Set 128-bit integrity key (K_RRCint derived from K_gNB)
  const char *key_hex = "FEDCBA9876543210FEDCBA9876543210";
  for (int i = 0; i < 16; i++) {
    sscanf(key_hex + i * 2, "%2hhx", &int_ctx.integrity_key[i]);
  }

  int_ctx.count = 0x00000042; // COUNT value
  int_ctx.bearer = 1;         // SRB1 (bearer = 1)
  int_ctx.direction = 0;      // Uplink

  std::cout << "\n[TRANSMITTER] UE sends protected signaling message\n";
  std::cout << "  → Message: \"" << message << "\"\n";
  std::cout << "  → Bearer: SRB1 (always integrity protected)\n";

  // Transmitter side
  PDCP_PDU_Integrity tx_pdu;
  uint8_t *integrity_message = nullptr;
  size_t msg_length;
  uint32_t mac_i;
  uint8_t *complete_pdu = new uint8_t[message.length() + 6];
  size_t complete_pdu_length;

  auto start_time = std::chrono::high_resolution_clock::now();

  stage1_prepare_pdu_for_integrity(message, int_ctx, tx_pdu, SRB);
  stage2_initialize_integrity_algorithm(int_ctx, tx_pdu, &integrity_message,
                                        msg_length);
  stage3_compute_mac_i(int_ctx, integrity_message, msg_length, mac_i);
  stage4_attach_mac_i(tx_pdu, mac_i, complete_pdu, complete_pdu_length);

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - start_time);

  std::cout << "\n[TRANSMISSION] Integrity-protected PDU transmitted...\n";
  std::cout << "  → Processing time: " << duration.count() << " microseconds\n";

  // Receiver side
  std::cout << "\n[RECEIVER] gNB receives and verifies message\n";
  bool is_valid;
  stage5_verify_integrity(complete_pdu, complete_pdu_length, int_ctx, is_valid);

  // Cleanup
  delete[] tx_pdu.data;
  delete[] integrity_message;
  delete[] complete_pdu;

  std::cout << "\n" << std::string(80, '-') << "\n";
  std::cout << "SCENARIO 1 RESULT: "
            << (is_valid ? "✓ SUCCESS - Message integrity verified"
                         : "✗ FAILED - Integrity check failed")
            << "\n";
  std::cout << std::string(80, '-') << "\n";
}

/**
 * @brief Demonstrate integrity verification failure (tampered PDU)
 */
void demonstrate_tampered_pdu() {
  std::cout << "\n\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout
      << "  SCENARIO 2: INTEGRITY VERIFICATION FAILURE (ATTACK DETECTED)\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  // Setup
  std::string message = "Transfer $1000 to account";
  IntegrityContext int_ctx;

  const char *key_hex = "0F1E2D3C4B5A69788796A5B4C3D2E1F0";
  for (int i = 0; i < 16; i++) {
    sscanf(key_hex + i * 2, "%2hhx", &int_ctx.integrity_key[i]);
  }

  int_ctx.count = 0x00000100;
  int_ctx.bearer = 5; // DRB
  int_ctx.direction = 0;

  std::cout << "\n[TRANSMITTER] UE sends protected transaction\n";
  std::cout << "  → Original message: \"" << message << "\"\n";

  // Transmitter side
  PDCP_PDU_Integrity tx_pdu;
  uint8_t *integrity_message = nullptr;
  size_t msg_length;
  uint32_t mac_i;
  uint8_t *complete_pdu = new uint8_t[message.length() + 6];
  size_t complete_pdu_length;

  stage1_prepare_pdu_for_integrity(message, int_ctx, tx_pdu, DRB);
  stage2_initialize_integrity_algorithm(int_ctx, tx_pdu, &integrity_message,
                                        msg_length);
  stage3_compute_mac_i(int_ctx, integrity_message, msg_length, mac_i);
  stage4_attach_mac_i(tx_pdu, mac_i, complete_pdu, complete_pdu_length);

  // ATTACK: Modify the PDU in transit
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════╗\n";
  std::cout << "║  ⚠ ATTACK SIMULATION: PDU TAMPERING                      ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════╝\n";
  std::cout << "\n[ATTACKER] Intercepting PDU and modifying data...\n";
  std::cout << "  → Original byte at position 10: 0x" << std::hex
            << std::setw(2) << std::setfill('0')
            << static_cast<int>(complete_pdu[10]) << std::dec << " ('"
            << static_cast<char>(complete_pdu[10]) << "')\n";

  complete_pdu[10] = '9'; // Change '1' to '9' (now "$9000")

  std::cout << "  → Modified byte at position 10: 0x" << std::hex
            << std::setw(2) << std::setfill('0')
            << static_cast<int>(complete_pdu[10]) << std::dec << " ('"
            << static_cast<char>(complete_pdu[10]) << "')\n";
  std::cout << "  → Tampered message would read: \"Transfer $9000 to "
               "account\"\n";
  std::cout << "  → MAC-I remains unchanged (attacker cannot forge it)\n";

  // Receiver side
  std::cout << "\n[RECEIVER] gNB receives and verifies (detects tampering)\n";
  bool is_valid;
  stage5_verify_integrity(complete_pdu, complete_pdu_length, int_ctx, is_valid);

  // Cleanup
  delete[] tx_pdu.data;
  delete[] integrity_message;
  delete[] complete_pdu;

  std::cout << "\n" << std::string(80, '-') << "\n";
  std::cout << "SCENARIO 2 RESULT: "
            << (!is_valid ? "✓ SUCCESS - Attack detected and blocked"
                          : "✗ FAILED - Attack went undetected (CRITICAL!)")
            << "\n";
  std::cout << std::string(80, '-') << "\n";
}

/**
 * @brief Demonstrate different bearer types
 */
void demonstrate_bearer_types() {
  std::cout << "\n\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "  SCENARIO 3: INTEGRITY PROTECTION ACROSS BEARER TYPES\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  IntegrityContext int_ctx;
  const char *key_hex = "A1B2C3D4E5F6071829384756A1B2C3D4";
  for (int i = 0; i < 16; i++) {
    sscanf(key_hex + i * 2, "%2hhx", &int_ctx.integrity_key[i]);
  }
  int_ctx.count = 0x00000001;
  int_ctx.direction = 0;

  // Test SRB
  std::cout << "\n[TEST 1] SRB (Signaling Radio Bearer)\n";
  int_ctx.bearer = 1;
  PDCP_PDU_Integrity srb_pdu;
  std::string srb_msg = "RRC Connection Setup";
  stage1_prepare_pdu_for_integrity(srb_msg, int_ctx, srb_pdu, SRB);
  std::cout
      << "  → Result: Integrity protection APPLIED (mandatory for SRBs)\n";

  // Test DRB with integrity enabled
  std::cout << "\n[TEST 2] DRB with Integrity Enabled\n";
  int_ctx.bearer = 5;
  PDCP_PDU_Integrity drb_pdu;
  std::string drb_msg = "User data packet";
  stage1_prepare_pdu_for_integrity(drb_msg, int_ctx, drb_pdu, DRB);
  std::cout
      << "  → Result: Integrity protection APPLIED (configured for this DRB)\n";

  // Test MRB
  std::cout << "\n[TEST 3] MRB (MBS Radio Bearer)\n";
  int_ctx.bearer = 10;
  PDCP_PDU_Integrity mrb_pdu;
  std::string mrb_msg = "Multicast broadcast data";
  stage1_prepare_pdu_for_integrity(mrb_msg, int_ctx, mrb_pdu, MRB);
  std::cout << "  → Result: Integrity protection NOT APPLIED (per 3GPP spec)\n";

  // Cleanup
  if (srb_pdu.data)
    delete[] srb_pdu.data;
  if (drb_pdu.data)
    delete[] drb_pdu.data;
  if (mrb_pdu.data)
    delete[] mrb_pdu.data;

  std::cout << "\n" << std::string(80, '-') << "\n";
  std::cout << "SCENARIO 3 COMPLETE: Bearer type handling verified\n";
  std::cout << std::string(80, '-') << "\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 10: MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════"
         "═══════════╗\n";
  std::cout
      << "║                                                                "
         "           ║\n";
  std::cout
      << "║  5G NR PDCP INTEGRITY PROTECTION - COMPLETE IMPLEMENTATION     "
         "           ║\n";
  std::cout << "║  (Stages 1-5: Setup → Initialization → MAC-I → Attachment → "
               "Verify)   ║\n";
  std::cout
      << "║                                                                "
         "           ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════"
         "═══════════╝\n";

  std::cout << "\nStandard Compliance:\n";
  std::cout << "  • 3GPP TS 38.323 v19.0.0 - PDCP Specification\n";
  std::cout << "  • 3GPP TS 33.501 - Security Architecture for 5G System\n";
  std::cout << "  • 3GPP TS 35.223 - 128-NIA2 Algorithm (AES-CMAC based)\n";
  std::cout << "  • RFC 4493 - AES-CMAC Algorithm\n";

  std::cout << "\n" << std::string(80, '=') << "\n";
  std::cout << "EDUCATIONAL NOTES:\n";
  std::cout << std::string(80, '=') << "\n";
  std::cout
      << "1. This implementation uses SIMPLIFIED AES-CMAC for educational "
         "purposes\n";
  std::cout << "2. Production systems MUST use certified cryptographic "
               "libraries:\n";
  std::cout << "   • OpenSSL: CMAC_Init(), CMAC_Update(), CMAC_Final()\n";
  std::cout << "   • mbedTLS: mbedtls_cipher_cmac_starts/update/finish()\n";
  std::cout << "   • Hardware: Intel AES-NI or ARM Crypto Extensions\n";
  std::cout << "3. Integrity protection is SEPARATE from ciphering\n";
  std::cout << "4. Integrity is performed BEFORE ciphering (plaintext MAC-I)\n";
  std::cout
      << "5. MAC-I is then ciphered along with data for confidentiality\n";
  std::cout << std::string(80, '=') << "\n";

  // Run demonstration scenarios
  demonstrate_integrity_protection();
  demonstrate_tampered_pdu();
  demonstrate_bearer_types();

  // Final summary
  std::cout << "\n\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════"
         "═══════════╗\n";
  std::cout
      << "║  DEMONSTRATION COMPLETE                                        "
         "           ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════"
         "═══════════╝\n";

  std::cout << "\nKey Takeaways:\n";
  std::cout << "  ✓ Integrity protection ensures message authenticity and "
               "integrity\n";
  std::cout << "  ✓ MAC-I computed over header + data BEFORE ciphering\n";
  std::cout << "  ✓ Verification failure causes PDU discard and upper layer "
               "notification\n";
  std::cout << "  ✓ SRBs always have integrity protection (mandatory)\n";
  std::cout << "  ✓ DRBs have optional integrity (configurable)\n";
  std::cout << "  ✓ MRBs do not use integrity protection\n";
  std::cout << "  ✓ COUNT value prevents replay attacks\n";
  std::cout << "  ✓ BEARER and DIRECTION provide context binding\n";

  std::cout << "\n⚠ SECURITY WARNING:\n";
  std::cout << "  This code is for EDUCATIONAL purposes only!\n";
  std::cout << "  Never use simplified crypto in production systems!\n";
  std::cout << "  Always use certified, tested cryptographic libraries!\n\n";

  return 0;
}
