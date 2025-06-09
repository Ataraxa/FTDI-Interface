#include <iostream>
#include <thread>

#include "../include/threadSafeBuffer.h"
#include "../include/sharedConfig.h"
#include "../include/thread_adbs.h"
#include "../include/thread_bayes.h"

int main() {
    ThreadSafeBuffer buffer;
    SharedConfig config(3);
    
    // Initialise config from file 

    // Start aDBS thread

    std::thread adbs(thread_adbs, std::ref(buffer), std::ref(config));
    std::thread bayes(thread_bayes, std::ref(buffer), std::ref(config));

    adbs.join();
    bayes.join();

    std::cout << "All threads completed\n";
    return 0;
}