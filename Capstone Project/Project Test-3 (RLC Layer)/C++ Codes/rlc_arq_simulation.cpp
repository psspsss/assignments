// RLC AM ARQ Simulation - TS 38.322
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct RLC_AM_ARQ {
  // State variables (TS 38.322 Sec 7.1)
  uint16_t VT_A = 0;      // Ack state: SN of oldest AMD PDU awaiting ack
  uint16_t VT_S = 0;      // Send state: SN for next new AMD PDU
  uint16_t VR_R = 0;      // Receive state: earliest SN not yet received
  uint16_t VR_H = 0;      // Highest received SN + 1
  uint16_t window = 2048; // AM window size (12-bit SN)

  std::map<uint16_t, std::string> tx_buf;
  std::map<uint16_t, bool> rx_buf;
  int retx_count = 0;

  void transmit(const std::string &data) {
    uint16_t sn = VT_S++;
    tx_buf[sn] = data;
    std::cout << "  TX SN=" << sn << " [" << data << "]"
              << " VT(A)=" << VT_A << " VT(S)=" << VT_S << "\n";
  }

  void receive(uint16_t sn, bool lost = false) {
    if (lost) {
      std::cout << "  ** SN=" << sn << " LOST in channel **\n";
      return;
    }
    rx_buf[sn] = true;
    if (sn >= VR_H)
      VR_H = sn + 1;
    while (rx_buf.count(VR_R))
      VR_R++;
    std::cout << "  RX SN=" << sn << " VR(R)=" << VR_R << " VR(H)=" << VR_H
              << "\n";
  }

  std::vector<uint16_t> gen_status() {
    std::vector<uint16_t> nacks;
    for (uint16_t s = VR_R; s < VR_H; s++)
      if (!rx_buf.count(s))
        nacks.push_back(s);
    std::cout << "  STATUS: ACK_SN=" << VR_H;
    if (!nacks.empty()) {
      std::cout << " NACKs={";
      for (auto n : nacks)
        std::cout << n << " ";
      std::cout << "}";
    }
    std::cout << "\n";
    return nacks;
  }

  void process_status(uint16_t ack_sn, std::vector<uint16_t> nacks) {
    VT_A = ack_sn;
    for (auto sn : nacks) {
      if (tx_buf.count(sn)) {
        std::cout << "  RETX SN=" << sn << "\n";
        retx_count++;
      }
    }
    for (auto it = tx_buf.begin(); it != tx_buf.end();)
      if (it->first < VT_A && !nacks.size())
        it = tx_buf.erase(it);
      else
        ++it;
  }
};

int main() {
  std::cout << "=== RLC AM ARQ Simulation (TS 38.322) ===\n\n";
  RLC_AM_ARQ arq;

  std::cout << "--- Transmit 5 PDUs ---\n";
  for (int i = 0; i < 5; i++)
    arq.transmit("DATA-" + std::to_string(i));

  std::cout << "\n--- Receive (SN=2 lost) ---\n";
  arq.receive(0);
  arq.receive(1);
  arq.receive(2, true); // lost!
  arq.receive(3);
  arq.receive(4);

  std::cout << "\n--- STATUS & Retransmit ---\n";
  auto nacks = arq.gen_status();
  arq.process_status(arq.VR_H, nacks);
  arq.receive(2); // retransmitted
  std::cout << "\nTotal retransmissions: " << arq.retx_count << "\n";
  return 0;
}
