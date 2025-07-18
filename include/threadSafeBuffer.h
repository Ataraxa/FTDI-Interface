#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <cstdint>

class ThreadSafeBuffer {
public:
    ThreadSafeBuffer();

    void addData(uint16_t data);
    std::vector<uint16_t> getData();

    void requestStop();
    bool shouldStop() const;
    bool isEmpty();

private:
    std::queue<uint16_t> buffer;
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stop_requested;
};