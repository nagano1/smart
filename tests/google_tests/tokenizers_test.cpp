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
    "aowowo" : 21249,
})");

    auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
    auto* context = document->context;
    DocumentUtils::parseText(document, text, strlen(text)); // for initialize a context

    {
        static constexpr const char class_chars[] = "\"my text\"";

        context->length = sizeof(class_chars) - 1;;
        context->chars = (char*)class_chars;
        int result = Tokenizers::stringLiteralTokenizer(nullptr, class_chars[0], 0, context);

        EXPECT_EQ(result, 9);
        EXPECT_EQ(context->virtualCodeNode->vtable, VTables::StringLiteralVTable);
        auto* stru = Cast::downcast<StringLiteralNodeStruct*>(context->virtualCodeNode);
        EXPECT_EQ(std::string{ stru->str }, std::string{ "my text" });
    }

    {
        static constexpr const char chars[] = u8R"("A\u864e\u306e\u0061")";

        context->length = sizeof(chars) - 1;
        context->chars = (char*)chars;
        int result = Tokenizers::stringLiteralTokenizer(nullptr, chars[0], 0, context);

        EXPECT_EQ(result, 21);
        EXPECT_EQ(context->virtualCodeNode->vtable, VTables::StringLiteralVTable);

        auto* stru = Cast::downcast<StringLiteralNodeStruct*>(context->virtualCodeNode);
        EXPECT_EQ(stru->strLength, 8);
        EXPECT_EQ(std::string{ stru->str }, std::string{ u8R"(A虎のa)" });

        EXPECT_EQ(stru->textLength, 21);
        EXPECT_EQ(std::string{ stru->text }, std::string{ u8R"("A\u864e\u306e\u0061")" });
    }

    {
        static constexpr const char chars[] = u8"\"\\r\\n\\t\\t\"";

        context->length = sizeof(chars) - 1;
        context->chars = (char*)chars;
        int result = Tokenizers::stringLiteralTokenizer(nullptr, chars[0], 0, context);

        EXPECT_EQ(result, 10);
        EXPECT_EQ(context->virtualCodeNode->vtable, VTables::StringLiteralVTable);

        auto* stru = Cast::downcast<StringLiteralNodeStruct*>(context->virtualCodeNode);
        EXPECT_EQ(stru->strLength, 4);
        EXPECT_EQ(std::string{ stru->str }, std::string{ u8"\r\n\t\t" });

        EXPECT_EQ(stru->textLength, 10);
        EXPECT_EQ(std::string{ stru->text }, std::string{ u8"\"\\r\\n\\t\\t\"" });
    }


    Alloc::deleteDocument(document);
}

//static void testTokenizer(const char* codeText) {
    //}
//}


ENDTEST


