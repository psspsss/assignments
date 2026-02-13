
// RLC SDU Discard Timer - TS 38.322
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct SDU_Entry {
  uint16_t sn;
  std::vector<uint8_t> data;
  int timer_ms; // discardTimer value
  int elapsed_ms = 0;
  bool transmitted = false;
  bool discarded = false;
};

struct RLC_TX_Buffer {
  std::map<uint16_t, SDU_Entry> buffer;
  int discard_timer_ms;
  int discarded_count = 0;

  RLC_TX_Buffer(int timer) : discard_timer_ms(timer) {}

  void enqueue_sdu(uint16_t sn, std::vector<uint8_t> data) {
    buffer[sn] = {sn, data, discard_timer_ms, 0, false, false};
    std::cout << "  Enqueue SN=" << sn << " (" << data.size()
              << "B) timer=" << discard_timer_ms << "ms\n";
  }

  void transmit_sdu(uint16_t sn) {
    if (buffer.count(sn) && !buffer[sn].discarded) {
      buffer[sn].transmitted = true;
      std::cout << "  Transmit SN=" << sn << "\n";
    }
  }

  void advance_time(int delta_ms) {
    std::cout << "  -- Time +" << delta_ms << "ms --\n";
    for (auto &[sn, entry] : buffer) {
      if (entry.discarded)
        continue;
      entry.elapsed_ms += delta_ms;
      if (entry.elapsed_ms >= entry.timer_ms) {
        entry.discarded = true;
        discarded_count++;
        std::cout << "  ** SN=" << sn << " DISCARDED (timer expired at "
                  << entry.elapsed_ms << "ms)";
        if (!entry.transmitted)
          std::cout << " [never transmitted]";
        else
          std::cout << " [was transmitted, awaiting ACK]";
        std::cout << "\n";
      }
    }
  }

  void show_buffer() {
    std::cout << "  Buffer: ";
    int active = 0;
    for (auto &[sn, e] : buffer) {
      if (!e.discarded) {
        std::cout << "SN=" << sn << "(" << e.elapsed_ms << "ms) ";
        active++;
      }
    }
    if (active == 0)
      std::cout << "(empty)";
    std::cout << " | Discarded: " << discarded_count << "\n";
  }
};

int main() {
  std::cout << "=== RLC SDU Discard Timer (TS 38.322 Sec 5.4) ===\n\n";
  RLC_TX_Buffer buf(300); // discardTimer = 300ms

  buf.enqueue_sdu(0, {0xAA, 0xBB});
  buf.enqueue_sdu(1, {0xCC, 0xDD});
  buf.enqueue_sdu(2, {0xEE, 0xFF});

  buf.transmit_sdu(0);
  buf.transmit_sdu(1);
  // SN=2 not transmitted (e.g., no UL grant)

  buf.advance_time(100);
  buf.show_buffer();
  buf.advance_time(100);
  buf.show_buffer();
  buf.advance_time(150); // 350ms total > 300ms timer
  buf.show_buffer();

  std::cout << "\nValid discardTimer values: 10,15,20,25,30,40,50,\n"
            << "  60,75,100,150,200,250,300,500,750,1500ms, infinity\n";
  return 0;
}
