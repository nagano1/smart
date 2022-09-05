#include <iostream>
#include <algorithm>
#include <iterator>
#include <atomic>
#include <condition_variable>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <random>
#include <vector>
#include <iomanip>
#include <locale>
#include <ctime>
#include <time.h>


#include "../test_common.h"

//#include "code_nodes.hpp"


using namespace std;


class Base {
public:
    ~Base() {}

    void func() { printf("eee"); }

    virtual void funcv() { printf("ccc"); }
};

class Derived : public Base {
public:
    virtual void funcv() {
        printf("GGG");
        Base::funcv();
    }
};

void caller(Base &b) { b.funcv(); }

int main24() {
    Base b;
    //b.func();
    //b.funcv();

    Base *d = new Derived;
    d->funcv();
    //d.funcv();
    return 10;
}


// class which has an assignment operator
class Kitty {
public:
    const char *str;

    Kitty() { str = "Kitty on your lap\n"; }

    //Kitty(const Kitty &obj) { this = obj; }
    Kitty &operator=(const Kitty &other) {
        if (this != &other) {

        }
        return *this;
    }
} g_obj;


/*
auto map = new std::unordered_map<int*, int>();
TEST(cplusplus_test, map_test) {
    int a;

    if (map->count(&a) > 0) {
        throw 1;
    }

    map->insert(std::make_pair(&a, 45));

    EXPECT_EQ(map->size(), 1) << "Map size should be 1";

    map->erase(&a);

    EXPECT_EQ(map->size(), 0) << "Map size should be 1";
}
*/


class LogStream {
    int mo, da, yr;
public:
    std::mutex mtx;

    LogStream(int m, int d, int y) {
        mo = m;
        da = d;
        yr = y;
    }

    friend ostream &operator<<(ostream &os, const LogStream &dt);
};


ostream &operator<<(ostream &os, const LogStream &dt) {
    //std::lock_guard<std::mutex> guard(dt.mtx);
    os << dt.mo << '/' << dt.da << '/' << dt.yr;
    return os;
}

TEST(cplusplus_test, stringstream_) {
    /*
    // Your imaginary buffer
    char    buffer[] = "A large buffer we don't want t\n\no copy but use in a stream";

    // An ordinary stream.
    std::stringstream   str;
    std::stringstream   str2;
    str << "WWOWOWOWO\n\nWowowo";

    // Get the streams buffer object. Reset the actual buffer being used.
    str.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
    //str << "WWOWOWOWO\n\nWowowo";

    std::copy(std::istreambuf_iterator<char>(str),
        std::istreambuf_iterator<char>(),
        std::ostream_iterator<char>(std::cout)
    );

    std::cout << "jfiowe\n\njfoiaw";
    */


    std::stringstream log;

    bool stopped = false;
    std::mutex mtx;

    auto &sstream = log;
    auto worker_thread = std::thread([&]() {
        std::vector<int> v;
        while (!stopped) {
            //printf(";");
//			std::this_thread::sleep_for(std::chrono::microseconds(1000));
            {
                std::lock_guard<std::mutex> lg{ mtx };
                std::copy(std::istreambuf_iterator<char>(sstream),
                    std::istreambuf_iterator<char>(),
                    std::ostream_iterator<char>(std::cout)
                );
            }
        }
    });

    for (int i = 0; i < 10; i++) {
        std::lock_guard<std::mutex> lg{ mtx };

        log << u8"日本語a";
        log << "C";

        // +i % 25);
        // std::this_thread::sleep_for(std::chrono::microseconds(2000));
    }


    //log << std::feof(nullptr);
    log << '\0';
    stopped = true;
    worker_thread.join();

    EXPECT_EQ(100, 100);
}

ENDTEST

/*
#define NANOS_IN_SECOND 1000000000
static long currentTimeInMicros() {
    //struct timeval tv;
    //gettimeofday (&tv, NULL);
    //return tv.tv_sec*1000000+ tv.tv_usec;

    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return (res.tv_sec * NANOS_IN_SECOND) + res.tv_nsec;
}
*/


TEST(cplusplus_test, test1) {
    {
        char a = -122;
        unsigned char b = a;
#if defined(__aarch64__) && defined(__ANDROID__)
       EXPECT_TRUE((int)a == (int)b);
#else
        EXPECT_TRUE((int)a != (int)b);
#endif

        EXPECT_FALSE(3 == 5);
    }


    //auto ret = TestUtil::testUtf8Text();

    //EXPECT_EQ(5, ret);

    //arrange
    //act
    //assert
    //EXPECT_EQ(1, 1) << "Vectors x and y are of unequal length";;

    //ASSERT_TRUE(true);

    EXPECT_EQ(20, 20);
    EXPECT_EQ(100, 100);

    /*
        EXPECT_EQ(Formula::bla(0), 0);
        EXPECT_EQ(Formula::bla(10), 20);
        EXPECT_EQ(Formula::bla(50), 100);
     */
}

ENDTEST

/*
std::atomic<unsigned long long> head3{ 2845 };


static std::mutex main_sleeper_mutex;

std::atomic<bool> maybe_sleeping{ false };
std::atomic<int> sleep_set_index{ -1 };
 */


void printFoo(std::ostream &os) {
    os << "Successful" << std::endl;
}


struct A {

};

#ifdef __ANDROID__

#ifdef __aarch64__
static long add(long i, long j);
#endif

TEST(cplusplus_test, ArmAssemblyTest2) {
    #ifdef __aarch64__

    int a = 1;
    int b = 2;
    int c = 0;

    c = add(a, b);

    printf("Result of %d + %d = %d\n", a, b, c);
    EXPECT_EQ(c, 3);
    EXPECT_EQ(sysconf(_SC_NPROCESSORS_CONF), 8);
    EXPECT_EQ(sysconf(_SC_NPROCESSORS_ONLN), 8);

    //register void *sp asm("sp");
    // void* p2 = NULL;


    //EXPECT_EQ(sp, (void*)&p2);
    #endif
}
#ifdef __aarch64__

static long add(long i, long j)
{
    long res = 0;
    __asm ("ADD %[result], %[input_i], %[input_j]"
    : [result] "=r" (res) : [input_i] "r" (i), [input_j] "r" (j)
    );

    //int address = 0;
    /*
     *
    __asm ("STR sp, [%0]\n\t"
    : "=r" ( address)
    );
     */
     /*
         __asm ("mov %0, sp\\n\\t"
         : "=r" (address)
         );
      */


    //long a = 324;
    //EXPECT_EQ(address, 3);

    return res;
}
#endif

ENDTEST

#endif


TEST(cplusplus_test, printFoo) {
    
    int sleepings = 0;
    sleepings = sleepings | (1 << 0);
    EXPECT_EQ(sleepings, 1);

    sleepings = sleepings | (1 << 0);
    sleepings = sleepings | (1 << 1);

    EXPECT_EQ(sleepings, 3);

    sleepings = sleepings & ~(1 << 0);
    sleepings = sleepings & ~(1 << 1);

    EXPECT_EQ(sleepings, 0);

    std::ostringstream output;
    printFoo(output);

    for (int ki = 0; ki < 100; ki++) {
        //std::this_thread::sleep_for(std::chrono::milliseconds{1});
        //GLOG << std::endl;

        for (int i = 0; i < 100; i++) {
            //std::this_thread::sleep_for(std::chrono::milliseconds{8});
            //auto *ains = new A();
            //delete ains;

            //GLOG << i << ",";
        }
    }
    // Not that familiar with gtest, but I think this is how you test they are
    // equal. Not sure if it will work with stringstream.
    //EXPECT_EQ(output.str(), "Successful\n");


    GLOG << "awfwe 23";// << std::endl;

    printf("\nfwe\n");
    return;

    // For reference, this is the equivalent assert in mstest
    // Assert::IsTrue( output == "Successful" );
}

ENDTEST



template<class T>
std::string FormatWithCommas(T value)
{
   auto s = std::to_string(value);
   int n = s.length() - 3;
   while (n > 0) {
      s.insert(n, ",");
      n -= 3;
   }

   return s;
}


TEST(cplusplus_test, ThreadQueue) {
    GLOG << "started: ";// << std::fflush;

    auto start = std::chrono::high_resolution_clock::now().time_since_epoch();

    ThreadPool tp;
    tp.init();
    tp.start();


    std::thread thread1([]() {

    });

    std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });

    thread1.join();

    auto elapsed = std::chrono::high_resolution_clock::now().time_since_epoch() - start;

    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    auto diff = microseconds * 1000;


    for (int i = 0; i < threads_size; i++) {
        tp.threads[i]->doStop();
    }

    for (int i = 0; i < threads_size; i++) {
        tp.threads[i]->join();
    }


    GLOG << diff << "diff!\n" << "\n";

    long long count = 0;
    for (int i = 0; i < threads_size; i++) {
        count += tp.threads[i]->set_count;
        std::cout << i << ": " << tp.threads[i]->set_count << ": \n";
    }

    long long count_b = 0;
    {
        for (int i = 0; i < thread_queue_size; i++) {
            GLOG << " \n";

            //count_b += tp.thread_queue[i]->lastRunNode.load()->idx;

            auto *firstNode = POINTER_LOAD(tp.thread_queue[i]->doingRunNode);
            auto *lastNode = POINTER_LOAD(tp.thread_queue[i]->lastRunNode);

            GLOG << "queue lastNode->nodeIndex = " << FormatWithCommas(tp.thread_queue[i]->lastNodeIndex) << std::endl;


            int count_temp = -1;
            while (firstNode) {
                count_temp++;
                //printf("%d,", leftNode->nodeIndex.load());
                if (firstNode == lastNode) {
                    GLOG << "lastNode->nodeIndex = " << lastNode->nodeIndex << std::endl;
                    break;
                }
                firstNode = firstNode->next;
            }

            count_b += count_temp;
            //		std::cout << i << ":: " << tp.thread_queue[i]->lastRunNode.load()->idx << "\n";// << aobj->gf;
        }

        GLOG << count << ": count by each thread: " << " " << "\n";
        GLOG << count_b << ": last node counter: " << "   !\n" << "\n";
        GLOG << diff << "diff         !\n" << "\n";// << aobj->gf;
    }

    EXPECT_EQ(count, count_b);
    //TestUtil::testUtf8Text();


    //std::cout << "\set_count = " << set_count_b << "\n";
    //std::cout << "\nconflict = " << conflict << "\n";
    //std::cout << "\nconflict2 = " << conflict2 << "\n";

    //_getchar_nolock();

    if (managed_cmake_test == false) {
        EXPECT_EQ("", "Fail to see log");
    }
}

ENDTEST










std::atomic<unsigned int> sleepings{ 0 };
bool stopped2 = false;


//
// measure time of waking up slept threads
//
TEST(concept, wakeup_test) {

    constexpr int sleeping_thread_size = 2;

    static unsigned long long wakeups[sleeping_thread_size];
    std::atomic<int> side[sleeping_thread_size];

    static std::condition_variable_any main_sleeper_cond;
    sleepings = 0;
    stopped2 = false;


    std::ostringstream output_stream;

    std::vector<std::thread *> worker_threads;

    for (int s = 0; s < sleeping_thread_size; s++) {

        side[s].store(0);

        wakeups[s] = 0;

        auto worker_thread = new std::thread([](int index) {
            std::mutex this_mtx;
            std::unique_lock<std::mutex> sleeper_lock{ this_mtx };

            while (true) {
                bool first = true;
                while (true) {
                    auto l = sleepings.load();
                    if (sleepings.compare_exchange_weak(l, l | (1 << index))) {
                        break;
                    }
                    else {
                        if (first == true) {
                            first = false;
                            //printf("[");
                        }
                        //printf("w-%d,",index);
                    }
                }

                if (stopped2) {
                    break;
                }

                //auto notified =
                main_sleeper_cond.wait_for(sleeper_lock,
                    std::chrono::milliseconds(1));

                {
                    bool first2 = true;
                    while (true) {
                        auto l = sleepings.load();
                        if (sleepings.compare_exchange_weak(l, l & ~(1 << index))) {
                            break;
                        }
                        else {
                            if (first2 == true) {
                                first2 = false;
                                //printf("[");
                            }
                            //printf("p-%d,", index);
                        }
                    }
                }

                //wakeupCount++;
                wakeups[index]++;
                /*
                for (int j = 0; j < 1500; j++) {
                    side[index].store(3335);
                    //int *ml = new int{0};
                    //delete ml;
                    //throw 3;
                }
                */
                //printf("%d,", sleepings.load());

                //std::this_thread::sleep_for(std::chrono::milliseconds(4));
            }
        }, s);

        //worker_thread->detach();
        worker_threads.push_back(worker_thread);
    }

    std::this_thread::sleep_for(std::chrono::microseconds(7000));

    auto start = std::chrono::high_resolution_clock::now();

    unsigned long long k = 0;
    using newtype = unsigned int;
    newtype loopCount = ARM ? 1000 * 70 : 1000 * 100;

    auto current = int{};

    for (newtype i = 0; i < loopCount; i++) {
        // avoid memory intense access
        if /*constexpr*/ (ARM) {
            for (int p = 0; p < 2050; p++) {
                current = p;
            }
        }
        // atomic<>.load is relatively slow on ARM
        if (sleepings.load(std::memory_order_relaxed) == 0) {
            //k = i;
            k++;
            //tryWakeupCount++;
        }
        else {
            //printf("%d,", sleepings.load());
            //main_sleeper_cond.notify_one();
            main_sleeper_cond.notify_all();
        }
    }

    stopped2 = true;


    //std::chrono::system_clock::now();
    auto elapsed = std::chrono::high_resolution_clock::now() - start;

    auto nanoseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    for (int i = 0; i < sleeping_thread_size; i++) {
        worker_threads[i]->join();
    }

    uint64_t totalWokeupCount = 0;
    for (int i = 0; i < sleeping_thread_size; i++) {
        totalWokeupCount += wakeups[i];
    }


    std::ostringstream output2;
    //output2 << "[error here]" << current << std::endl;
    printf("k = %llu \n", k);
    auto one_op_nanosec = nanoseconds / static_cast<float>(loopCount);
    auto wokeupCountPerMillisecond = (double)totalWokeupCount / (nanoseconds);

    if (speed_test && !EMULATOR) { // thread switching is extremely slow on emulators
        EXPECT_GT(wokeupCountPerMillisecond, ARM ? 10.0f : 5.0f);
        printf("wokeupCountPerMillisecond = %f\n", wokeupCountPerMillisecond);
    }


    GLOG << "wokeupCountPerMillisecond = " << wokeupCountPerMillisecond;
    if (speed_test) {
        EXPECT_LT(one_op_nanosec, 43.0f);
    }
    //EXPECT_LT(k, loopCount * 20);

    //EXPECT_LT(totalWokeupCount, tryWakeupCount);
    //EXPECT_LT(totalWokeupCount, 3000);
    //FAIL() << output2.str() << ":" << milliseconds << "ms" << "," << k;

}

ENDTEST




#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
CONDITION_VARIABLE ConditionVar;
CRITICAL_SECTION CritSection;



//スレッド関数
DWORD WINAPI ThreadFunc(LPVOID arg)
{
    Sleep(200);

    WakeConditionVariable(&ConditionVar);

    int i;
    for (i = 0; i < 100; i++) {
        //printf("ThreadFunc %d\n", i);
        //Sleep(50);
    }
    return 0;
}

#endif



TEST(concept, add_consume_test_win32) {

    //#ifdef WIN32
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    //InterlockedCompareExchange64;
    //InitializeConditionVariable;

    HANDLE h;

    //HANDLE sh = CreateEvent(NULL, TRUE, FALSE, TEXT("EVENT"));

    //対象のイベントオブジェクトを取得
    //h = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("EVENT"));
    //シグナル状態になるまで待つ。
    //WaitForSingleObject(h, 1000/*INFINITE*/);
    //非シグナル状態にする。
    //ResetEvent(h);



    InitializeConditionVariable(&ConditionVar);
    InitializeCriticalSection(&CritSection);


    HANDLE hThread;
    DWORD dwThreadId;
    int64_t aval = 24;

    // run thread
    hThread = CreateThread(
        NULL, // security attributes
        100, // stack size
        ThreadFunc, // function to invoke on the thread
        NULL, // arguments
        0, // create option (0 or CREATE_SUSPENDED)
        &dwThreadId);// thread id


    if (hThread > 0) {
        // We are not gonna use this
        //int *a = NULL;
        //int *newValue = new int{ 3 };
        //InterlockedExchangePointerNoFence((PVOID *)&a, newValue);


        // CAS
        int64_t newValue = 51234;
        auto metValue = InterlockedCompareExchange64(&aval, newValue, 24);
        EXPECT_EQ(metValue, 24);
        if (metValue == 24) {
            // succeed
            EXPECT_EQ(aval, newValue);
        }
        else {
            // failed, but a could be already equivalent to newValue thanks to other thread.
        }



        SuspendThread(hThread);
        Sleep(2);
        ResumeThread(hThread);

        EnterCriticalSection(&CritSection);
        SleepConditionVariableCS(&ConditionVar, &CritSection, INFINITE/*100*/);
        LeaveCriticalSection(&CritSection);




        WaitForSingleObject(hThread, 10000);

        CloseHandle(hThread);
    }
    //FAIL();
#endif
}

ENDTEST



TEST(concept, i_have_32threads_cpu) {
    std::atomic<bool> atomicvalue{ true };
    if (atomicvalue) {
        // FAIL();
    }
}
ENDTEST



TEST(concept, add_consume_test) {

    // Let's measure time for two threads to communicate via a work queue


    using newtype = unsigned int;

    static int work_node_max = ARM ? 2050 * 2 : 1800 * 2;

    static std::atomic<bool> wow2{ false };

    static constexpr int SET_VALUE = 11;

    newtype loopCount = work_node_max;// ARM ? 1000 * 70 : 1000 * 100;
    newtype outLoop = ARM ? 1000 * 1 : 100 * 1;
    static int total_i = 0;
    total_i = 0;

    static long long total_nano_sec = 0;
    total_nano_sec = 0;

    static long long total_calc = 0;
    total_calc = 0;



    //#pragma pack(8)
    struct WorkItem {
        std::atomic<WorkItem*> next{ nullptr };

        // incremental flag to deal with cache
        uint64_t serverRevision = 0;
        uint64_t clientRevision = 0;

        //uint64_t serverLocalId = 10;

        //uint32_t sessionIndex = 0;
        std::atomic<int> finished{ -1 };

        /*
        int value16;
        int value17;
        int value18;
        int value19;
        int value20;
        int value21;
        int value22;
        int value23;
        int value24;
        int value25;
        int value26;
        int value27;
        int value28;
        int value29;
        int value30;
        int value31;
        */


        char value0;
        char value1;
        char value2;
        char value3;
        char value4;
        char value5;
        char value6;
        char value7;
        char value8;
        char value9;
        char value10;
        char value11;
        char value12;
        char value13;
        char value14;
        char value15;


        /*
        int val2[8];
        int val32[16];
        int val42[16];
        int val52[16];
        uint16_t arr[16];
        uint16_t arr0[16];
        uint16_t arr1[16];
        uint16_t arr2[16];
*/
    };



    auto *cacheVec = new std::vector<WorkItem *>{};
    {
        for (int i = 0; i < work_node_max; i++) {
            auto *newLastItem = new WorkItem();
            cacheVec->push_back(newLastItem);
        }
    }



    for (int ntry = 0; ntry < outLoop; ntry++) {

        for (int jj = 0; jj < 1; jj++) {

            for (int k = 0; k < cacheVec->size(); k++) {
                auto &&aa = cacheVec->at(k);
                //aa->sessionIndex = 3;

                const int gk = -3 - jj;
                aa->value15 = gk + 8;
                aa->value0 = gk + 8;
                aa->value1 = gk + 8;
                aa->value2 = gk + 8;
                aa->value3 = gk + 8;
                aa->value4 = gk + 8;
                aa->value5 = gk + 8;
                aa->value6 = gk + 8;
                aa->value7 = gk + 8;
                aa->value8 = gk + 8;
                aa->value9 = gk + 8;
                aa->value10 = gk + 8;
                aa->value11 = gk + 8;
                aa->value12 = gk + 8;
                aa->value13 = gk + 8;
                aa->value14 = gk + 8;
            };
        }


        WorkItem *firstItem = new WorkItem{};


        std::mt19937 get_rand_mt;
        //std::shuffle(cacheVec->begin(), cacheVec->end(), get_rand_mt);

        std::atomic_thread_fence(std::memory_order_release);

        auto add_thread = std::thread(
            [](int index, WorkItem *firstItem, std::vector<WorkItem *>* cacheVec) {


            WorkItem *currentItem = firstItem;

            // Add work queues
            for (auto&&item : *cacheVec) {
                item->next.store(nullptr, std::memory_order_relaxed);
                item->finished.store(-1, std::memory_order_relaxed);
                item->serverRevision++;

                std::atomic_thread_fence(std::memory_order_release);

                currentItem->next.store(item, std::memory_order_relaxed);

                std::atomic_thread_fence(std::memory_order_release);



                constexpr int k = 3;
                item->value0 = k + 8;
                item->value1 = k + 8;
                item->value2 = k + 8;
                item->value3 = k + 8;
                item->value4 = k + 8;
                item->value5 = k + 8;
                item->value6 = k + 8;
                item->value7 = k + 8;
                std::atomic_thread_fence(std::memory_order_release);
                item->value8 = k + 8;
                item->value9 = k + 8;
                item->value10 = k + 8;
                item->value11 = k + 8;
                item->value12 = k + 8;
                item->value13 = k + 8;
                item->value14 = k + 8;

                std::atomic_thread_fence(std::memory_order_release);

                item->value15 = k + 8;

                currentItem = item;
            }
            currentItem->finished.store(15, std::memory_order_relaxed);

            std::atomic_thread_fence(std::memory_order_release);



        }, 3, firstItem, cacheVec);

        // add_thread.join();

        // std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });


        auto consume_thread = std::thread([](int index, WorkItem *firstItem) {

            auto start = std::chrono::high_resolution_clock::now();

            int i_sum = 0;
            auto *targetItem = firstItem;
            int i = 0;
            while (true) {
                std::atomic_thread_fence(std::memory_order_acquire);

                auto *tempNext = targetItem->next.load(std::memory_order_relaxed);
                if (tempNext != nullptr
                    //&& tempNext->clientRevision+1 == tempNext->serverRevision
                    ) {

                    if (tempNext->clientRevision + 1 != tempNext->serverRevision)
                    {
                        FAIL();
                    }

                    i_sum++;
                    targetItem = tempNext;
                    targetItem->clientRevision++;


                    if (targetItem->value15 == SET_VALUE) {

                        std::atomic_thread_fence(std::memory_order_acquire);

                        if (
                            targetItem->value0 == SET_VALUE
                            && targetItem->value5 == SET_VALUE
                            && targetItem->value6 == SET_VALUE
                            && targetItem->value7 == SET_VALUE
                            && targetItem->value8 == SET_VALUE
                            && targetItem->value9 == SET_VALUE
                            && targetItem->value1 == SET_VALUE
                            && targetItem->value2 == SET_VALUE
                            && targetItem->value3 == SET_VALUE
                            && targetItem->value4 == SET_VALUE
                            && targetItem->value10 == SET_VALUE
                            && targetItem->value11 == SET_VALUE
                            && targetItem->value12 == SET_VALUE
                            && targetItem->value13 == SET_VALUE
                            && targetItem->value14 == SET_VALUE
                            /*
                            && targetItem->value16 == SET_VALUE
                            && targetItem->value17 == SET_VALUE
                            && targetItem->value18 == SET_VALUE
                            && targetItem->value19 == SET_VALUE
                            && targetItem->value20 == SET_VALUE
                            && targetItem->value21 == SET_VALUE
                            && targetItem->value22 == SET_VALUE
                            && targetItem->value23 == SET_VALUE
                            && targetItem->value24 == SET_VALUE
                            && targetItem->value25 == SET_VALUE
                            && targetItem->value26 == SET_VALUE
                            && targetItem->value27 == SET_VALUE
                            && targetItem->value28 == SET_VALUE
                            && targetItem->value29 == SET_VALUE
                            && targetItem->value30 == SET_VALUE
                            */

                            )
                        {

                        }
                        else {
                            //FAIL();
                        }
                    }


                    if (targetItem->value15 != SET_VALUE) {
                        i++;

                        {
                            int try_n = 0;
                            volatile int vi = 0;
                            while (true) {
                                vi++;

                                if (ARM) {
                                    wow2.store(!wow2.load(), std::memory_order_relaxed);
                                }
                                else {
                                    do_pause_or_nothing();
                                }

                                if (SET_VALUE == targetItem->value15) {
                                    i--;
                                    break;
                                }

                                try_n++;
                                if (try_n > 300) {
                                    int try_n2 = 0;
                                    while (true) {
                                        std::this_thread::sleep_for(
                                            std::chrono::milliseconds{ 0 }
                                        );
                                        if (SET_VALUE == targetItem->value15) {
                                            i--;
                                            break;
                                        }

                                        try_n2++;

                                        if (try_n2 > 9) {
                                            while (true) {
                                                if (SET_VALUE == targetItem->value15) {
                                                    i--;
                                                    break;
                                                }
                                                std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
                                            }

                                            break;
                                        }
                                    }

                                    break;
                                }
                            }
                        }
                    }
                }
                else {
                    if (targetItem->finished.load(std::memory_order_relaxed) == 15) {
                        break;
                    }
                    else {
                        wow2.store(!wow2.load(), std::memory_order_relaxed);
                    }

                }
            }

            total_i += i;



            auto elapsed = std::chrono::high_resolution_clock::now() - start;
            auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
            total_nano_sec += nanoseconds;

            total_calc += i_sum;

        }, 3, firstItem);

        add_thread.join();
        consume_thread.join();

    }

    // dispose
    for (auto&&item : *cacheVec) {
        delete item;
    }
    delete cacheVec;

    auto one_op_nanosec = total_nano_sec / static_cast<float>(loopCount*outLoop * 16);

    EXPECT_EQ(total_i, 0);
    EXPECT_EQ(total_calc, outLoop*loopCount);

    if (speed_test) {
        EXPECT_LT(one_op_nanosec, ARM ? 35 : 1);
    }


    std::cout << one_op_nanosec << ": nanosec-----------------------------";

    if (managed_cmake_test == false) {
        EXPECT_EQ("", "Fail to see log");
    }
}

ENDTEST


auto arr = new int[1024];

//static int max_retry = 1000 * 10000;


//static bool finished = false;

TEST(concept, unordered_map_test) {

    auto map = new std::unordered_map<int, int>();
    //map->reserve(1000 * 1000);
    int loopCount = 1000 * 10;
    for (int i = 0; i < loopCount; i++) {
        map->insert(std::make_pair(i, i + 1));
    }

    auto start = std::chrono::high_resolution_clock::now();

    int result = 0;
    for (int i = 0; i < loopCount; i++) {
        result = map->at(i);
    }

    auto elapsed = std::chrono::high_resolution_clock::now() - start;

    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();

    auto one_op_nanosec = nanoseconds / static_cast<float>(loopCount);

    if (speed_test) {
        EXPECT_LT(one_op_nanosec, 100);
        printf("%f", one_op_nanosec);
    }
    EXPECT_EQ(result, loopCount);

}

ENDTEST



