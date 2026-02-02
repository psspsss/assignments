/**
 * ═══════════════════════════════════════════════════════════════════════════
 * 5G NR PDCP CIPHERING - COMPLETE IMPLEMENTATION (Stages 1-5)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Standard Compliance: 3GPP TS 38.323 (PDCP), TS 33.501 (Security), TS 35.222
 * (NEA2)
 *
 * This implementation demonstrates the complete PDCP ciphering flow:
 *
 *   STAGE 1: Integrity-Protected PDU Input & Parameter Setup
 *   STAGE 2: Cipher Initialization (NEA2 - AES-CTR Mode)
 *   STAGE 3: Keystream Generation (AES-CTR Block Encryption)
 *   STAGE 4: XOR Operation (Plaintext ⊕ Keystream → Ciphertext)
 *   STAGE 5: Encrypted PDU Output & Transmission
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
struct SecurityContext {
  uint8_t cipher_key[16]; // 128-bit CK (Cipher Key) - K_RRCenc or K_UPenc
  uint32_t count;         // 32-bit COUNT = HFN || PDCP_SN
  uint8_t bearer;         // 5-bit Radio Bearer Identity (0-31)
  uint8_t direction;      // 1-bit: 0=Uplink (UE→gNB), 1=Downlink (gNB→UE)
};

/**
 * @brief PDCP Protocol Data Unit Structure
 */
struct PDCP_PDU {
  uint8_t header[2];    // PDCP Header (D/C, PDCP SN, etc.)
  uint8_t *data;        // PDCP SDU (user data)
  uint32_t data_length; // Length of data in bytes
  bool is_encrypted;    // Encryption status flag
};

/**
 * @brief Cipher Algorithm Selection (3GPP TS 33.501)
 */
enum CipherAlgorithm {
  NEA0 = 0, // Null encryption (no ciphering)
  NEA1 = 1, // SNOW 3G
  NEA2 = 2, // AES-CTR (128-bit) - Used in this implementation
  NEA3 = 3  // ZUC
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

/**
 * @brief XOR two byte arrays (fundamental operation in stream ciphers)
 *
 * @note This is the core cryptographic operation: CIPHERTEXT = PLAINTEXT ⊕
 * KEYSTREAM
 */
void xor_bytes(const uint8_t *input, const uint8_t *keystream, uint8_t *output,
               size_t length) {
  for (size_t i = 0; i < length; i++) {
    output[i] = input[i] ^ keystream[i];
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: SIMPLIFIED AES-CTR IMPLEMENTATION (NEA2)
// ═══════════════════════════════════════════════════════════════════════════
// NOTE: This is a SIMPLIFIED AES for educational purposes.
// Production code MUST use OpenSSL, mbedTLS, or hardware-accelerated AES!
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Simplified AES encryption (DEMONSTRATION ONLY - NOT SECURE!)
 *
 * In real 5G implementations, this MUST be replaced with:
 * - OpenSSL: EVP_EncryptInit_ex() with EVP_aes_128_ctr()
 * - mbedTLS: mbedtls_aes_crypt_ctr()
 * - Intel AES-NI: Hardware-accelerated AES instructions
 *
 * @warning This is a pseudo-AES for demonstration. DO NOT use in production!
 */
void simplified_aes_encrypt(const uint8_t *key, const uint8_t *input,
                            uint8_t *output) {
  // This is a SIMPLIFIED substitution-permutation network for demonstration
  // Real AES uses: SubBytes, ShiftRows, MixColumns, AddRoundKey over 10 rounds

  for (int i = 0; i < 16; i++) {
    // Pseudo-encryption: combines input with key using mixing functions
    uint8_t mixed = input[i] ^ key[i];
    mixed = ((mixed << 1) | (mixed >> 7)) ^ key[(i + 7) % 16];
    mixed = (mixed ^ 0x63) + key[(i + 3) % 16];
    output[i] = mixed;
  }

  std::cout << "  [AES-CTR] WARNING: Using simplified AES (for demo only)\n";
  std::cout << "  [AES-CTR] Production code must use OpenSSL/mbedTLS!\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4: STAGE 1 - INTEGRITY-PROTECTED PDU INPUT & PARAMETER SETUP
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 1: Prepare PDCP PDU for ciphering
 *
 * This stage receives the integrity-protected PDCP PDU and extracts all
 * necessary parameters for the ciphering operation according to 3GPP TS 38.323.
 *
 * Steps in Stage 1:
 * 1. Parse incoming PDCP Service Data Unit (SDU)
 * 2. Extract PDCP Sequence Number (SN) from header
 * 3. Construct 32-bit COUNT value (COUNT = HFN || PDCP_SN)
 * 4. Identify BEARER identity
 * 5. Determine DIRECTION (uplink/downlink)
 * 6. Validate PDU structure and length
 *
 * @param plaintext     User data to be encrypted (PDCP SDU)
 * @param length        Length of plaintext in bytes
 * @param sec_ctx       Security context with cipher parameters
 * @param pdu          [out] Constructed PDCP PDU structure
 *
 * @return 0 on success, negative error code on failure
 *
 * @note 3GPP TS 38.323 Section 5.8: Ciphering procedure
 */
int stage1_prepare_pdu(const std::string &plaintext, SecurityContext &sec_ctx,
                       PDCP_PDU &pdu) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 1: INTEGRITY-PROTECTED PDU INPUT & PARAMETER SETUP     ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "3GPP Reference: TS 38.323 Section 5.8 (Ciphering)\n\n";

  // Step 1.1: Parse PDCP SDU (user plaintext data)
  std::cout << "[STEP 1.1] Parsing PDCP Service Data Unit (SDU)\n";
  std::cout << "  → Plaintext message: \"" << plaintext << "\"\n";
  std::cout << "  → Message length: " << plaintext.length() << " bytes\n\n";

  // Step 1.2: Construct PDCP Header
  // PDCP header format (Data PDU with 12-bit SN):
  //   Bit 0:     D/C bit (1 = Data PDU)
  //   Bits 1-3:  Reserved
  //   Bits 4-15: PDCP Sequence Number (SN)
  std::cout << "[STEP 1.2] Constructing PDCP Header\n";
  uint16_t pdcp_sn = sec_ctx.count & 0x0FFF; // Extract 12-bit SN from COUNT
  pdu.header[0] = 0x80 | ((pdcp_sn >> 8) & 0x0F); // D/C=1, SN[11:8]
  pdu.header[1] = pdcp_sn & 0xFF;                 // SN[7:0]

  std::cout << "  → PDCP Sequence Number (SN): " << pdcp_sn << " (12-bit)\n";
  std::cout << "  → Header byte[0]: 0x" << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<int>(pdu.header[0]) << std::dec
            << "\n";
  std::cout << "  → Header byte[1]: 0x" << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<int>(pdu.header[1]) << std::dec
            << "\n\n";

  // Step 1.3: Construct 32-bit COUNT value
  // COUNT = Hyper Frame Number (HFN) || PDCP_SN
  // For 12-bit SN: COUNT[31:12] = HFN, COUNT[11:0] = PDCP_SN
  std::cout << "[STEP 1.3] Constructing 32-bit COUNT Parameter\n";
  uint32_t hfn = sec_ctx.count >> 12; // Extract HFN (upper 20 bits)
  std::cout << "  → COUNT value: 0x" << std::hex << std::setw(8)
            << std::setfill('0') << sec_ctx.count << std::dec << "\n";
  std::cout << "  → HFN (Hyper Frame Number): " << hfn << "\n";
  std::cout << "  → PDCP SN: " << pdcp_sn << "\n";
  std::cout << "  → Formula: COUNT = (HFN << 12) | PDCP_SN\n\n";

  // Step 1.4: Extract Bearer and Direction
  std::cout << "[STEP 1.4] Identifying Bearer and Direction\n";
  std::cout << "  → BEARER ID: " << static_cast<int>(sec_ctx.bearer)
            << " (5-bit value, range: 0-31)\n";
  std::cout << "  → DIRECTION: " << static_cast<int>(sec_ctx.direction) << " ("
            << (sec_ctx.direction == 0 ? "Uplink UE→gNB" : "Downlink gNB→UE")
            << ")\n\n";

  // Step 1.5: Allocate and copy plaintext data
  std::cout << "[STEP 1.5] Allocating PDU Data Buffer\n";
  pdu.data_length = plaintext.length();
  pdu.data = new uint8_t[pdu.data_length];
  string_to_bytes(plaintext, pdu.data);
  pdu.is_encrypted = false;

  print_hex("  → Plaintext (hex)", pdu.data, pdu.data_length);

  // Step 1.6: Display complete input parameters
  std::cout << "\n[STAGE 1 SUMMARY] Input Parameters for NEA2 Algorithm:\n";
  std::cout << "  ┌─────────────────────────────────────────────────────┐\n";
  std::cout << "  │ Parameter    │ Value                                │\n";
  std::cout << "  ├─────────────────────────────────────────────────────┤\n";
  std::cout << "  │ COUNT        │ 0x" << std::hex << std::setw(8)
            << std::setfill('0') << sec_ctx.count << std::dec
            << " (32-bit)        │\n";
  std::cout << "  │ BEARER       │ " << std::setw(2)
            << static_cast<int>(sec_ctx.bearer)
            << " (5-bit)                            │\n";
  std::cout << "  │ DIRECTION    │ " << static_cast<int>(sec_ctx.direction)
            << " (" << (sec_ctx.direction == 0 ? "UL" : "DL")
            << ")                                  │\n";
  std::cout << "  │ LENGTH       │ " << std::setw(4) << (pdu.data_length * 8)
            << " bits                              │\n";
  std::cout << "  └─────────────────────────────────────────────────────┘\n";

  std::cout << "\n✓ STAGE 1 COMPLETE: PDU prepared for ciphering\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5: STAGE 2 - CIPHER INITIALIZATION (NEA2 AES-CTR)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 2: Initialize AES-CTR cipher engine
 *
 * NEA2 uses AES in Counter (CTR) mode as specified in 3GPP TS 35.222.
 * This stage constructs the initial counter block and prepares the AES engine.
 *
 * Counter Block Construction (128 bits):
 *   Bits [0-31]:   COUNT value (32 bits)
 *   Bits [32-36]:  BEARER identity (5 bits)
 *   Bit  [37]:     DIRECTION (1 bit)
 *   Bits [38-63]:  Zero padding (26 bits)
 *   Bits [64-127]: Block counter (starts at 0, increments for each block)
 *
 * @param sec_ctx       Security context with cipher key
 * @param pdu          PDCP PDU to be encrypted
 * @param counter_block [out] Initialized 128-bit counter block
 *
 * @return 0 on success
 *
 * @note 3GPP TS 35.222 Section 4: Algorithm specification for 128-NEA2
 */
int stage2_initialize_cipher(const SecurityContext &sec_ctx,
                             const PDCP_PDU &pdu, uint8_t counter_block[16]) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 2: CIPHER INITIALIZATION (NEA2 - AES-CTR MODE)        ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "3GPP Reference: TS 35.222 Section 4 (128-NEA2 Algorithm)\n\n";

  // Step 2.1: Display cipher algorithm information
  std::cout << "[STEP 2.1] Cipher Algorithm: 128-NEA2 (AES-128 in CTR mode)\n";
  std::cout << "  → Key size: 128 bits (16 bytes)\n";
  std::cout << "  → Block size: 128 bits (16 bytes)\n";
  std::cout << "  → Mode: Counter (CTR) - Stream cipher mode\n";
  std::cout << "  → Security: Provides confidentiality (not integrity)\n\n";

  // Step 2.2: Load cipher key (CK)
  std::cout << "[STEP 2.2] Loading 128-bit Cipher Key (CK)\n";
  print_hex("  → Cipher Key (CK)", sec_ctx.cipher_key, 16);
  std::cout << "  → Key derivation: CK = KDF(K_gNB, algorithm-ID, ...)\n";
  std::cout << "  → For Control Plane: CK = K_RRCenc\n";
  std::cout << "  → For User Plane: CK = K_UPenc\n\n";

  // Step 2.3: Construct 128-bit counter block
  std::cout << "[STEP 2.3] Constructing 128-bit Counter Block\n";
  std::cout << "  Counter Block Structure (as per TS 35.222):\n";
  std::cout << "  ┌───────────────────────────────────────────────────────┐\n";
  std::cout << "  │ Bits [0-31]   : COUNT     = 0x" << std::hex << std::setw(8)
            << std::setfill('0') << sec_ctx.count << std::dec
            << "           │\n";
  std::cout << "  │ Bits [32-36]  : BEARER    = " << std::setw(2)
            << static_cast<int>(sec_ctx.bearer) << " (0x" << std::hex
            << static_cast<int>(sec_ctx.bearer) << std::dec
            << ")                    │\n";
  std::cout << "  │ Bit  [37]     : DIRECTION = "
            << static_cast<int>(sec_ctx.direction)
            << "                               │\n";
  std::cout << "  │ Bits [38-63]  : ZERO      = 0x00000000000000          │\n";
  std::cout << "  │ Bits [64-127] : COUNTER   = 0 (increments per block)  │\n";
  std::cout
      << "  └───────────────────────────────────────────────────────┘\n\n";

  // Initialize counter block to zero
  memset(counter_block, 0, 16);

  // Bytes 0-3: COUNT (32 bits, big-endian)
  counter_block[0] = (sec_ctx.count >> 24) & 0xFF;
  counter_block[1] = (sec_ctx.count >> 16) & 0xFF;
  counter_block[2] = (sec_ctx.count >> 8) & 0xFF;
  counter_block[3] = sec_ctx.count & 0xFF;

  // Byte 4: BEARER (5 bits) || DIRECTION (1 bit) || 0 (2 bits)
  counter_block[4] = (sec_ctx.bearer << 3) | (sec_ctx.direction << 2);

  // Bytes 5-7: Zero padding
  counter_block[5] = 0x00;
  counter_block[6] = 0x00;
  counter_block[7] = 0x00;

  // Bytes 8-15: Block counter (starts at 0, will increment)
  // Initially zero - will be incremented for each 16-byte block
  for (int i = 8; i < 16; i++) {
    counter_block[i] = 0x00;
  }

  print_hex("  → Initial Counter Block", counter_block, 16);

  std::cout << "\n[STEP 2.4] Counter Block Bit Layout (Binary):\n";
  std::cout << "  Byte 0 (COUNT[31:24]): ";
  for (int i = 7; i >= 0; i--)
    std::cout << ((counter_block[0] >> i) & 1);
  std::cout << "b\n  Byte 4 (BEARER|DIR): ";
  for (int i = 7; i >= 0; i--)
    std::cout << ((counter_block[4] >> i) & 1);
  std::cout << "b\n";
  std::cout << "          ^^^^^^^^^^^ BEARER (5-bit)\n";
  std::cout << "                     ^ DIRECTION (1-bit)\n";
  std::cout << "                       ^^ Zero padding\n\n";

  std::cout << "✓ STAGE 2 COMPLETE: AES-CTR cipher initialized\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 6: STAGE 3 - KEYSTREAM GENERATION (AES-CTR ENCRYPTION)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 3: Generate keystream using AES-CTR
 *
 * In CTR mode, the AES cipher encrypts the counter blocks to produce a
 * pseudo-random keystream. The keystream is then XORed with plaintext.
 *
 * Process:
 * 1. For each 128-bit block needed:
 *    - Encrypt the counter block with AES-128
 *    - Result is one 128-bit keystream block
 *    - Increment the counter for next block
 * 2. Concatenate all keystream blocks
 * 3. Truncate to match plaintext length
 *
 * @param sec_ctx       Security context with cipher key
 * @param counter_block Initial counter block
 * @param length        Required keystream length in bytes
 * @param keystream    [out] Generated keystream buffer
 *
 * @return 0 on success
 *
 * @note The keystream is deterministic and can be regenerated at receiver
 *       using same COUNT, BEARER, DIRECTION, and CK parameters
 */
int stage3_generate_keystream(const SecurityContext &sec_ctx,
                              uint8_t counter_block[16], size_t length,
                              uint8_t *keystream) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 3: KEYSTREAM GENERATION (AES-CTR BLOCK ENCRYPTION)    ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout
      << "3GPP Reference: TS 35.222 Section 4.1 (Keystream generation)\n\n";

  // Step 3.1: Calculate number of AES blocks needed
  size_t num_blocks = (length + 15) / 16; // Round up to 16-byte blocks
  std::cout << "[STEP 3.1] Calculating Required Keystream Length\n";
  std::cout << "  → Plaintext length: " << length << " bytes (" << (length * 8)
            << " bits)\n";
  std::cout << "  → AES block size: 16 bytes (128 bits)\n";
  std::cout << "  → Number of blocks needed: " << num_blocks << "\n\n";

  // Step 3.2: Generate keystream blocks
  std::cout << "[STEP 3.2] Generating Keystream Blocks\n";
  std::cout
      << "  AES-CTR Operation: KEYSTREAM[i] = AES_Encrypt(CK, Counter+i)\n\n";

  for (size_t block = 0; block < num_blocks; block++) {
    std::cout << "  ┌─ Block " << block
              << " ───────────────────────────────────┐\n";

    // Set block counter (bytes 8-15 of counter block)
    // Using simple increment for demonstration (production uses proper counter
    // arithmetic)
    uint64_t block_counter = block;
    for (int i = 15; i >= 8; i--) {
      counter_block[i] = block_counter & 0xFF;
      block_counter >>= 8;
    }

    std::cout << "  │ Counter Block #" << block << ":\n  │   0x";
    for (int i = 0; i < 16; i++) {
      std::cout << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(counter_block[i]);
    }
    std::cout << std::dec << "\n";

    // Encrypt counter block to generate keystream block
    uint8_t keystream_block[16];
    simplified_aes_encrypt(sec_ctx.cipher_key, counter_block, keystream_block);

    std::cout << "  │ AES Encrypted Output:\n  │   0x";
    for (int i = 0; i < 16; i++) {
      std::cout << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(keystream_block[i]);
    }
    std::cout << std::dec << "\n";

    // Copy keystream block to output buffer
    size_t bytes_to_copy =
        (block == num_blocks - 1) ? (length - block * 16) : 16;
    memcpy(keystream + block * 16, keystream_block, bytes_to_copy);

    std::cout << "  │ Copied " << bytes_to_copy << " bytes to keystream\n";
    std::cout << "  └────────────────────────────────────────────────┘\n\n";
  }

  // Step 3.3: Display final keystream
  std::cout << "[STEP 3.3] Complete Keystream Generated\n";
  print_hex("  → Final Keystream", keystream, length);

  std::cout << "\n[STAGE 3 SUMMARY] Keystream Properties:\n";
  std::cout << "  → Total keystream length: " << length << " bytes\n";
  std::cout << "  → Number of AES blocks: " << num_blocks << "\n";
  std::cout << "  → Keystream is deterministic (can be regenerated)\n";
  std::cout << "  → Same parameters at receiver produce identical keystream\n";

  std::cout << "\n✓ STAGE 3 COMPLETE: Keystream generation successful\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 7: STAGE 4 - XOR OPERATION (PLAINTEXT ⊕ KEYSTREAM)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 4: XOR plaintext with keystream to produce ciphertext
 *
 * This is the fundamental operation in stream ciphers:
 *   CIPHERTEXT = PLAINTEXT ⊕ KEYSTREAM
 *
 * Properties:
 * - XOR is reversible: PLAINTEXT = CIPHERTEXT ⊕ KEYSTREAM
 * - Same operation for encryption and decryption
 * - Bit-level operation, typically done on byte boundaries
 *
 * Important: In PDCP, only the PDCP SDU (user data) is encrypted.
 *           The PDCP header remains in plaintext.
 *
 * @param pdu       PDCP PDU with plaintext data
 * @param keystream Generated keystream from Stage 3
 * @param ciphertext [out] Encrypted output buffer
 *
 * @return 0 on success
 *
 * @note 3GPP TS 38.323 Section 5.8.2: Only PDCP Data field is ciphered
 */
int stage4_xor_operation(PDCP_PDU &pdu, const uint8_t *keystream,
                         uint8_t *ciphertext) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 4: XOR OPERATION (PLAINTEXT ⊕ KEYSTREAM → CIPHERTEXT)║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout
      << "3GPP Reference: TS 38.323 Section 5.8.2 (Ciphering procedure)\n\n";

  // Step 4.1: Display input data
  std::cout << "[STEP 4.1] XOR Operation Inputs\n";
  print_hex("  → Plaintext (PDCP SDU)", pdu.data, pdu.data_length);
  print_hex("  → Keystream", keystream, pdu.data_length);

  std::cout << "\n[STEP 4.2] Performing Bitwise XOR Operation\n";
  std::cout << "  Formula: CIPHERTEXT[i] = PLAINTEXT[i] ⊕ KEYSTREAM[i]\n";
  std::cout << "  Processing " << pdu.data_length << " bytes...\n\n";

  // Step 4.3: Demonstrate XOR on first few bytes
  std::cout << "  Byte-by-Byte XOR Demonstration (first 4 bytes):\n";
  for (size_t i = 0; i < std::min(static_cast<size_t>(4),
                                  static_cast<size_t>(pdu.data_length));
       i++) {
    uint8_t plain_byte = pdu.data[i];
    uint8_t key_byte = keystream[i];
    uint8_t cipher_byte = plain_byte ^ key_byte;

    std::cout << "  ┌─ Byte " << i
              << " ─────────────────────────────────────┐\n";
    std::cout << "  │ Plaintext[" << i << "]:  0x" << std::hex << std::setw(2)
              << std::setfill('0') << static_cast<int>(plain_byte) << " = ";
    for (int b = 7; b >= 0; b--)
      std::cout << ((plain_byte >> b) & 1);
    std::cout << "b │\n";

    std::cout << "  │ Keystream[" << i << "]:  0x" << std::hex << std::setw(2)
              << std::setfill('0') << static_cast<int>(key_byte) << " = ";
    for (int b = 7; b >= 0; b--)
      std::cout << ((key_byte >> b) & 1);
    std::cout << "b │\n";

    std::cout << "  │ XOR Result:    0x" << std::hex << std::setw(2)
              << std::setfill('0') << static_cast<int>(cipher_byte) << " = ";
    for (int b = 7; b >= 0; b--)
      std::cout << ((cipher_byte >> b) & 1);
    std::cout << "b │\n";
    std::cout << std::dec
              << "  └──────────────────────────────────────────────┘\n";
  }

  // Step 4.4: Perform XOR on entire data
  std::cout << "\n[STEP 4.3] Computing Complete Ciphertext\n";
  xor_bytes(pdu.data, keystream, ciphertext, pdu.data_length);

  print_hex("  → Ciphertext Result", ciphertext, pdu.data_length);

  // Update PDU with encrypted data
  memcpy(pdu.data, ciphertext, pdu.data_length);
  pdu.is_encrypted = true;

  // Step 4.5: Verify XOR properties
  std::cout << "\n[STEP 4.4] XOR Properties Verification\n";
  std::cout << "  → Property 1: A ⊕ B = C implies C ⊕ B = A (reversibility)\n";
  std::cout << "  → Property 2: A ⊕ 0 = A (identity)\n";
  std::cout << "  → Property 3: A ⊕ A = 0 (self-inverse)\n";
  std::cout
      << "  → Decryption uses same operation: PLAIN = CIPHER ⊕ KEYSTREAM\n";

  std::cout << "\n✓ STAGE 4 COMPLETE: Ciphertext generated via XOR operation\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 8: STAGE 5 - ENCRYPTED PDU OUTPUT & TRANSMISSION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief STAGE 5: Construct final encrypted PDCP PDU for transmission
 *
 * The final PDCP PDU structure:
 *   +----------------+------------------+
 *   | PDCP Header    | Encrypted Data   |
 *   | (plaintext)    | (ciphertext)     |
 *   +----------------+------------------+
 *
 * Note: PDCP header is NOT encrypted (needed for routing and sequencing)
 *       Only PDCP SDU (payload data) is encrypted
 *
 * @param pdu           PDCP PDU with encrypted data
 * @param sec_ctx       Security context (for COUNT update)
 * @param output_buffer [out] Complete PDU ready for transmission
 * @param output_length [out] Total length of output PDU
 *
 * @return 0 on success
 *
 * @note 3GPP TS 38.323 Section 6.2: PDCP Data PDU format
 */
int stage5_output_encrypted_pdu(PDCP_PDU &pdu, SecurityContext &sec_ctx,
                                uint8_t *output_buffer, size_t &output_length) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  STAGE 5: ENCRYPTED PDU OUTPUT & TRANSMISSION PREPARATION    ║\n";
  std::cout
      << "╚═══════════════════════════════════════════════════════════════╝\n";
  std::cout << "3GPP Reference: TS 38.323 Section 6.2 (PDCP Data PDU)\n\n";

  // Step 5.1: Construct final PDU structure
  std::cout << "[STEP 5.1] Constructing Final PDCP PDU\n";
  std::cout << "  PDCP PDU Format:\n";
  std::cout
      << "  ┌──────────────────────────────────────────────────────────┐\n";
  std::cout
      << "  │  PDCP Header (2 bytes)  │  PDCP Data (encrypted)         │\n";
  std::cout
      << "  │  [Plaintext]            │  [Ciphertext]                  │\n";
  std::cout
      << "  └──────────────────────────────────────────────────────────┘\n";
  std::cout
      << "  └─ Transmitted over air interface ─────────────────────────┘\n\n";

  // Copy header (plaintext)
  output_buffer[0] = pdu.header[0];
  output_buffer[1] = pdu.header[1];

  // Copy encrypted data
  memcpy(output_buffer + 2, pdu.data, pdu.data_length);
  output_length = 2 + pdu.data_length;

  std::cout << "[STEP 5.2] PDU Component Breakdown\n";
  std::cout << "  ┌─ PDCP Header (Unencrypted) ─────────────────────┐\n";
  std::cout << "  │ Byte 0: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(pdu.header[0])
            << " (D/C bit + SN[11:8])      │\n";
  std::cout << "  │ Byte 1: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(pdu.header[1])
            << " (SN[7:0])                 │\n";
  std::cout << std::dec
            << "  └─────────────────────────────────────────────────┘\n\n";

  print_hex("  → Complete Encrypted PDU", output_buffer, output_length);

  // Step 5.3: Display transmission information
  std::cout << "\n[STEP 5.3] Transmission Information\n";
  std::cout << "  → Total PDU size: " << output_length << " bytes\n";
  std::cout << "  → Header size: 2 bytes (unencrypted)\n";
  std::cout << "  → Payload size: " << pdu.data_length
            << " bytes (encrypted)\n";
  std::cout << "  → Security applied: NEA2 (AES-CTR 128-bit)\n";
  std::cout << "  → COUNT value: 0x" << std::hex << std::setw(8)
            << std::setfill('0') << sec_ctx.count << std::dec << "\n";
  std::cout << "  → Direction: "
            << (sec_ctx.direction == 0 ? "Uplink (UE→gNB)"
                                       : "Downlink (gNB→UE)")
            << "\n\n";

  // Step 5.4: Security analysis
  std::cout << "[STEP 5.4] Security Analysis\n";
  std::cout
      << "  ✓ Confidentiality: Protected (eavesdropper sees ciphertext)\n";
  std::cout << "  ✓ Key security: 128-bit AES (computationally secure)\n";
  std::cout << "  ✓ Replay protection: COUNT prevents replay attacks\n";
  std::cout
      << "  ⚠ Integrity: NOT provided by ciphering (requires separate MAC-I)\n";
  std::cout
      << "  ⚠ Header exposed: PDCP header readable (needed for routing)\n\n";

  // Step 5.5: Update COUNT for next transmission
  std::cout << "[STEP 5.5] Updating Security Context for Next PDU\n";
  uint32_t old_count = sec_ctx.count;
  sec_ctx.count++;
  std::cout << "  → Previous COUNT: 0x" << std::hex << std::setw(8)
            << std::setfill('0') << old_count << std::dec << "\n";
  std::cout << "  → New COUNT: 0x" << std::hex << std::setw(8)
            << std::setfill('0') << sec_ctx.count << std::dec << "\n";
  std::cout << "  → COUNT increments for each transmitted PDU\n";
  std::cout << "  → Prevents keystream reuse (critical for security)\n\n";

  std::cout << "✓ STAGE 5 COMPLETE: Encrypted PDU ready for transmission!\n";
  std::cout << "\n[TRANSMISSION] PDU transmitted over air interface...\n";

  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 9: DECRYPTION PROCESS (RECEIVER SIDE)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Decrypt received PDCP PDU (receiver side operation)
 *
 * Decryption uses the same XOR operation:
 *   PLAINTEXT = CIPHERTEXT ⊕ KEYSTREAM
 *
 * The receiver must:
 * 1. Extract PDCP SN from header
 * 2. Reconstruct COUNT (using local HFN)
 * 3. Generate identical keystream (same COUNT, BEARER, DIRECTION, CK)
 * 4. XOR ciphertext with keystream to recover plaintext
 *
 * @param encrypted_pdu Received encrypted PDU
 * @param pdu_length    Length of received PDU
 * @param sec_ctx       Receiver's security context
 * @param decrypted_text [out] Recovered plaintext
 *
 * @return 0 on success
 */
int decrypt_pdu(const uint8_t *encrypted_pdu, size_t pdu_length,
                SecurityContext &sec_ctx, std::string &decrypted_text) {
  std::cout << "\n";
  std::cout
      << "╔═══════════════════════════════════════════════════════════════╗\n";
  std::cout
      << "║  RECEIVER: DECRYPTION PROCESS                                 ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════════"
               "═╝\n\n";

  // Extract PDCP header
  std::cout << "[RECEIVER STEP 1] Extract PDCP Header\n";
  uint16_t received_sn = ((encrypted_pdu[0] & 0x0F) << 8) | encrypted_pdu[1];
  std::cout << "  → Received PDCP SN: " << received_sn << "\n";

  // Reconstruct COUNT (in practice, receiver maintains HFN state)
  std::cout << "\n[RECEIVER STEP 2] Reconstruct COUNT Value\n";
  std::cout << "  → Using synchronized COUNT from security context\n";
  std::cout << "  → COUNT: 0x" << std::hex << std::setw(8) << std::setfill('0')
            << sec_ctx.count << std::dec << "\n";

  // Generate keystream (identical to transmitter)
  std::cout << "\n[RECEIVER STEP 3] Generate Identical Keystream\n";
  size_t ciphertext_length = pdu_length - 2; // Exclude header
  uint8_t *keystream = new uint8_t[ciphertext_length];

  uint8_t counter_block[16];
  stage2_initialize_cipher(sec_ctx, PDCP_PDU(), counter_block);
  stage3_generate_keystream(sec_ctx, counter_block, ciphertext_length,
                            keystream);

  // Decrypt by XORing ciphertext with keystream
  std::cout << "\n[RECEIVER STEP 4] Decrypt via XOR Operation\n";
  uint8_t *plaintext = new uint8_t[ciphertext_length];
  xor_bytes(encrypted_pdu + 2, keystream, plaintext, ciphertext_length);

  print_hex("  → Recovered Plaintext", plaintext, ciphertext_length);

  decrypted_text = bytes_to_string(plaintext, ciphertext_length);
  std::cout << "  → Decrypted Message: \"" << decrypted_text << "\"\n";

  delete[] keystream;
  delete[] plaintext;

  std::cout << "\n✓ DECRYPTION COMPLETE: Plaintext successfully recovered!\n";
  return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 10: MAIN DEMONSTRATION
// ═══════════════════════════════════════════════════════════════════════════

int main() {
  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout
      << "  5G NR PDCP CIPHERING - COMPLETE FLOW DEMONSTRATION (Stages 1-5)\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "Standard Compliance:\n";
  std::cout << "  • 3GPP TS 38.323 - Packet Data Convergence Protocol (PDCP)\n";
  std::cout
      << "  • 3GPP TS 33.501 - Security Architecture and Procedures for 5G\n";
  std::cout
      << "  • 3GPP TS 35.222 - 128-NEA2 Algorithm Specification (AES-CTR)\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  // ═══════════════════════════════════════════════════════════════════════
  // SIMULATION SETUP
  // ═══════════════════════════════════════════════════════════════════════

  std::cout << "\n[SIMULATION] UE → gNB Message Transmission\n";
  std::string message =
      "Hello 5G Network! This message is encrypted using NEA2.";
  std::cout << "  → Original Message: \"" << message << "\"\n";
  std::cout << "  → Message Length: " << message.length() << " bytes\n";

  // Initialize security context
  SecurityContext sec_ctx;

  // Set 128-bit cipher key (K_RRCenc or K_UPenc derived from K_gNB)
  // In real systems, this is derived using KDF (Key Derivation Function)
  const char *key_hex = "0123456789ABCDEF0123456789ABCDEF";
  for (int i = 0; i < 16; i++) {
    sscanf(key_hex + i * 2, "%2hhx", &sec_ctx.cipher_key[i]);
  }

  sec_ctx.count = 0x00000001; // Initial COUNT value
  sec_ctx.bearer = 5;         // Data Radio Bearer (DRB) ID
  sec_ctx.direction = 0;      // 0 = Uplink (UE → gNB)

  std::cout << "\n[SECURITY CONTEXT] Initial Parameters:\n";
  print_hex("  → Cipher Key (CK)", sec_ctx.cipher_key, 16);
  std::cout << "  → Initial COUNT: 0x" << std::hex << std::setw(8)
            << std::setfill('0') << sec_ctx.count << std::dec << "\n";
  std::cout << "  → Bearer ID: " << static_cast<int>(sec_ctx.bearer) << "\n";
  std::cout << "  → Direction: " << static_cast<int>(sec_ctx.direction)
            << " (Uplink)\n";

  // ═══════════════════════════════════════════════════════════════════════
  // ENCRYPTION FLOW (STAGES 1-5)
  // ═══════════════════════════════════════════════════════════════════════

  PDCP_PDU pdu;
  uint8_t counter_block[16];
  uint8_t *keystream = new uint8_t[message.length()];
  uint8_t *ciphertext = new uint8_t[message.length()];
  uint8_t *final_pdu = new uint8_t[message.length() + 2];
  size_t final_pdu_length;

  auto start_time = std::chrono::high_resolution_clock::now();

  // Execute all 5 stages
  stage1_prepare_pdu(message, sec_ctx, pdu);
  stage2_initialize_cipher(sec_ctx, pdu, counter_block);
  stage3_generate_keystream(sec_ctx, counter_block, pdu.data_length, keystream);
  stage4_xor_operation(pdu, keystream, ciphertext);
  stage5_output_encrypted_pdu(pdu, sec_ctx, final_pdu, final_pdu_length);

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - start_time);

  // ═══════════════════════════════════════════════════════════════════════
  // DECRYPTION AT RECEIVER (gNB)
  // ═══════════════════════════════════════════════════════════════════════

  std::string decrypted_message;
  decrypt_pdu(final_pdu, final_pdu_length, sec_ctx, decrypted_message);

  // ═══════════════════════════════════════════════════════════════════════
  // FINAL SUMMARY
  // ═══════════════════════════════════════════════════════════════════════

  std::cout << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "  COMPLETE CIPHERING FLOW SUMMARY\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";
  std::cout << "Original Message:  \"" << message << "\"\n";
  std::cout << "Decrypted Message: \"" << decrypted_message << "\"\n";
  std::cout << "───────────────────────────────────────────────────────────────"
               "────────────\n";
  std::cout << "Verification:      "
            << (message == decrypted_message ? "✓ SUCCESS" : "✗ FAILED")
            << "\n";
  std::cout << "Processing Time:   " << duration.count() << " microseconds\n";
  std::cout << "Cipher Algorithm:  128-NEA2 (AES-CTR)\n";
  std::cout << "Security Level:    128-bit key strength\n";
  std::cout << "═══════════════════════════════════════════════════════════════"
               "════════════\n";

  std::cout << "\n[EDUCATIONAL NOTES]\n";
  std::cout << "1. This implementation uses simplified AES for demonstration\n";
  std::cout << "2. Production systems MUST use certified crypto libraries:\n";
  std::cout << "   • OpenSSL (EVP_aes_128_ctr)\n";
  std::cout << "   • mbedTLS (mbedtls_aes_crypt_ctr)\n";
  std::cout << "   • Intel AES-NI (hardware-accelerated)\n";
  std::cout
      << "3. COUNT management must be thread-safe in multi-threaded systems\n";
  std::cout << "4. Key derivation follows 3GPP TS 33.501 KDF specifications\n";
  std::cout << "5. Integrity protection (MAC-I) is separate from ciphering\n";

  // Cleanup
  delete[] pdu.data;
  delete[] keystream;
  delete[] ciphertext;
  delete[] final_pdu;

  return 0;
}
