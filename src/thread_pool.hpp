#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <array>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>


//#include <emmintrin.h>


#if defined(__arm__)
#elif defined(__aarch64__)
#elif defined(__i386__)
#elif defined(__x86_64__)
#elif defined(__MACHINEARM)
#elif defined(__MACHINEARM64)
#elif defined(__MACHINEARM64_X64)
#elif defined(__MACHINEARM_ARM64_X64)
#elif defined(__MACHINEX86_X64)
#elif defined(__MACHINEX64)
#elif defined(__LP64__) // for Mac
#else
#define USE_ATOMIC_FOR_POINTER 1
#endif



#ifdef USE_ATOMIC_FOR_POINTER
#define POINTER_ATOMIC_TYPE(T) std::atomic<T>
#define POINTER_STORE(pointer, val) pointer.store(val, std::memory_order_relaxed)
#define POINTER_LOAD(pointer) pointer.load(std::memory_order_relaxed)
#else
#define POINTER_ATOMIC_TYPE(T) T
#define POINTER_STORE(pointer, val) pointer = val
#define POINTER_LOAD(pointer) pointer

#endif



/**
 *  〇 -> 〇 -> 〇
 *   |     |
 *  〇    〇
 */



using NodeIndexInteger = unsigned long long;

class TestUtil {
public:
    static int testMoveSemantics();

    static int copyConstructTest();
};

struct RunNode {
public:
    POINTER_ATOMIC_TYPE(RunNode *) next{ nullptr };
    POINTER_ATOMIC_TYPE(NodeIndexInteger) nodeIndex{ 1 };
    int func_id;
    void *parameters;
    //long long idx{ 0 };
    //int thread_index{ 0 };
};


class ThreadQueue {
public:
    POINTER_ATOMIC_TYPE(RunNode *) lastRunNode{ nullptr };
    POINTER_ATOMIC_TYPE(RunNode *) doingRunNode{ nullptr };
    std::atomic<NodeIndexInteger> currentDoingIndex{ 1 };
    std::atomic<NodeIndexInteger> currentLastNodeIndex{ 1 };
    std::atomic<RunNode *> firstRunNode{ nullptr };
    unsigned long long lastNodeIndex {0};
};


constexpr int thread_queue_size = 6;
constexpr int threads_size = 6;

class ThreadPool;

class GreenThread {
public:

    RunNode *cacheRunNodeFirst{ nullptr };
    RunNode *cacheRunNodeEnd{ nullptr };

    unsigned int cacheUsedCount{ 0 };
    unsigned int cacheAddedCount{ 0 };

    std::condition_variable main_sleeper_cond;
    //std::atomic<bool> main_sleeper_ready{ false };
    //std::mutex main_sleeper_mutex;


    ThreadPool *threadPool;

    int prev_idx = 0;

    //	inline bool addRunNode(RunNode *runNode);

    long long idx;
    int thread_index;

    long long set_count{ 0 };
    int remove_count{ 0 };


    // volatile
    //std::atomic<bool> stop{ false };
    bool stop{ false };
    std::thread *tha;

    long currentRunNodeIndex = 1000000;
    long disposinngIndex = 0;

    RunNode *buffer{ nullptr };
    RunNode *prev_buffer{ nullptr };

    RunNode *getNextRunNode();

    int init(int idx3);

    //void startConsuming();
    void start();

    void join();

    void doStop();
};


class ThreadPool {
public:
    std::vector<GreenThread *> threads;
    std::array<ThreadQueue *, thread_queue_size> thread_queue;

    int init();

    int start();
};

