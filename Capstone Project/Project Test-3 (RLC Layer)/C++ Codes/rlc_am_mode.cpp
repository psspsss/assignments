// RLC Acknowledged Mode - TS 38.322
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

enum class SI : uint8_t { FULL = 0, FIRST = 1, LAST = 2, MIDDLE = 3 };

struct AMD_PDU {
  uint8_t dc = 1; // 1=Data, 0=Control
  uint8_t p = 0;  // Poll bit
  SI si = SI::FULL;
  uint16_t sn;     // 12 or 18 bit
  uint16_t so = 0; // Segment offset
  std::vector<uint8_t> data;

  std::vector<uint8_t> encode_12bit() const {
    std::vector<uint8_t> pdu;
    uint8_t b0 = (dc << 7) | (p << 6) | (static_cast<uint8_t>(si) << 4) |
                 ((sn >> 8) & 0x0F);
    pdu.push_back(b0);
    pdu.push_back(sn & 0xFF);
    if (si == SI::LAST || si == SI::MIDDLE) {
      pdu.push_back((so >> 8) & 0xFF);
      pdu.push_back(so & 0xFF);
    }
    pdu.insert(pdu.end(), data.begin(), data.end());
    return pdu;
  }
};

struct STATUS_PDU {
  uint16_t ack_sn;
  struct NACK {
    uint16_t nack_sn;
    uint16_t so_start;
    uint16_t so_end;
  };
  std::vector<NACK> nacks;

  std::vector<uint8_t> encode_12bit() const {
    std::vector<uint8_t> pdu;
    uint8_t b0 = 0x00 | ((ack_sn >> 8) & 0x0F); // D/C=0, CPT=000
    pdu.push_back(b0);
    pdu.push_back(ack_sn & 0xFF);
    uint8_t e1 = nacks.empty() ? 0 : 1;
    pdu.push_back(e1 << 7);
    for (auto &n : nacks) {
      pdu.push_back((n.nack_sn >> 4) & 0xFF);
      pdu.push_back(((n.nack_sn & 0x0F) << 4));
    }
    return pdu;
  }
};

void hex_dump(const std::string &l, const std::vector<uint8_t> &d) {
  std::cout << l;
  for (auto b : d)
    std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b << " ";
  std::cout << std::dec << "\n";
}

int main() {
  std::cout << "=== RLC AM Mode (TS 38.322 Sec 5.2.3) ===\n\n";
  AMD_PDU amd;
  amd.sn = 42;
  amd.p = 1;
  amd.si = SI::FULL;
  amd.data = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02};
  hex_dump("AMD PDU (SN=42, P=1): ", amd.encode_12bit());
  std::cout << "  D/C=1(Data) P=1(Poll) SI=00(Full) SN=42\n\n";

  STATUS_PDU st;
  st.ack_sn = 45;
  st.nacks = {{43, 0, 0}};
  hex_dump("STATUS PDU (ACK=45, NACK=43): ", st.encode_12bit());
  std::cout << "  D/C=0(Ctrl) ACK_SN=45 NACK_SN=43\n";
  std::cout << "  -> Receiver got SN 42,44 but missing SN 43\n";
  return 0;
}
