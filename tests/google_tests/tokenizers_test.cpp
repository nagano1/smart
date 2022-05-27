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
    DocumentUtils::parseText(document, text, strlen(text));

    {
        static constexpr const char class_chars[] = "\"mytext\"";

        context->length = sizeof(class_chars) - 1;;
        context->chars = (char*)class_chars;
        int result = Tokenizers::stringLiteralTokenizer(nullptr, class_chars[0], 0, context);

        EXPECT_EQ(result, 8);
    }

    {
        static constexpr const char class_chars[] = u8"\"A\\r\\n\"";

        context->length = sizeof(class_chars) - 1;;
        context->chars = (char*)class_chars;
        int result = Tokenizers::stringLiteralTokenizer(nullptr, class_chars[0], 0, context);

        EXPECT_EQ(result, 7);
        EXPECT_EQ(context->codeNode->vtable, VTables::StringLiteralVTable);

        auto* stru = Cast::downcast<StringLiteralNodeStruct*>(context->codeNode);
        EXPECT_EQ(std::string{ stru->str }, std::string{ "A\r\n"});
    }









    Alloc::deleteDocument(document);

}

//static void testTokenizer(const char* codeText) {
    //}
//}


ENDTEST


