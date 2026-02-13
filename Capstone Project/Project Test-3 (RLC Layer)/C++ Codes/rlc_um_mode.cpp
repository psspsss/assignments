// RLC Unacknowledged Mode - TS 38.322
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>

enum class SI : uint8_t {
  FULL = 0b00,  // Complete SDU
  FIRST = 0b01, // First segment
  LAST = 0b10,  // Last segment
  MIDDLE = 0b11 // Middle segment
};

struct UMD_PDU {
  SI si;
  uint16_t sn; // 6-bit or 12-bit
  uint16_t so; // Segment Offset (present if SI=MIDDLE or LAST)
  std::vector<uint8_t> data;
  uint8_t sn_size; // 6 or 12

  std::vector<uint8_t> encode() const {
    std::vector<uint8_t> pdu;
    uint8_t si_val = static_cast<uint8_t>(si);
    if (sn_size == 6) {
      pdu.push_back((si_val << 6) | (sn & 0x3F));
    } else {
      pdu.push_back((si_val << 6) | ((sn >> 8) & 0x0F));
      pdu.push_back(sn & 0xFF);
    }
    if (si == SI::LAST || si == SI::MIDDLE) {
      pdu.push_back((so >> 8) & 0xFF);
      pdu.push_back(so & 0xFF);
    }
    pdu.insert(pdu.end(), data.begin(), data.end());
    return pdu;
  }
};

void hex_dump(const std::string &label, const std::vector<uint8_t> &d) {
  std::cout << label;
  for (auto b : d)
    std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b << " ";
  std::cout << std::dec << "\n";
}

int main() {
  std::cout << "=== RLC UM Segmentation (TS 38.322 Sec 5.2.2) ===\n\n";
  std::vector<uint8_t> sdu(20);
  for (int i = 0; i < 20; i++)
    sdu[i] = 0xA0 + i;
  hex_dump("Original SDU (20B): ", sdu);

  uint16_t sn = 5;
  // Segment into 3 parts: 8 + 6 + 6 bytes
  UMD_PDU p1{SI::FIRST, sn, 0, {sdu.begin(), sdu.begin() + 8}, 12};
  UMD_PDU p2{SI::MIDDLE, sn, 8, {sdu.begin() + 8, sdu.begin() + 14}, 12};
  UMD_PDU p3{SI::LAST, sn, 14, {sdu.begin() + 14, sdu.end()}, 12};

  std::cout << "\nSegmented into 3 UMD PDUs (SN=" << sn << "):\n";
  hex_dump("  PDU1 (FIRST,  SO=0):  ", p1.encode());
  hex_dump("  PDU2 (MIDDLE, SO=8):  ", p2.encode());
  hex_dump("  PDU3 (LAST,   SO=14): ", p3.encode());

  std::cout << "\nHeader: SI(2b)|SN(12b) [+SO(16b) if segmented]\n";
  return 0;
}
