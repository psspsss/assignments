
#include <iostream>
#include <vector>
#include <functional>
#include <cstdint>

struct UmHeader {
    uint16_t sn;
    bool fi_start;
    bool fi_end;
};

struct UmPdu {
    UmHeader h;
    std::vector<uint8_t> payload;
};

class RlcUmRx {
public:
    using DeliverFn = std::function<void(const std::vector<uint8_t>&)>;

    explicit RlcUmRx(DeliverFn deliver) : deliver_(deliver) {}

    void onPdu(const UmPdu& pdu) {
        if (pdu.h.fi_start) {
            reassembly_.clear();
        }

        reassembly_.insert(
            reassembly_.end(),
            pdu.payload.begin(),
            pdu.payload.end());

        if (pdu.h.fi_end) {
            deliver_(reassembly_);
            reassembly_.clear();
        }
    }

private:
    DeliverFn deliver_;
    std::vector<uint8_t> reassembly_;
};

int main() {
    RlcUmRx rx([](const std::vector<uint8_t>& sdu) {
        std::cout << "DELIVERED SDU (NEW): size=" << sdu.size() << "\nData: ";
        for (auto b : sdu) std::cout << int(b) << " ";
        std::cout << "\n";
    });

    rx.onPdu({{0, true, false},  {0,1,2,3,4,5,6,7,8,9}});
    rx.onPdu({{1, false, false}, {10,11,12,13,14,15,16,17,18,19}});
    rx.onPdu({{2, false, true},  {20,21,22,23,24}});
}
