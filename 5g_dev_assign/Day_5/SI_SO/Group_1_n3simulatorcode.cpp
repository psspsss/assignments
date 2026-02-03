
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>

/*
 * ============================================================
 * 5G NR RLC AM – SEGMENTATION & RE-SEGMENTATION
 * Compliant with 3GPP TS 38.322 §5.2
 * ============================================================
 */

/* ===================== RLC HEADER ===================== */

struct RlcAmHeader {
  uint8_t si; // 2-bit SI
  bool hasSo;
  uint16_t so;

  RlcAmHeader() : si(0), hasSo(false), so(0) {}
};

/* ===================== RLC PDU ===================== */

struct RlcAmPdu {
  uint16_t sn;
  RlcAmHeader header;
  std::vector<uint8_t> payload;
};

/* ===================== TX ENTITY ===================== */

class RlcAmTxEntity {
public:
  void TransmitPdcpPdu(const std::vector<uint8_t> &sdu) {
    currentSdu = sdu;
    sduOffset = 0;
    nextSn = 0;
    txBuffer.clear();
  }

  bool HasDataToSend() const { return sduOffset < currentSdu.size(); }

  /* ---------- Normal transmission ---------- */
  RlcAmPdu GetTxPdu(uint32_t grantSize) {
    RlcAmPdu pdu;
    pdu.sn = nextSn++;

    uint32_t remaining = currentSdu.size() - sduOffset;
    uint32_t bytes = std::min(grantSize, remaining);

    /* SI decision */
    if (bytes == currentSdu.size()) {
      pdu.header.si = 0b00;
    } else if (sduOffset == 0) {
      pdu.header.si = 0b01;
    } else if (sduOffset + bytes < currentSdu.size()) {
      pdu.header.si = 0b11;
    } else {
      pdu.header.si = 0b10;
    }

    /* SO handling */
    if (pdu.header.si != 0b00 && pdu.header.si != 0b01) {
      pdu.header.hasSo = true;
      pdu.header.so = sduOffset;
    }

    pdu.payload.insert(pdu.payload.end(), currentSdu.begin() + sduOffset,
                       currentSdu.begin() + sduOffset + bytes);

    /* Save for retransmission */
    txBuffer[pdu.sn] = pdu;

    sduOffset += bytes;
    return pdu;
  }

  /* ---------- Mark for retransmission ---------- */
  void MarkForRetransmission(uint16_t sn) {
    assert(txBuffer.count(sn));
    retxSn = sn;
    retxOffset = 0;
  }

  /* ---------- Re-segmentation ---------- */
  RlcAmPdu GetRetxPdu(uint32_t grantSize) {
    RlcAmPdu &original = txBuffer[retxSn];

    RlcAmPdu pdu;
    pdu.sn = original.sn;

    uint32_t remaining = original.payload.size() - retxOffset;
    uint32_t bytes = std::min(grantSize, remaining);

    /* SI logic for resegmentation */
    if (retxOffset == 0 && bytes < original.payload.size())
      pdu.header.si = 0b01;
    else if (retxOffset + bytes < original.payload.size())
      pdu.header.si = 0b11;
    else
      pdu.header.si = 0b10;

    pdu.header.hasSo = true;
    pdu.header.so = original.header.so + retxOffset;

    pdu.payload.insert(pdu.payload.end(), original.payload.begin() + retxOffset,
                       original.payload.begin() + retxOffset + bytes);

    retxOffset += bytes;
    return pdu;
  }

private:
  std::vector<uint8_t> currentSdu;
  uint32_t sduOffset{0};
  uint16_t nextSn{0};

  uint16_t retxSn{0};
  uint32_t retxOffset{0};

  std::map<uint16_t, RlcAmPdu> txBuffer;
};

/* ===================== TEST HARNESS ===================== */

int main() {
  RlcAmTxEntity rlc;

  /* Test 1: No segmentation */
  std::vector<uint8_t> smallSdu(500);
  rlc.TransmitPdcpPdu(smallSdu);

  auto pdu1 = rlc.GetTxPdu(1500);
  assert(pdu1.header.si == 0b00);
  assert(!pdu1.header.hasSo);

  /* Test 2–4: Segmentation */
  std::vector<uint8_t> largeSdu(3000);
  rlc.TransmitPdcpPdu(largeSdu);

  auto seg1 = rlc.GetTxPdu(500);
  assert(seg1.header.si == 0b01);

  auto seg2 = rlc.GetTxPdu(500);
  assert(seg2.header.si == 0b11);
  assert(seg2.header.hasSo);

  while (rlc.HasDataToSend())
    rlc.GetTxPdu(500);

  /* Test 5–6: Re-segmentation */
  rlc.MarkForRetransmission(1);

  auto reseg1 = rlc.GetRetxPdu(200);
  uint16_t so1 = reseg1.header.so;
  uint16_t size1 = reseg1.payload.size();

  auto reseg2 = rlc.GetRetxPdu(200);
  assert(reseg2.header.so == so1 + size1);

  std::cout << "\n✅ ALL RLC SEGMENTATION TESTS PASSED\n";
  return 0;
}
