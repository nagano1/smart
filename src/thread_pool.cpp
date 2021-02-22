#include "thread_pool.hpp"

#include <stdio.h>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <array>
#include <algorithm>


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



constexpr int BUFFER_SIZE = 16;

int conflict = 0;
int conflict2 = 0;
int set_count_b = 0;


void GreenThread::doStop() {
    this->stop = true;
};

void GreenThread::join() {
    this->tha->join();
}

constexpr int SIZE_ARRAY = 10240;


struct A {
    int* p;

    A() : p(new int(9)) {}

    A(A&& v) : p(v.p) {
        v.p = nullptr;
    }

    ~A() { delete p; }
};


#define _move false? throw 3:
#define _copy false? throw 3:
#define mauto auto
#define let auto
#define mut auto   
#define con const auto
#define var auto
struct mytype {
public:
    static void trans(const mytype &another, const mytype &m2);

    int func() {
        return 32;
    };

    mytype(const mytype &a) { std::cout << "Copy Constructor " << std::endl; }

    mytype& operator=(mytype &other)//;h = delete;
    {
        printf("AAAAAAAAAAA");
        return other;
        /*
        *
        this = other;
        return *this;
        */
    }

    mytype() {

    }

    mytype(mytype &&a) noexcept {
        std::cout << "Move Constructor " << std::endl; 
    }
    mytype& operator=(mytype&& v) { // ムーブ代入演算子
        //if (&v != this) {   // 自己代入の回避
        //    delete p;         // 既存のリソースを解放
        //    p = v.p;
        //    v.p = nullptr;
        //}
        return *this;
    }
    /*
    */
    /*
    mytype(const mytype &myClass)
    {
        //this->copy(myClass);
    }
    */
};

/*
std::vector<int>&& func() {
    std::vector<int> x;
    std::vector<int> y = x;


    return std::move(y);// std::move(x);
}
*/

int TestUtil::copyConstructTest() {
    /*
    ref	stack const
    // mytype m2 = mytype();
    */
    {
        const mytype &m2 = mytype{};// stack
    }


    /*
        ref stack mutable
        mu mytype m1 = mytype();
        mytype m2 = mytype();
        m1 = m2;
    */

    /*
    ref stack mutable
    mu mytype m1 = mytype();
    {
        mytype m2 = mytype();
        m1 = m2;
    }
    */


    // uint32
    {
        //printf("\n[");
        mytype __m1__ = mytype{};// stack //backing field   
        mytype *m1 = &__m1__;// mytype{};// stack
        mytype m2 = mytype{};// stack

        // mytype another = trans(m2);//.copy(	) }
        mytype another;
        //mytype::trans(another, m2);

        // mytype another;
        // another = trans(m2);
//		mytype another;
    //	mytype::trans(another, m2);

        

        m1 = &m2;
        // m1.func
        m2.func();
        for (int m = 0; m < 100000; m++) {
            //k += m1->func();
        }
        /*
        */



        {
            /*
            mytype intA = mytype();
            {

            }
*/
            mytype __intA__ = mytype{};// stack //backing field   
            mytype *intA= &__intA__;// mytype{};// stack
            mytype intB = mytype{};// stack
            //var intA = mytype();
            //con intB = mytype(); // caution


            mut aiwejfoiawjofaioawjeiof = 234;
            let iofweoif = 3294;
            auto lfe = 324;
            mytype intW = mytype();
            //const intG = mytype();//val intB = mytype(); // caution
            mauto *intG = new mytype();//val intB = mytype(); // caution


            intA = &intB;
            //intA = intB;

            {
                auto intC = mytype();

                *intA = std::move(intC);
                //intA = move intC;

                //*intA = copy(intC);
                //intA = copy intC;
            }
        }


        //int &m2 = m1;// mytype{};// stack
    }

    return 0;
}


int TestUtil::testMoveSemantics() {
    A ka = std::move(A()); // 一時オブジェクトの値を変数aに移動
    printf("%d", *ka.p);  // "9"（一時オブジェクトから引き継いだ値）
    // 一時オブジェクト`A()`のデストラクタが呼ばれても、
    // 変数aにはなんの影響もない（ポインタa.pはまだ解放されていない）

    return 10;
}


// for each queue
struct DisposeItem {
    std::atomic<RunNode*> targetDoingRunNode{ nullptr };
    std::array<std::atomic<int>, threads_size> list;

    std::atomic<bool> requested{ false }; // any thread can be a sender of a request
    std::atomic<bool> disposed{ false }; // any thread can be a disposer
    int retryCount{ 0 };

    std::mutex mtx;
};

struct DisposeRequest {
public:
    std::array<DisposeItem, thread_queue_size> dispose_requests;
    std::array<std::atomic<bool>, threads_size> atomic_for_cache;

    std::atomic<bool> disposed{ false }; // any thread can be a disposer

    std::mutex mtx;

};

std::array<std::atomic<int>, threads_size> geeze;

DisposeRequest shared_dispose_data;


RunNode *GreenThread::getNextRunNode() {
    this->currentRunNodeIndex++;
    if (this->currentRunNodeIndex >= SIZE_ARRAY) {
        if (this->prev_buffer != nullptr) {
            delete[] this->prev_buffer;// = new RunNode[10240];
        }

        if (this->buffer != nullptr) {
            this->prev_buffer = this->buffer;
        }

        this->buffer = new RunNode[SIZE_ARRAY];
        this->currentRunNodeIndex = 0;
    }

    return &this->buffer[this->currentRunNodeIndex];
}



void GreenThread::start() {

    this->tha = new std::thread([this]() {

        // Add work to central work queues
        auto gfunc = [this](uint64_t i) {
            auto &thq = this->threadPool->thread_queue;

            RunNode *runNode = nullptr;// new RunNode();
            //auto *runNode = this->getNextRunNode();// new RunNode();

            if (this->cacheRunNodeFirst != this->cacheRunNodeEnd) {

                if (this->cacheAddedCount - this->cacheUsedCount > 200) {
                    //printf("here;%llu - %llu", this->cacheAddedCount.load(), this->cacheUsedCount.load());

                    auto *node = this->cacheRunNodeFirst;//.load();
                    //auto *nextTemp = node->next.load();
                    auto *nextTemp = POINTER_LOAD(node->next);// .load();


                    if (nextTemp != nullptr) {
                        //runNode->rawnext = nullptr;
                        /*
                        if (i % 70000 == 1) {
                            printf("k", this->cacheAddedCount.load() - this->cacheUsedCount.load());
                            //printf(";%llu", this->cacheAddedCount.load()- this->cacheUsedCount.load());
                        }
                        */
                        runNode = node;// this->cacheRunNodeFirst.load();
                        this->cacheRunNodeFirst = nextTemp;// , std::memory_order_relaxed);
                        runNode->next = (nullptr);// , std::memory_order_relaxed);
                        this->cacheUsedCount = (this->cacheUsedCount + 1);// , std::memory_order_relaxed);
                    }
                    else {
                        // assert
                        //printf("miss;");
                    }
                }
            }

            bool printg = false;
            if (runNode == nullptr) {
                /*
                if (i % 70000 == 1) {
                    printg = true;
                    printf("g");
                }
                                */

                                //runNode = getNextRunNode();//new RunNode();
                runNode = new RunNode();
            }

            int repeatedError = 0;

            bool prev_idx_changed = false;






            auto k = this->prev_idx;

            while (true) {
                //for (int km = 0; km < thread_queue_size; km++) {
                auto *thread_queue = thq[k];
                auto *lastNode = POINTER_LOAD(thread_queue->lastRunNode);
                auto *old_next = POINTER_LOAD(lastNode->next);
                if (old_next == nullptr) {

                    auto currentNodeIndex = thread_queue->currentLastNodeIndex.load(std::memory_order_relaxed);
                    auto nodeIndex = POINTER_LOAD(lastNode->nodeIndex);

                    if (currentNodeIndex == nodeIndex) {
                        NodeIndexInteger nextValue = currentNodeIndex + static_cast<NodeIndexInteger>(1);
                        auto result = thread_queue->currentLastNodeIndex.compare_exchange_strong(currentNodeIndex, nextValue);
                        if (result) {
                            thread_queue->lastNodeIndex++;
                            POINTER_STORE(runNode->nodeIndex, nextValue);// , std::memory_order_release);
                            POINTER_STORE(thread_queue->lastRunNode, runNode);// , std::memory_order_release);
                            POINTER_STORE(lastNode->next, runNode);// , std::memory_order_release);
                            this->set_count++;

                            if (prev_idx_changed) {
                                this->prev_idx = k;
                            }

                            repeatedError = 0;

                            return true;
                        }
                        else {
                            //throw 1;
                        }
                    }
                    else {
                    }
                }

                //std::this_thread::sleep_for(std::chrono::milliseconds(1));
                /*
                repeatedError++;
                if (repeatedError > 50000) {
                    //printf("%d:{%llu-%llu}",repeatedError, a , lastNode->nodeIndex.load());
                }
                */
                k++;
                if (k < thread_queue_size == false) {
                    k = 0;
                }
                prev_idx_changed = true;

                //}

                //return false;
            }

            printf("failed!!!");
            return false;
        };

        auto dispose = [this](int threadIndex, long queueIndex, RunNode * &doingRunNode) {
            this->disposinngIndex++;
            constexpr static int base_dispose_size = 4000;// 10000;
            constexpr static int each_dispose_size = base_dispose_size / 20;
            if (this->disposinngIndex > base_dispose_size) {
                this->disposinngIndex = 0;


                bool delete_done_once = false;
                for (int q = 0; q < thread_queue_size; q++) {

                    auto &req = shared_dispose_data.dispose_requests[q];
                    bool shouldScanForDispose = true;

                    if (q == queueIndex) {
                        if (req.requested) {
                            if (req.targetDoingRunNode.load() != nullptr) {
                                if (req.list[threadIndex] == 0) {
                                    req.list[threadIndex].store(1, std::memory_order_relaxed);
                                    shouldScanForDispose = false;
                                }
                            }
                            else {
                                shouldScanForDispose = false;
                            }
                        }
                        else {
                            shouldScanForDispose = false;

                            //std::lock_guard<std::mutex> guard{ shared_dispose_data.mtx };


                            // try to make a dispose request
                            bool isfalse = false;
                            auto result = req.requested.compare_exchange_weak(isfalse, true);
                            if (result) {
                                // START!
                                // Initialize
                                for (int i = 0; i < threads_size; i++) {
                                    if (i == this->thread_index) {
                                        req.list[i].store(1, std::memory_order_relaxed);
                                    }
                                    else {
                                        req.list[i].store(0, std::memory_order_relaxed);
                                    }
                                }
                                req.targetDoingRunNode = doingRunNode;
                            }
                        }
                    }
                    else {

                        if (req.requested) {
                            if (req.targetDoingRunNode.load() != nullptr) {
                                if (req.list[threadIndex].load() == 0) {
                                    req
                                        .list[threadIndex]
                                        .store(1, std::memory_order_relaxed);

                                    shouldScanForDispose = false;
                                }
                            }
                            else {
                                shouldScanForDispose = false;
                            }
                        }
                        else {
                            shouldScanForDispose = false;
                        }
                    }

                    if (shouldScanForDispose) {
                        bool doneDelete = false;

                        if (delete_done_once == false) {
                            delete_done_once = true;
                            //std::lock_guard<std::mutex> guard{ shared_dispose_data.mtx };
                            //req.retryCount++;

                            if (req.disposed == false) {
                                bool is_false = false;
                                auto result = req.disposed.compare_exchange_weak(is_false, true);
                                if (result) {
                                    auto *targetDoingRunNode = req.targetDoingRunNode.load();
                                    if (targetDoingRunNode == nullptr) {
                                        printf("null;");
                                    }

                                    if (targetDoingRunNode != nullptr) {
                                        bool ok = true;
                                        for (int i = 0; i < threads_size; i++) {
                                            if (req.list[i].load() != 1) {
                                                ok = false;
                                                break;
                                            }
                                        }

                                        if (ok == false) {
                                            //req.retryCount++;
                                            if (req.retryCount > 12200) {
                                                printf("[=%d]", q);
                                                ok = true;
                                            }
                                        }

                                        if (ok) {
                                            doneDelete = true;
                                            // Dispose
                                            auto *node = this->threadPool->thread_queue[q]->firstRunNode.load();
                                            auto *firstNode = node;
                                            int total_delet_count = 0;
                                            int temp_delete_count = 0;
                                            int next_add_thread_index = 0;
                                            bool shouldBreak = false;

                                            while (node) {
                                                auto *next = POINTER_LOAD(node->next);
                                                if (node == targetDoingRunNode) {
                                                    shouldBreak = true;
                                                    //if (count % 20 == 0) {
//														printf("[g%d, %d, retry=%d]", q, total_delet_count, req.retryCount);
                                                    //}
                                                    if (next) {
                                                        this->threadPool->thread_queue[q]->firstRunNode.store(next, std::memory_order_relaxed);
                                                    }
                                                }

                                                temp_delete_count++;
                                                total_delet_count++;
                                                if (temp_delete_count > each_dispose_size || shouldBreak) {
                                                    auto &target_thread = this->threadPool->threads[next_add_thread_index];
                                                    auto should_delete = target_thread->cacheAddedCount - target_thread->cacheUsedCount > 8000;

                                                    if (should_delete == false) {
                                                        auto &atomic_cache = shared_dispose_data.atomic_for_cache[next_add_thread_index];
                                                        if (atomic_cache.compare_exchange_weak(is_false, true)) {
                                                            /*
                                                            auto *end = target_thread->cacheRunNodeEnd.load();
                                                            node->next.store(nullptr, std::memory_order_relaxed);
                                                            target_thread->cacheRunNodeEnd.store(node, std::memory_order_relaxed);
                                                            end->next.store(firstNode, std::memory_order_relaxed);

                                                            target_thread->cacheAddedCount.fetch_add(temp_delete_count, std::memory_order_relaxed);

                                                            atomic_cache.store(false, std::memory_order_relaxed);
                                                            */

                                                        }
                                                        else {
                                                            should_delete = true;
                                                            //printf("fail;"); // assert
                                                        }
                                                    }


                                                    /*
                                                    bool inserted = false;
                                                    for (int t = 0; t < threads_size; t++) {

                                                        auto &target_thread = this->threadPool->threads[next_add_thread_index];
                                                        bool have_capacity = target_thread->cacheAddedCount - target_thread->cacheUsedCount < 40000;

                                                        if (have_capacity) {
                                                            auto &atomic_cache = shared_dispose_data.atomic_for_cache[next_add_thread_index];
                                                            if (atomic_cache.compare_exchange_weak(is_false, true)) {

                                                                auto *end = target_thread->cacheRunNodeEnd.load();
                                                                node->next.store(nullptr, std::memory_order_relaxed);
                                                                target_thread->cacheRunNodeEnd.store(node, std::memory_order_relaxed);
                                                                end->next.store(firstNode, std::memory_order_relaxed);

                                                                target_thread->cacheAddedCount.fetch_add(temp_delete_count, std::memory_order_relaxed);

                                                                atomic_cache.store(false, std::memory_order_relaxed);
                                                                inserted = true;
                                                                break;
                                                            }
                                                            else {
                                                                //printf("fail;"); // assert
                                                            }
                                                        }

                                                        next_add_thread_index++;
                                                        if (next_add_thread_index >= threads_size) {
                                                            next_add_thread_index = 0;
                                                        }
                                                    }
                                                    */



                                                    if (should_delete) {
                                                        auto *deleteNode = firstNode;
                                                        if (total_delet_count % 20 == 0) {

                                                            /*
                                                            if (target_thread->cacheAddedCount > INT64_MAX) {
                                                                printf("==;");
                                                            }
                                                                                                                          */
                                                            //printf("del: %d/n", temp_delete_count);

                                                        }
                                                        //printf("here;%llu - %llu", this->threadPool->threads[next_add_thread_index]->cacheAddedCount.load(), this->threadPool->threads[next_add_thread_index]->cacheUsedCount.load());

                                                        while (deleteNode != node) {
                                                            auto * delete_temp = deleteNode;
                                                            deleteNode = deleteNode->next;// .load();
                                                            delete delete_temp;
                                                        }
                                                        delete node;
                                                    }
                                                    else {

                                                    }

                                                    firstNode = next;
                                                    next_add_thread_index++;
                                                    if (next_add_thread_index >= threads_size) {
                                                        next_add_thread_index = 0;
                                                    }
                                                    temp_delete_count = 0;

                                                }

                                                if (shouldBreak) {
                                                    break;
                                                }

                                                node = next;

                                            }

                                            req.retryCount = 0;
                                            req.targetDoingRunNode = nullptr;
                                            req.requested = false;


                                        }
                                    }
                                    req.disposed.store(false, std::memory_order_relaxed);
                                }
                            }
                        }

                        if (doneDelete == false) {
                            req.retryCount++;
                        }
                    }
                }
            }
        };

        auto gfunc_consume = [this, &dispose]() {
            auto &thq = this->threadPool->thread_queue;
            //int queue_info_list[thread_queue_size];

            /*
            this->prev_idx++;
            if (this->prev_idx < thread_queue_size == false) {
                this->prev_idx = 0;
            }
            */
            while (true) {
                auto k = this->prev_idx;

                for (int km = 0; km < thread_queue_size; km++, k++) {
                    if (k < thread_queue_size == false) {
                        k = 0;
                    }

                    auto *thread_queue = thq[k];
                    auto *doingRunNode = POINTER_LOAD(thread_queue->doingRunNode);// .load(std::memory_order_acquire);
                    auto *doingNextNode = POINTER_LOAD(doingRunNode->next);// .load(std::memory_order_acquire);

                    if (doingNextNode != nullptr) {
                        auto currentDoingIndex = thread_queue->currentDoingIndex.load(std::memory_order_relaxed);
                        NodeIndexInteger nextValue = currentDoingIndex + (NodeIndexInteger)1;

                        if (doingNextNode->nodeIndex == nextValue) {
                            auto funcId = doingNextNode->func_id;
                            auto *parameters = doingNextNode->parameters;

                            auto result = thread_queue->currentDoingIndex.compare_exchange_strong(currentDoingIndex, nextValue);
                            if (result) {
                                thread_queue->doingRunNode = doingNextNode;// , std::memory_order_release);
                                this->set_count--;
                                /*
                                for (int j = 0; j < 1; j++) {
                                    geeze[this->thread_index].fetch_add(1);
                                }
                                */

                                //printf("[[%d]]", thread_queue->currentDoingIndex.load());

                                auto *end = this->cacheRunNodeEnd;
                                POINTER_STORE(doingRunNode->next, nullptr);
                                this->cacheRunNodeEnd = doingRunNode;
                                //end->next.store(doingRunNode, std::memory_order_relaxed);
                                POINTER_STORE(doingRunNode->next, nullptr);
                                POINTER_STORE(end->next, doingRunNode);

                                //this->cacheAddedCount.fetch_add(1, std::memory_order_relaxed);
                                this->cacheAddedCount = (this->cacheAddedCount + 1LLU);// , std::memory_order_relaxed);

                                return false;
                            }
                        }
                        else {
                            /*
                            if (this->set_count % 100000 == 5) {
                                printf(";");
                            }
                            */

                        }
                    }
                    else {
                        //this->set_count++;

                        //queue_info_list[k] = 532;
                    }
                }
                //				std::this_thread::sleep_for(std::chrono::milliseconds(1));

                return false;
            }

            printf("failed!!!");
            return false;
        };


        while (true) {

            uint64_t i = 0;
            uint64_t j = 0;

            {
                /*
                for (int m = 0; m < 10000000; m++) {
                    //gfunc(m);
                }
                */
            }

            while (true) {
                if (this->stop) {
                    break;
                }
                //this->set_count++;
                //this->addRunNode(nullptr);

                i++;
                //gfunc();
                if (i % 2 == 0) {
                    j++;
                    gfunc(j);
                }
                else {
                    gfunc_consume();
                }
                continue;

                for (int i = 0; i < 1000 * 1000; i++) {
                    auto *rr = new RunNode();//[1000*1000];
                    //this->addRunNode(rr);
                    //this->addRunNode(&this->rr[i]);
                    //rr->next = nullptr;

                }
            }

            if (this->stop) {
                break;
            }

            /*
            std::unique_lock<std::mutex> lock{ this->main_sleeper_mutex };
            auto notified = this->main_sleeper_cond.wait_for(
                    lock, std::chrono::microseconds(1000 * 1), [this] {
                    return main_sleeper_ready == true;
                    }
                    );
                    */
                    //printf("sec %d,", this->idx);
                    //			fflush(stdout);
        }
    });
    //tha->detach();

};


int GreenThread::init(int idx3) {
    this->cacheRunNodeFirst = new RunNode();
    this->cacheRunNodeEnd = new RunNode();
    this->cacheRunNodeFirst->next = this->cacheRunNodeEnd;// .load();

    for (int i = 0; i < 8000; i++) {
        auto * runNode = new RunNode();

        auto *end = this->cacheRunNodeEnd;// .load();
        runNode->next = nullptr;// , std::memory_order_relaxed);
        this->cacheRunNodeEnd = runNode;// , std::memory_order_relaxed);
        end->next = runNode;// , std::memory_order_relaxed);

        this->cacheAddedCount = this->cacheAddedCount + 1;// , std::memory_order_relaxed);
    }


    this->cacheAddedCount = 2;
    this->cacheUsedCount = 0;

    this->idx = idx3;
    this->thread_index = idx3;

    //this->rr = new RunNode[1000*1000];

    return 0;
};





int ThreadPool::init() {
    for (int i = 0; i < thread_queue_size; i++) {
        this->thread_queue[i] = new ThreadQueue();

        this->thread_queue[i]->doingRunNode =
            this->thread_queue[i]->firstRunNode =
            this->thread_queue[i]->lastRunNode =
            new RunNode();

        //RunNode * forNode = new RunNode();
        //this->addRunNode(forNode);
    }


    std::atomic<int> *atomic_add = nullptr;

    //atomic_add = new std::atomic<int>{ 24 };

    for (int i = 0; i < threads_size; i++) {
        GreenThread *gt = new GreenThread();
        gt->threadPool = this;
        gt->init(i);
        this->threads.push_back(gt);
    }

    return 0;
};

int ThreadPool::start() {
    for (int i = 0; i < this->threads.size(); i++) {
        /*
        if (i < consuming_threads_size) {
            this->threads[i]->startConsuming();
        }
        else {
            this->threads[i]->start();

        }
        */
        this->threads[i]->start();

    }

    return 5;
};
