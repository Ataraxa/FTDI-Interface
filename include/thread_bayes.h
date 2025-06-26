#ifndef THREAD_BAYES_H
#define THREAD_BAYES_H

#include "../include/threadSafeBuffer.h"
#include "../include/sharedConfig.h"

void thread_bayes(ThreadSafeBuffer& memoryBuffer, SharedConfig& config);

#endif