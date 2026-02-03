#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>

/*
====================================================
3GPP TS 38.322 - RLC Transparent Mode (TM)
----------------------------------------------------
- No RLC header
- No segmentation
- No reassembly
- SDU == PDU
====================================================
*/

// -----------------------------
// Mock MAC Layer
// -----------------------------
class MAC {
public:
  void receivePDU(const uint8_t *pdu, size_t size) {
    std::cout << "[MAC] Received PDU of size: " << size << " bytes\n";

    receivedSize = size;
    std::memcpy(receivedBuffer, pdu, size);
  }

  uint8_t receivedBuffer[1024]{};
  size_t receivedSize = 0;
};

// -----------------------------
// Mock Upper Layer (PDCP / RRC)
// -----------------------------
class UpperLayer {
public:
  void receiveSDU(const uint8_t *sdu, size_t size) {
    std::cout << "[UpperLayer] Received SDU of size: " << size << " bytes\n";

    receivedSize = size;
    std::memcpy(receivedBuffer, sdu, size);
  }

  uint8_t receivedBuffer[1024]{};
  size_t receivedSize = 0;
};

// -----------------------------
// RLC Transparent Mode Entity
// -----------------------------
class RLC_TM {
public:
  RLC_TM(MAC *mac, UpperLayer *upper) : macLayer(mac), upperLayer(upper) {}

  /*
  ------------------------------------------------
  TRANSMIT PATH (Upper → RLC → MAC)
  ------------------------------------------------
  - No header
  - No modification
  - SDU forwarded as-is
  */
  void RLC_TM_Transmit(const uint8_t *sdu, size_t size) {
    std::cout << "[RLC TM] Transmitting SDU as PDU\n";
    macLayer->receivePDU(sdu, size);
  }

  /*
  ------------------------------------------------
  RECEIVE PATH (MAC → RLC → Upper)
  ------------------------------------------------
  - No decoding
  - No reassembly
  */
  void RLC_TM_Receive(const uint8_t *pdu, size_t size) {
    std::cout << "[RLC TM] Receiving PDU and forwarding as SDU\n";
    upperLayer->receiveSDU(pdu, size);
  }

private:
  MAC *macLayer;
  UpperLayer *upperLayer;
};

// -----------------------------
// TEST CASES
// -----------------------------
int main() {
  MAC mac;
  UpperLayer upper;
  RLC_TM rlc(&mac, &upper);

  // =============================
  // Test Case 1.1: TM Passthrough (Tx)
  // =============================
  std::cout << "\n--- Test Case 1.1: TM Passthrough (Tx) ---\n";

  uint8_t txSDU[100];
  for (int i = 0; i < 100; i++) {
    txSDU[i] = static_cast<uint8_t>(i);
  }

  rlc.RLC_TM_Transmit(txSDU, sizeof(txSDU));

  assert(mac.receivedSize == sizeof(txSDU));
  assert(std::memcmp(txSDU, mac.receivedBuffer, sizeof(txSDU)) == 0);

  std::cout << "[PASS] Tx Passthrough verified\n";

  // =============================
  // Test Case 1.2: TM Passthrough (Rx)
  // =============================
  std::cout << "\n--- Test Case 1.2: TM Passthrough (Rx) ---\n";

  uint8_t rxPDU[50];
  for (int i = 0; i < 50; i++) {
    rxPDU[i] = static_cast<uint8_t>(255 - i);
  }

  rlc.RLC_TM_Receive(rxPDU, sizeof(rxPDU));

  assert(upper.receivedSize == sizeof(rxPDU));
  assert(std::memcmp(rxPDU, upper.receivedBuffer, sizeof(rxPDU)) == 0);

  std::cout << "[PASS] Rx Passthrough verified\n";

  std::cout << "\nAll RLC TM tests passed successfully ✅\n";
  return 0;
}
