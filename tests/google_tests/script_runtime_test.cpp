﻿#include <stdio.h>
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


    /*
    static void testJson(const char* codeText);


    static void testJson(const char* codeText) {
        auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
        DocumentUtils::parseText(document, codeText, strlen(codeText));

        char *treeText = DocumentUtils::getTextFromTree(document);
        DocumentUtils::generateHashTables(document);
        EXPECT_EQ(std::string{ treeText }, std::string{ codeText });

        Alloc::deleteDocument(document);
    }
    */


    TEST(ScriptEngine, mallocItem_test1) {
        ScriptEnv* env = ScriptEnv::newScriptEnv();

        int* mem;
        for (int i = 0; i < 1000; i++) {
            mem = (int*)env->context->mallocItem(sizeof(int));
            *mem = 53;

            if (i % 3 == 2) {
                env->context->freeItem(mem);
            }
        }
        console_log("test");

        //EXPECT_EQ(*mem, 3);
        EXPECT_EQ(env->context->memBufferForMalloc.firstBufferBlock, env->context->memBufferForMalloc.currentBufferBlock);

        ScriptEnv::deleteScriptEnv(env);

        EXPECT_EQ(3, 3);

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