#include "../include/sharedConfig.h"

SharedConfig::SharedConfig(size_t num_ints) 
    : data_(num_ints, 0) {}  // Initialize all to 0

void SharedConfig::updateInt(size_t index, int value) {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (index < data_.size()) {
            data_[index] = value;
            data_changed_ = true;
        }
    }
    cv_.notify_one();
}

const std::vector<int>& SharedConfig::waitForChange() {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this] {
        return data_changed_ || stop_requested_;
    });
    data_changed_ = false;
    return data_;
}

const std::vector<int>& SharedConfig::getCurrent() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return data_;
}

void SharedConfig::requestStop() {
    stop_requested_ = true;
    cv_.notify_all();
}

bool SharedConfig::shouldStop() const {
    return stop_requested_;
}