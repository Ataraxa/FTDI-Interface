#include "ThreadSafeBuffer.h"
#include <cstdio>

ThreadSafeBuffer::ThreadSafeBuffer() : stop_requested(false) {}

void ThreadSafeBuffer::addData(uint16_t data) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        buffer.push(data);
    }
    cv.notify_one();
}

std::vector<uint16_t> ThreadSafeBuffer::getData() {
    std::unique_lock<std::mutex> lock(mtx);

    std::vector<uint16_t> data;

    while (!buffer.empty()) {
        data.push_back(buffer.front());
        buffer.pop();
    }

    return data;
}

bool ThreadSafeBuffer::isEmpty() {
    return buffer.empty();
}

void ThreadSafeBuffer::requestStop() {
    stop_requested = true;
    cv.notify_all();
}

bool ThreadSafeBuffer::shouldStop() const {
    return stop_requested;
}