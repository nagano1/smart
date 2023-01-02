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
    TEST(ScriptEngine, register_test2)
    {
        uint32_t num = 0;
        __asm
        {
            mov ecx, 10; ecx = 10
            mov eax, ecx; eax = ecx
            add eax, ecx; eax = eax + ecx
            mov num, eax;
        }
        EXPECT_EQ(num, 20);
        ENDTEST
    }

    TEST(ScriptEngine, register_test)
    {
        uint32_t num = UINT32_MAX;
        unsigned short x = 0;
        unsigned short y = 0;

        __asm
        {
            mov eax, 0xFFFFFFFF;
            mov ax, 0;
            mov num, eax;
        }
        // mov esp, eax;
        EXPECT_EQ(num, 0xFFFF0000);
        EXPECT_EQ(1, 2);
        ENDTEST
    }
#endif



    TEST(ScriptEngine, CPURegister_test1)
    {
        CPURegister reg{};
        EXPECT_EQ(EAX(&reg), 0);
        EXPECT_EQ(sizeof(reg.rax), 8);
        EXPECT_EQ(sizeof(bool), 1);

        RAX(&reg) = 0xFFFFFFFFFFFFFFFF;
        AX(&reg) = 0;
        EXPECT_EQ(RAX(&reg), 0xFFFFFFFFFFFF0000);


        RAX(&reg) = 0xFFFFFFFFFFFFFFFF;
        EAX(&reg) = 0x0;
        EXPECT_EQ(RAX(&reg), 0xFFFFFFFF00000000);


        EAX(&reg) = 0xFFFFFFFF;
        AX(&reg) = 0;
        EXPECT_EQ(EAX(&reg), 0xFFFF0000);


        AX(&reg) = 0xFFFF;
        AL(&reg) = 0x00;
        EXPECT_EQ(AX(&reg), 0xFF00);

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
        ENDTEST
    }

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


        ScriptEnv::deleteScriptEnv(env);

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
        stackMemory.moveTo(-4, 4, (st_byte*)&a);
        stackMemory.moveFrom(-4, 4, (st_byte*)&b);
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

        ScriptEnv::deleteScriptEnv(env);

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


        ScriptEnv::deleteScriptEnv(env);

        ENDTEST
    }

    TEST(ScriptEngine, StackMemoryTest_overflow_localVariables) {
        ScriptEnv* env = ScriptEnv::newScriptEnv();
        auto& stackMemory = env->context->stackMemory;
        
        stackMemory.localVariables(stackMemory.stackSize);
        EXPECT_EQ(stackMemory.isOverflowed, true);

        ScriptEnv::deleteScriptEnv(env);

        ENDTEST
    }

    TEST(ScriptEngine, ScriptEngineTestSomeScript0) {

        constexpr char source[] = R"(
fn main()
{
    int b = 9
    int a = 500
    int c = 500
    
    return c - (b + a)
}
)";
        int ret = ScriptEnv::startScript((char*)source, sizeof(source) - 1);
        EXPECT_EQ(ret, -9);
        EXPECT_EQ(1, sizeof(char));

        ENDTEST
    }




    TEST(ScriptEngine, ScriptEngineTest_assign) {
        constexpr char source[] = R"(
fn main()
{
    ?int *ptr = null
    $int b = 9
    b = (10 + 1) - 2
    return b
}
)";
        int ret = ScriptEnv::startScript((char*)source, sizeof(source) - 1);
        EXPECT_EQ(ret, 9);
        ENDTEST
    }

    TEST(ScriptEngine, ScriptEngineTest_null) {
        constexpr char source[] = R"(
fn main()
{
    ?int *ptr = null
    return ptr
}
)";
        int ret = ScriptEnv::startScript((char*)source, sizeof(source) - 1);
        EXPECT_EQ(ret, 0);
        ENDTEST
    }

    TEST(ScriptEngine, ScriptEngineTest_heapString) {
        constexpr char source[] = R"(
fn main()
{
    String *ptr = "ijfowjio"
    String *ptr2 = ptr
    return ptr2
}
)";
        int ret = ScriptEnv::startScript((char*)source, sizeof(source) - 1);
        EXPECT_NE(ret, 0);
        ENDTEST
    }
    
    TEST(ScriptEngine, ScriptEngineTest_sub) {
        constexpr char source[] = R"(
fn main()
{
    let b = 9
    int a = 5
    int c = 5
    
    return c + b - a
}
)";
        int ret = ScriptEnv::startScript((char*)source, sizeof(source) - 1);
        EXPECT_EQ(ret, 9);

        ENDTEST
    }

    TEST(ScriptEngine, ScriptEngineTest_variable_to_variable) {
        constexpr char source[] = R"(
fn main()
{
    let b = 9
    int c = b
    
    return c + b
}
)";
        int ret = ScriptEnv::startScript((char*)source, sizeof(source) - 1);
        EXPECT_EQ(ret, 18);
        ENDTEST
    }


    TEST(ScriptEngine, ScriptEngineTest_i64) {
        constexpr char source[] = R"(
fn main()
{
    i64 b = 5L
    i64 a = 900L
    i64 c = 901L
    
    return c - (a + b)
}
)";
        ScriptEnv* env = ScriptEnv::loadScript((char*)source, sizeof(source) - 1);
        if (env->document->context->syntaxErrorInfo.hasError) {
            FAIL();
            return;// env->document->context->syntaxErrorInfo.errorItem.errorId;
        }

        env->validateScript();
        if (env->context->logicErrorInfo.hasError) {
            FAIL();
            env->context->setErrorPositions();
            return;// env->context->logicErrorInfo.firstErrorItem->codeErrorItem.errorId;
        }
        auto* node = env->mainFunc->bodyNode.firstChildNode;
        while (node) {
            if (node->vtable == VTables::ReturnStatementVTable) {
                auto returnState = Cast::downcast<ReturnStatementNodeStruct*>(node);
                EXPECT_EQ(returnState->valueNode->vtable, VTables::BinaryOperationVTable);
                auto binary0 = Cast::downcast<BinaryOperationNodeStruct*>(returnState->valueNode);

                auto *c = Cast::downcast<VariableNodeStruct*>(binary0->leftExprNode);

                auto pare = Cast::downcast<ParenthesesNodeStruct*>(binary0->rightExprNode);
                EXPECT_EQ(pare->valueNode->vtable, VTables::BinaryOperationVTable);
                auto binary = Cast::downcast<BinaryOperationNodeStruct*>(pare->valueNode);
                auto *a = Cast::downcast<VariableNodeStruct*>(binary->leftExprNode);
                auto *b = Cast::downcast<VariableNodeStruct*>(binary->rightExprNode);

                // return c - (a + b)
                EXPECT_EQ(c->calcRegEnum, PrimitiveCalcRegisterEnum::ebx);

                EXPECT_EQ(a->calcRegEnum, PrimitiveCalcRegisterEnum::eax);
                EXPECT_EQ(b->calcRegEnum, PrimitiveCalcRegisterEnum::ebx);
                
                EXPECT_EQ(binary->calcRegEnum, PrimitiveCalcRegisterEnum::ecx);
                EXPECT_EQ(binary0->rightExprNode->calcRegEnum, PrimitiveCalcRegisterEnum::ecx);

                // stack 
                EXPECT_EQ(c->stackOffset, -24);
                EXPECT_EQ(a->stackOffset, -16);
                EXPECT_EQ(b->stackOffset, -8);
                EXPECT_EQ(env->mainFunc->stackSize, 24);
            }
            
            node = node->nextNode;
        }
        int ret = env->runScriptEnv();
        EXPECT_EQ(ret, -4);

        //EXPECT_EQ(c - (a + b), 6);

        ENDTEST
    }
}