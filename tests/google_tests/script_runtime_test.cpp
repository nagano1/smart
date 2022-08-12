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





    TEST(ScriptEngine, scriptDemo) {

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