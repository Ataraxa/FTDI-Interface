#ifndef THREAD_SAFE_BUFFER_H
#define THREAD_SAFE_BUFFER_H

#include <memory>
#include <cstdint>
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadSafeBuffer {
public:
    ThreadSafeBuffer();
    void addData(const std::uint32_t& data);
    std::vector<std::shared_ptr<std::uint32_t>> getData();
    void returnToPool(std::vector<std::shared_ptr<std::uint32_t>>& items);
    void requestStop();
    bool shouldStop() const;

    // Delete copy constructor/assignment
    ThreadSafeBuffer(const ThreadSafeBuffer&) = delete;
    ThreadSafeBuffer& operator=(const ThreadSafeBuffer&) = delete;

private:
    std::deque<std::shared_ptr<std::uint32_t>> buffer;
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stop_requested;
    const size_t max_buffer_size;
    
    std::vector<std::shared_ptr<std::uint32_t>> memory_pool;
    const size_t pool_size;
};

#endif // THREAD_SAFE_BUFFER_H