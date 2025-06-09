#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <chrono>
#include <atomic>
#include <memory>
#include <deque>
#include <cstdint>

class ThreadSafeBuffer {
private:
    std::deque<std::shared_ptr<std::uint32_t>> buffer;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stop_requested{false};
    const size_t max_buffer_size = 1000;
    
    // Memory pool optimization
    std::vector<std::shared_ptr<std::uint32_t>> memory_pool;
    const size_t pool_size = 1000;

public:
    ThreadSafeBuffer() {
        memory_pool.reserve(pool_size);
        for (size_t i = 0; i < pool_size; ++i) {
            memory_pool.push_back(std::make_shared<std::uint32_t>());
            memory_pool.back()->reserve(256);
        }
    }

    void addData(const std::uint32_t& data) {
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

    std::vector<std::shared_ptr<std::string>> getData() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !buffer.empty() || stop_requested; });
        
        std::vector<std::shared_ptr<std::string>> result;
        result.reserve(buffer.size());
        while (!buffer.empty()) {
            result.push_back(std::move(buffer.front()));
            buffer.pop_front();
        }
        return result;
    }

    void returnToPool(std::vector<std::shared_ptr<std::string>>& items) {
        std::unique_lock<std::mutex> lock(mtx);
        for (auto& item : items) {
            if (memory_pool.size() < pool_size) {
                item->clear();
                memory_pool.push_back(std::move(item));
            }
        }
    }

    void requestStop() {
        stop_requested = true;
        cv.notify_all();
    }

    bool shouldStop() const {
        return stop_requested;
    }
};

void writer_thread(ThreadSafeBuffer& buffer) {
    for (int i = 0; i < 10000; ++i) {
        std::string data = "Data item " + std::to_string(i) + " " + std::string(200, 'x');
        buffer.addData(data);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    buffer.requestStop();
}

void reader_thread(ThreadSafeBuffer& buffer) {
    // Open file with large buffer (correct way)
    char file_buffer[1 << 20]; // 1MB buffer
    std::ofstream outfile("output.txt", std::ios::app);
    outfile.rdbuf()->pubsetbuf(file_buffer, sizeof(file_buffer));
    
    while (!buffer.shouldStop()) {
        auto data = buffer.getData();
        
        if (!data.empty()) {
            for (const auto& item : data) {
                outfile << *item << "\n";
            }
            outfile.flush();
            
            buffer.returnToPool(data);
        }
    }
    
    // Write any remaining data
    auto final_data = buffer.getData();
    for (const auto& item : final_data) {
        outfile << *item << "\n";
    }
    outfile.close();
}

int main() {
    ThreadSafeBuffer buffer;

    std::thread writer(writer_thread, std::ref(buffer));
    std::thread reader(reader_thread, std::ref(buffer));

    writer.join();
    reader.join();

    std::cout << "All threads completed\n";
    return 0;
}