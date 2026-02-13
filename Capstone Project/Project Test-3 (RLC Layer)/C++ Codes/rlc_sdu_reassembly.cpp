// RLC SDU Reassembly - TS 38.322
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

enum class SI : uint8_t { FULL = 0, FIRST = 1, LAST = 2, MIDDLE = 3 };

struct Segment {
  SI si;
  uint16_t sn;
  uint16_t so;
  std::vector<uint8_t> data;
};

struct Reassembler {
  struct SduCtx {
    std::map<uint16_t, std::vector<uint8_t>> segments; // so -> data
    bool has_first = false, has_last = false;
    uint16_t total_len = 0;
  };
  std::map<uint16_t, SduCtx> pending;
  int delivered = 0;

  void receive_segment(const Segment &seg) {
    auto &ctx = pending[seg.sn];
    ctx.segments[seg.so] = seg.data;
    if (seg.si == SI::FULL) {
      ctx.has_first = ctx.has_last = true;
    }
    if (seg.si == SI::FIRST)
      ctx.has_first = true;
    if (seg.si == SI::LAST) {
      ctx.has_last = true;
      ctx.total_len = seg.so + seg.data.size();
    }
    std::cout << "  Recv SN=" << seg.sn << " SI=" << (int)seg.si
              << " SO=" << seg.so << " len=" << seg.data.size() << "\n";
    try_reassemble(seg.sn);
  }

  void try_reassemble(uint16_t sn) {
    auto &ctx = pending[sn];
    if (!ctx.has_first || !ctx.has_last)
      return;
    // Check contiguous
    uint16_t offset = 0;
    std::vector<uint8_t> sdu;
    for (auto &[so, data] : ctx.segments) {
      if (so != offset) {
        return;
      } // gap
      sdu.insert(sdu.end(), data.begin(), data.end());
      offset += data.size();
    }
    if (offset != ctx.total_len)
      return;
    std::cout << "  ** SDU SN=" << sn << " reassembled (" << sdu.size()
              << "B): ";
    for (auto b : sdu)
      std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    std::cout << std::dec << " **\n";
    pending.erase(sn);
    delivered++;
  }
};

int main() {
  std::cout << "=== RLC SDU Reassembly (TS 38.322 Sec 5.2.2.2) ===\n\n";
  Reassembler r;

  // SDU SN=3: arrives out of order (last, first, middle)
  std::cout << "SDU SN=3 (12 bytes, arrives out of order):\n";
  r.receive_segment({SI::LAST, 3, 8, {0x09, 0x0A, 0x0B, 0x0C}});
  r.receive_segment({SI::FIRST, 3, 0, {0x01, 0x02, 0x03, 0x04}});
  r.receive_segment({SI::MIDDLE, 3, 4, {0x05, 0x06, 0x07, 0x08}});

  std::cout << "\nSDU SN=4 (complete, no segmentation):\n";
  r.receive_segment({SI::FULL, 4, 0, {0xAA, 0xBB, 0xCC}});

  std::cout << "\nSDU SN=5 (missing middle segment):\n";
  r.receive_segment({SI::FIRST, 5, 0, {0x10, 0x20}});
  r.receive_segment({SI::LAST, 5, 6, {0x50, 0x60}});
  std::cout << "  (SN=5 cannot be reassembled - gap at SO=2)\n";

  std::cout << "\nDelivered: " << r.delivered << " SDUs\n";
  return 0;
}
