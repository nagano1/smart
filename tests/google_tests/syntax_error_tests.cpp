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






static void testJsonSyntaxError(const char* codeText, int textlen, ErrorCode errorCode, const char* errorCodeText);
static void testSyntaxError(const char* codeText, int textlen, ErrorCode errorCode, const char* errorCodeText);

#define TEST_JSON_SYNTAX_ERROR(codeText, textlen, errorCode) \
    testJsonSyntaxError(codeText, textlen, errorCode, #errorCode)

#define TEST_SYNTAX_ERROR(codeText, textlen, errorCode) \
    testSyntaxError(codeText, textlen, errorCode, #errorCode)


TEST(SyntaxErrors, JsonSyntaxError) {
    // missing_object_delemeter
    {
        constexpr char text2[] = u8R"(
            {"key": "value }
)";

        TEST_JSON_SYNTAX_ERROR(text2, sizeof(text2) - 1, ErrorCode::missing_closing_quote);
    }

    // missing_object_delemeter
    {
        constexpr char text2[] = u8R"(
            {"key" "value"  }
)";

        TEST_JSON_SYNTAX_ERROR(text2, sizeof(text2) - 1, ErrorCode::missing_object_delemeter);
    }
}




static void testJsonSyntaxError(const char* codeText, int textlen, ErrorCode errorCode, const char* errorCodeText)
{
    auto* document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
    DocumentUtils::parseText(document, codeText, textlen);


    GLOG << std::endl  << "------------------------------------------------------" << std::endl;
    GLOG << errorCodeText << std::endl;
    GLOG << codeText << std::endl;

    auto* context = document->context;
    EXPECT_EQ(context->syntaxErrorInfo.hasError, true);
    EXPECT_EQ(context->syntaxErrorInfo.errorCode, errorCode);

    Alloc::deleteDocument(document);
}

static void testSyntaxError(const char* codeText, int textlen, ErrorCode errorCode, const char* errorCodeText)
{
    auto* document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
    DocumentUtils::parseText(document, codeText, textlen);


    GLOG << std::endl << "------------------------------------------------------" << std::endl;
    GLOG << errorCodeText << std::endl;
    GLOG << codeText << std::endl;

    auto* context = document->context;
    EXPECT_EQ(context->syntaxErrorInfo.hasError, true);
    EXPECT_EQ(context->syntaxErrorInfo.errorCode, errorCode);

    Alloc::deleteDocument(document);
}


ENDTEST



TEST(SyntaxErrors, CodeSyntaxError) {

    // invalid_class_name
    {
        constexpr char text[] = u8R"(
class {


}
)";
        TEST_SYNTAX_ERROR(text, sizeof(text) - 1, ErrorCode::invalid_class_name);
    }


    // no_brace_for_class
    {
        constexpr char code[] = u8R"(
class A 


}
)";

        TEST_SYNTAX_ERROR(code, sizeof(code) - 1, ErrorCode::no_brace_for_class);
    }


    // no_brace_of_end_for_class
    {
        constexpr char code[] = u8R"(
class A {


)";
        TEST_SYNTAX_ERROR(code, sizeof(code) - 1, ErrorCode::no_brace_of_end_for_class);

        constexpr char code2[] = u8R"(
class A {
    class B {
    }
)";
        TEST_SYNTAX_ERROR(code2, sizeof(code2) - 1, ErrorCode::no_brace_of_end_for_class);

    }


}


ENDTEST





TEST(SyntaxErrors, IndentSyntaxError) {

    // indent syntax errors
    {
        constexpr char text[] = u8R"(
class A {
    /*
   aa

    */

}
)";
        TEST_SYNTAX_ERROR(text, sizeof(text) - 1, ErrorCode::indent_error);
    }
}


ENDTEST

