#include "../include/threadSafeBuffer.h"
#include <fstream>
#include <thread>

// Initialize constants in constructor
ThreadSafeBuffer::ThreadSafeBuffer() 
    : stop_requested(false),
      max_buffer_size(1000),
      pool_size(1000) {
    memory_pool.reserve(pool_size);
    for (size_t i = 0; i < pool_size; ++i) {
        memory_pool.push_back(std::make_shared<std::uint32_t>());
        memory_pool.back()->reserve(256);
    }
}

void ThreadSafeBuffer::addData(const std::uint32_t& data) {
    std::shared_ptr<std::uint32_t> item;
    
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (!memory_pool.empty()) {
            item = std::move(memory_pool.back());
            memory_pool.pop_back();
        }
    }
    
    if (!item) {
        item = std::make_shared<std::uint32_t>();
        item->reserve(256);
    }
    
    *item = data;
    
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (buffer.size() >= max_buffer_size) {
            memory_pool.push_back(std::move(buffer.front()));
            buffer.pop_front();
        }
        buffer.push_back(std::move(item));
        cv.notify_one();
    }
}

std::vector<std::shared_ptr<std::uint32_t>> ThreadSafeBuffer::getData() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return !buffer.empty() || stop_requested; });
    
    std::vector<std::shared_ptr<std::uint32_t>> result;
    result.reserve(buffer.size());
    while (!buffer.empty()) {
        result.push_back(std::move(buffer.front()));
        buffer.pop_front();
    }
    return result;
}

void ThreadSafeBuffer::returnToPool(std::vector<std::shared_ptr<std::uint32_t>>& items) {
    std::unique_lock<std::mutex> lock(mtx);
    for (auto& item : items) {
        if (memory_pool.size() < pool_size) {
            item->clear();
            memory_pool.push_back(std::move(item));
        }
    }
}

void ThreadSafeBuffer::requestStop() {
    stop_requested = true;
    cv.notify_all();
}

bool ThreadSafeBuffer::shouldStop() const {
    return stop_requested;
}