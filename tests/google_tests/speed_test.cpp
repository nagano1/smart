#include <iostream>
#include <algorithm>
#include <iterator>
#include <atomic>
#include <condition_variable>
#include <unordered_map>
#include <functional>

#include "../test_common.h"
//#include "tokenizer.hpp"
#include "../../src/tokenizer.hpp"


class P {
public:
    int a = 50;
    bool isBodyEnd = false;
    P *endBodyNode;
    std::vector<P *> expressionNodes;
};



TEST(SpeedTest, memory_order_relaxed_is_fast) {

    EXPECT_EQ(true, ParseUtil::isIdentifierLetter('a'));


    auto start = std::chrono::high_resolution_clock::now().time_since_epoch();

    uint64_t loopCount = 1000 * 10 * 1000LLU;
    std::atomic<uint64_t> *current = new std::atomic<uint64_t>();
    current->store(9823, std::memory_order_relaxed);

    //long kkk;
    P *p = new P();//uint64_t(33);
    int *a = NULL;
    int *newValue = new int{ 3 };

    for (unsigned long long i = 0; i < loopCount; i++) {
        if (i > 5) {
            current->store(i, std::memory_order_relaxed);
            a = newValue;
            //InterlockedExchangePointerNoFence((PVOID *)&a, newValue);

        }
        else {
            a = NULL;// newValue;

        }
        //current->compare_exchange_weak(loopCount, loopCount+1);

        //p->a = static_cast<int>(i);
        //do{} while(0); //noop
//        kkk = current->load(std::memory_order_relaxed);
    }

    auto elapsed = std::chrono::high_resolution_clock::now().time_since_epoch() - start;
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    auto one_op_nanosec = nanoseconds / static_cast<double>(loopCount);

    if (speed_test) {
        EXPECT_LT(one_op_nanosec, ARM ? 40 : 1.1);
        printf("%f", one_op_nanosec);
    }
    EXPECT_EQ(current->load(), loopCount - 1);
}

ENDTEST


TEST(SpeedTest, try_catch_is_slow) {

    auto start = std::chrono::high_resolution_clock::now();

    uint64_t loopCount = 1000 * 1000LLU;
    int a = 0;

    try {
        for (int i = 0; i < loopCount; i++) {
            a = i;
        }
    }
    catch (...) {
        std::exception_ptr p = std::current_exception();
        printf("std::exception_ptr\n");

    }

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    auto one_op_nanosec = nanoseconds / static_cast<double>(loopCount);

    // we have to use a once
    printf("a = %d", a);

    if (ARM) {
        EXPECT_LT(one_op_nanosec, 15);
    } else {
        EXPECT_GT(one_op_nanosec, 0.02);
        EXPECT_LT(one_op_nanosec, 4);
    }

}

ENDTEST



struct AData {
    int a = 3;
};

TEST(SpeedTest, stack_assign_is_fast) {
    {
        auto start = std::chrono::high_resolution_clock::now();

        const int64_t loopCount = 1000 *     1000LL;
        int current = 0;//int{};
        long long current32 = 0;
        auto *aData = new AData{};
        auto *current2 = &aData->a;
        int lim = loopCount - 15;
        for (int i = 0; i < loopCount; i++) {
            if (i > lim) {
                current = i;
            }
        }

        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();

        auto one_op_nanosec = nanoseconds / static_cast<double>(loopCount);

        GLOG << nanoseconds/1000000.0 << "ms";

        EXPECT_LT(one_op_nanosec, ARM ? 10.0: 2.0);
        EXPECT_EQ(current, loopCount - 1);
        //EXPECT_EQ(*current2, loopCount - 1);
    }
#ifdef  __MYWIN__

    //_mm_pause() takes more than 0.5us
   {
       const auto start = std::chrono::high_resolution_clock::now();

       int64_t max = 1000 * 100LL;
       for (long long i = 0; i < max; i++) {
           _mm_pause();
       }

       auto elapsed = std::chrono::high_resolution_clock::now() - start;
       auto nano_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();

       EXPECT_GT(nano_elapsed, max * 0.5);

       if (os_type == OS_TYPE::Windows) {
           EXPECT_LT(nano_elapsed, max * 90); // 90 nano seconds
       }
       else if (os_type == OS_TYPE::Mac) { // Mac
           EXPECT_LT(nano_elapsed, max * 130);
       }
       else { // linux
           EXPECT_LT(nano_elapsed, max * 90);//16
       }

   }
#endif

}

ENDTEST




std::function<const int(const int)> gfunc;
//int(*gFunc)(, double);


std::function<const int(const int)> Fx(const std::function<const int(const int)> &_fx) {

    gfunc = _fx;
    int x = gfunc(100);
    std::thread thread1(gfunc, 3);
    thread1.join();

    std::cout << "argument function : " << x << std::endl;

    return [=](const int y) -> const int { return x + y; };
}

/*
static int fff_ori(int kl) {
	return kl + 1;
}
*/
auto fff_ori = [](int kl) {
    return kl + 1;
};

typedef int(*handle_character_proc)(int next_character);

static handle_character_proc *state_proc;

static void SetStateProc(handle_character_proc next_proc) {
    state_proc = &next_proc;
}


int handle1(int a) {
    return 521;
}


TEST(SpeedTest, functor) {
    /*
    int var = Fx([&](const int x) {
        int kk = 8;
        return kk;
    })(200);
    */

    //std::cout << "argument function : " << var << std::endl;

    auto gfunc = &fff_ori;
    auto &&gfunc2 = gfunc;
    auto result = (*gfunc2)(3);
    SetStateProc(handle1);
    auto state_proc2 = *state_proc;

    //fff(5);
    //auto fff_ori2 = gfunc;
    //k2 = gfunc2(3);


    //std::function<const int(const int)> &&fff = std::move(fff_ori);

    //int k = state_proc2(3);
    //int k = (*state_proc)(3);
    //fff_ori(5);


    EXPECT_EQ(10, 10);
}

ENDTEST
 
