#include "ThreadSafeBuffer.h"

ThreadSafeBuffer::ThreadSafeBuffer() : stop_requested(false) {}

void ThreadSafeBuffer::addData(uint16_t data) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        buffer.push(data);
    }
    cv.notify_one();
}

std::vector<uint16_t> ThreadSafeBuffer::getData(size_t min_elements) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this, min_elements] {
        return buffer.size() >= min_elements || stop_requested;
    });

    std::vector<uint16_t> data;
    while (!buffer.empty()) {
        data.push_back(buffer.front());
        buffer.pop();
    }
    return data;
}

void ThreadSafeBuffer::requestStop() {
    stop_requested = true;
    cv.notify_all();
}

bool ThreadSafeBuffer::shouldStop() const {
    return stop_requested;
}