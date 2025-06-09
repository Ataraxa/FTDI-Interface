#include<vector>
#include <thread>

#include "../include/thread_adbs.h"
#include "../include/threadSafeBuffer.h"
#include "../include/sharedConfig.h"
#include "../include/dummy_interfaces.h"

void thread_adbs(ThreadSafeBuffer& buffer, SharedConfig& sharedCfg) {
    std::vector<int> config;
    
    
    config = sharedCfg.getCurrent();
    int frequency = config[0];
    int amplitude = config[1];

    uint32_t data;

    std::chrono::milliseconds waitTime(50);

    // Main loop
    for(int i=0;i<1000;i++)
    {
        data = dummyReadADC();
        buffer.addData(data);
        std::this_thread::sleep_for(waitTime);
    }
}
