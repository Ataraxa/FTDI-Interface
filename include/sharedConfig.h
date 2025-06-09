#ifndef SHARED_INTS_NOTIFIER_H
#define SHARED_INTS_NOTIFIER_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>

class SharedConfig {
public:
    explicit SharedConfig(size_t num_ints);
    
    // Thread-safe write
    void updateInt(size_t index, int value);
    
    // Blocking wait-for-change
    const std::vector<int>& waitForChange();
    
    // Non-blocking check
    const std::vector<int>& getCurrent() const;
    
    void requestStop();
    bool shouldStop() const;

private:
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::vector<int> data_;
    std::atomic<bool> data_changed_{false};
    std::atomic<bool> stop_requested_{false};
};

#endif