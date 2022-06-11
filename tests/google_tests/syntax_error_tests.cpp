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






static void testSyntaxError(const char* codeText, int textlen, ErrorCode errorCode, const char* errorCodeText);

#define TEST_SYNTAX_ERROR(codeText, textlen, errorCode) \
testSyntaxError(codeText, textlen, errorCode, #errorCode)


TEST(SyntaxErrors_, AssignmentSyntaxError) {

    {
        constexpr char text2[] = u8R"(
            {"key": "value }
)";

        TEST_SYNTAX_ERROR(text2, sizeof(text2) - 1, ErrorCode::missing_closing_quote);
    }
    {
        constexpr char text2[] = u8R"(
            {"key" "value" }
)";

        TEST_SYNTAX_ERROR(text2, sizeof(text2) - 1, ErrorCode::missing_object_delemeter);
    }

}



static void testSyntaxError(const char* codeText, int textlen, ErrorCode errorCode, const char* errorCodeText)
{
    auto* document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
    DocumentUtils::parseText(document, codeText, textlen);


    GLOG << std::endl  << " ------------------------------------------------------ ";
    GLOG << errorCodeText << std::endl;
    GLOG << codeText << std::endl;

    auto* context = document->context;
    EXPECT_EQ(context->syntaxErrorInfo.hasError, true);
    EXPECT_EQ(context->syntaxErrorInfo.errorCode, errorCode);

    Alloc::deleteDocument(document);
}


ENDTEST


