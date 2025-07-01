#include <vector>
#include <mutex>
#include <atomic>

#include "../include/sharedConfig.h"

SharedConfigDBS::SharedConfigDBS() : version_{0}, state(1) {};

void SharedConfigDBS::updateData(const std::array<int16_t, 6>& new_data) {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        data = new_data;
        version_++;
    }
}

void SharedConfigDBS::toggleState(bool target_state) {
    if(state != target_state)
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            state = target_state;
        }
    }
}

// Try to get updated data if version changed.
// Returns true if data was updated and cached_data refreshed.
bool SharedConfigDBS::tryUpdateCache(std::array<int16_t, 6>& cached_data, uint64_t& cached_version) {
    uint64_t current_version = version_.load(std::memory_order_acquire);
    if (current_version != cached_version) {
        std::lock_guard<std::mutex> lock(mtx_);
        cached_data = data;
        cached_version = current_version;
        return true;
    }
    return false;
}
