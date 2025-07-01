#ifndef SHARED_INTS_NOTIFIER_H
#define SHARED_INTS_NOTIFIER_H

#include <array>
#include <mutex>
#include <atomic>

class SharedConfigDBS {
    
public:
    SharedConfigDBS();
    
    void updateData(const std::array<int16_t, 6>& new_data);
    bool tryUpdateCache(std::array<int16_t, 6>& cached_data, uint64_t& cached_version);
    void toggleState(bool target_state);
    bool state;
    std::array<int16_t, 6> data = {0x0987, 0x0917, 100, 100, 100, 130}; // Initial values

private:
    std::mutex mtx_;
    std::atomic<uint64_t> version_{0};
};


#endif