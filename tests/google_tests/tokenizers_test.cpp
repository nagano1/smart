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

using namespace smart;




//static void testTokenizer(const char* codeText);

TEST(TokenizersTest_, StringLiteralTest) {
    auto text = const_cast<char *>(u8R"(
{
        "aowowo" :    21249,
"jio fw" : null,
            "text" : "日本語"
            "ijofw": [2134
                  	    ,
                            "test", true
                        null,
                        {"君はどうなんだろう": [true]}
            ]

})");

    auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
    DocumentUtils::parseText(document, text, strlen(text));


    auto testText = const_cast<char *>(u8"\"abc\"");
    int result;
    if (-1 < (result = Tokenizers::stringLiteralTokenizer(Cast::upcast(document), testText[0], 0, document->context))) {
    }

    EXPECT_EQ(result, -1);
//    EXPECT_EQ(std::string{ treeText }, std::string{ codeText });

    //char *typeText = DocumentUtils::getTypeTextFromTree(document);
    //    if (typeText != nullptr) {
    //EXPECT_EQ(std::string{ typeText }, std::string{ "fjow" });
    //}

    //char *treeText = DocumentUtils::getTextFromTree(document);
    DocumentUtils::generateHashTables(document);
    /*
    auto *jsonObject = DocumentUtils::generateHashTables(document);
    if (jsonObject) {
        auto *item = jsonObject->hashMap->get2("aowfowo");
        printf("item - %d", item);
    }
    */

    Alloc::deleteDocument(document);

}

//static void testTokenizer(const char* codeText) {
    //}
//}


ENDTEST


