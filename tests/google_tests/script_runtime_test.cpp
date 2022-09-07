#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <atomic>
#include <condition_variable>
#include <unordered_map>


#include "code_nodes.hpp"
#include "parse_util.hpp"
#include "../test_common.h"
#include "script_runtime/script_runtime.hpp"

namespace smart {

#if defined(_X86_) and defined(_WIN32)
    TEST(ScriptEngine, register_test)
    {
        uint32_t num = UINT32_MAX;
        unsigned short x = 0;
        unsigned short y = 0;

        __asm
        {
            mov eax, num;
            mov ax, x;
            mov num, eax;
        }
        // mov esp, eax;
        //EXPECT_EQ(x, 65535);
        EXPECT_EQ(num, 0xFFFF0000);
        ENDTEST
    }
#endif


    TEST(ScriptEngine, CPURegister_test1)
    {
        CPURegister reg;

        RAX(&reg) = 0xFFFFFFFFFFFFFFFF;
        EAX(&reg) = 0x0;

        EXPECT_EQ(RAX(&reg), 0xFFFFFFFF00000000);



        EAX(&reg) = UINT32_MAX;
        AX(&reg) = 0;
        
        EXPECT_EQ(EAX(&reg), 0xFFFF0000);

        ENDTEST
    }

    TEST(ScriptEngine, MallocItem_test1)
    {
        ScriptEnv* env = ScriptEnv::newScriptEnv();

        int* mem;
        for (int i = 0; i < 1024; i++) {
            mem = (int*)env->context->mallocItem(sizeof(int));
            *mem = 53;

            if (i % 3 == 2) {
                env->context->freeItem(mem);
            }
        }

        console_log("test");

        EXPECT_EQ(*mem, 53);
        EXPECT_NE(env->context->memBufferForMalloc2.firstBufferBlock, env->context->memBufferForMalloc2.currentBufferBlock);

        ScriptEnv::deleteScriptEnv(env);


    }

    ENDTEST

    TEST(ScriptEngine, StackMemory_PushPop) {
        ScriptEnv* env = ScriptEnv::newScriptEnv();

        auto& stackMemory = env->context->stackMemory;
        auto* stackPointer1 = stackMemory.stackPointer;

        stackMemory.push(5);
        stackMemory.push(6);
        
        EXPECT_NE((uint64_t)stackPointer1, (uint64_t)stackMemory.stackPointer);

        EXPECT_EQ(6, stackMemory.pop());
        EXPECT_EQ(5, stackMemory.pop());

        EXPECT_EQ((uint64_t)stackPointer1, (uint64_t)stackMemory.stackPointer);

        ENDTEST
    }
    TEST(ScriptEngine, StackMemoryTest_Call_Ret) {
        ScriptEnv* env = ScriptEnv::newScriptEnv();

        auto& stackMemory = env->context->stackMemory;
        auto *basePointer0 = stackMemory.stackBasePointer;
        auto *stackPointer0 = stackMemory.stackPointer;

        stackMemory.call();
        stackMemory.localVariables(8 * 4);
        stackMemory.localVariables(8 * 4);
        uint32_t a = 100;
        uint32_t b;
        stackMemory.moveTo(-4, 4, (char*)&a);
        stackMemory.moveFrom(-4, 4, (char*)&b);
        EXPECT_EQ(100, b);
        stackMemory.ret();

        EXPECT_EQ((uint64_t)basePointer0, (uint64_t)stackMemory.stackBasePointer);
        EXPECT_EQ((uint64_t)stackPointer0, (uint64_t)stackMemory.stackPointer);

        ScriptEnv::deleteScriptEnv(env);

        ENDTEST
    }


    TEST(ScriptEngine, StackMemoryTest_cal_ret_2) {
        ScriptEnv* env = ScriptEnv::newScriptEnv();

        auto& stackMemory = env->context->stackMemory;

        auto* basePointer0 = stackMemory.stackBasePointer;
        auto* stackPointer0 = stackMemory.stackPointer;

        // call func1
        stackMemory.call();

        stackMemory.push(8);
        auto* stackPointer1 = stackMemory.stackPointer;
        auto* basePointer1 = stackMemory.stackBasePointer;


        // call func2
        stackMemory.call();

        stackMemory.localVariables(8 * 4);

        stackMemory.ret();
        // retruned from func2

        EXPECT_EQ((uint64_t)basePointer1, (uint64_t)stackMemory.stackBasePointer);
        EXPECT_EQ((uint64_t)stackPointer1, (uint64_t)stackMemory.stackPointer);

        stackMemory.ret();
        // returned from func1

        EXPECT_EQ((uint64_t)basePointer0, (uint64_t)stackMemory.stackBasePointer);
        EXPECT_EQ((uint64_t)stackPointer0, (uint64_t)stackMemory.stackPointer);

        ScriptEnv::deleteScriptEnv(env);
        
        ENDTEST
    }

    TEST(ScriptEngine, StackMemoryTest_Overflow_Push) {
        ScriptEnv* env = ScriptEnv::newScriptEnv();

        auto& stackMemory = env->context->stackMemory;

        for (int i = 0; i < stackMemory.stackSize/stackMemory.baseBytes - 1; i++) {
            stackMemory.push(5);
        }
        EXPECT_EQ(stackMemory.isOverflowed, false);
        stackMemory.push(5);

        EXPECT_EQ(stackMemory.isOverflowed, true);

        ENDTEST
    }

    TEST(ScriptEngine, SktackMemoryTest_overflow_call) {
        ScriptEnv* env = ScriptEnv::newScriptEnv();

        auto& stackMemory = env->context->stackMemory;

        for (int i = 0; i < stackMemory.stackSize / stackMemory.baseBytes - 1; i++) {
            stackMemory.call();
        }
        EXPECT_EQ(stackMemory.isOverflowed, false);
        stackMemory.call();

        EXPECT_EQ(stackMemory.isOverflowed, true);

        ENDTEST
    }

    TEST(ScriptEngine, StackMemoryTest_overflow_localVariables) {
        ScriptEnv* env = ScriptEnv::newScriptEnv();
        auto& stackMemory = env->context->stackMemory;
        
        stackMemory.localVariables(stackMemory.stackSize);
        EXPECT_EQ(stackMemory.isOverflowed, true);

        ENDTEST
    }

    TEST(ScriptEngine, ScriptEngineTestSomeScript0) {

        constexpr char source[] = R"(
fn main()
{
    int b = 5
    int ab = 3
    
    return b + ab
}
)";
        int ret = ScriptEnv::startScript((char*)source, sizeof(source)-1);
        EXPECT_EQ(ret, 8);

        ENDTEST
    }
}