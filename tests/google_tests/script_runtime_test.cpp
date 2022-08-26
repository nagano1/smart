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
        EXPECT_NE(env->context->memBufferForMalloc.firstBufferBlock, env->context->memBufferForMalloc.currentBufferBlock);

        ScriptEnv::deleteScriptEnv(env);

        ENDTEST
    }

    TEST(ScriptEngine, StackMemory_PushPop) {
        ScriptEnv* env = ScriptEnv::newScriptEnv();

        auto& stackMemory = env->context->stackMemory;
        auto* stackPointer1 = stackMemory.stackPointer;

        stackMemory.push(5);
        stackMemory.push(6);
        
        EXPECT_NE(stackPointer1, stackMemory.stackPointer);

        EXPECT_EQ(6, stackMemory.pop());
        EXPECT_EQ(5, stackMemory.pop());

        EXPECT_EQ(stackPointer1, stackMemory.stackPointer);

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
        stackMemory.moveTo(-stackMemory.baseBytes, 100);
        EXPECT_EQ(100, stackMemory.moveFrom(-stackMemory.baseBytes));
        stackMemory.ret();

        EXPECT_EQ(basePointer0, stackMemory.stackBasePointer);
        EXPECT_EQ(stackPointer0, stackMemory.stackPointer);

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

        EXPECT_EQ(basePointer1, stackMemory.stackBasePointer);
        EXPECT_EQ(stackPointer1, stackMemory.stackPointer);

        stackMemory.ret();
        // returned from func1

        EXPECT_EQ(basePointer0, stackMemory.stackBasePointer);
        EXPECT_EQ(stackPointer0, stackMemory.stackPointer);

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

    TEST(ScriptEngine, StackMemoryTest3_overflow_localVariables) {
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
    let a = 0
    int b = 0
    print("test日本語", 3 + 56)
    
    return (4 + 5)
}
)";
        int ret = ScriptEnv::startScript((char*)source, sizeof(source)-1);
        EXPECT_EQ(ret, 9);

        ENDTEST
    }
}