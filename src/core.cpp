#include <iostream>
#include <thread>
#include <Windows.h>

#include "../include/threadSafeBuffer.h"
#include "../include/sharedConfig.h"
#include "../include/thread_adbs.h"
#include "../include/thread_bayes.h"

int main() {
    ThreadSafeBuffer buffer;
    SharedConfigDBS sharedStimConfig;
    
    // Initialise config from file 

    // Start aDBS thread

    std::thread adbs(thread_adbs, std::ref(buffer), std::ref(sharedStimConfig));
    DWORD threadId = adbs.native_handle();
    HANDLE threadHandle = OpenThread(THREAD_SET_INFORMATION, FALSE, threadId);

    BOOL res = SetThreadPriority(threadHandle, THREAD_PRIORITY_HIGHEST);
    if(!res)
    {
        printf("Couldnt increase thread priority...\n");
    }
    std::thread bayes(thread_bayes, std::ref(buffer), std::ref(sharedStimConfig));

    adbs.join();
    bayes.join();

    std::cout << "All threads completed\n";
    return 0;
}