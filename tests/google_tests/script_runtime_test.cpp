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

    TEST(ScriptEngine, mallocItem_test1) {
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
    }
    ENDTEST


    TEST(ScriptEngine, scriptEngine) {

        constexpr char source[] = R"(
fn main()
{
    let a = 0
    int b = 0
    print("test日本語")
}
)";
        startScript((char*)source, sizeof(source)-1);
    }

    ENDTEST

}