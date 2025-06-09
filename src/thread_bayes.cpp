#include "../include/thread_bayes.h"
#include "../include/dummy_interfaces.h"

void thread_bayes(ThreadSafeBuffer& buffer, SharedConfig& config)
{
    std::ofstream outfile("data/output.txt", std::ios::app);

    while (!buffer.shouldStop()) 
    {
        auto data = buffer.getData();

        if (!data.empty())
        {
            for (const auto& item : data)
            {
                outfile << *item << "\n";
            }
            outfile.flush();

            buffer.returnToPool(data):

            longOperation();
        }
    }
}