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


TEST(CodingOp, IndentSelection) {
     
     // DocumentUtils::performCodingOperation
    {
        char *text = const_cast<char *>(u8R"(
{
"jsonrpc":"2.0",
                    "jsonrpc2":"2.0",
                                        "jsonrpc3": {
"a":"日本語"
}
}
)");

        char *autoIndentedText = const_cast<char *>(u8R"(
{
    "jsonrpc":"2.0",
    "jsonrpc2":"2.0",
    "jsonrpc3": {
        "a":"日本語"
    }
}
)");
        auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
        DocumentUtils::parseText(document, text, strlen(text));
        EXPECT_EQ(document->context->syntaxErrorInfo.hasError, false);

        DocumentUtils::generateHashTables(document);

        auto *rootJson = Cast::downcast<JsonObjectStruct*>(document->firstRootNode);
        auto *item = rootJson->firstKeyValueItem->keyNode;
        if (item) {
            EXPECT_EQ(item->vtable, VTables::JsonObjectKeyVTable);
            DocumentUtils::performCodingOperation(
                CodingOperations::AutoIndentSelection
                , document, Cast::upcast(document->firstRootNode), Cast::upcast(&document->endOfFile));
        }


        char *treeText = DocumentUtils::getTextFromTree(document);
        EXPECT_EQ(std::string{ treeText }, std::string{ autoIndentedText });
    }





    {
     //   DocumentUtils::performCodingOperation CodingOperations::AutoIndentForSpacingRule
        {
            char *text = const_cast<char *>(u8R"(
{
"jsonrpc":"2.0",
                    "jsonrpc2":"2.0",
                        "jsonrpc3": {
    "a":"日本語"
}
}
)");


            char *autoIndentedText = const_cast<char *>(u8R"(
{
    "jsonrpc":"2.0",
                    "jsonrpc2":"2.0",
                        "jsonrpc3": {
                            "a":"日本語"
    }
}
)");
            auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
            DocumentUtils::parseText(document, text, strlen(text));
            EXPECT_EQ(document->context->syntaxErrorInfo.hasError, false);

            DocumentUtils::generateHashTables(document);

            auto *rootJson = Cast::downcast<JsonObjectStruct*>(document->firstRootNode);
            auto *item = rootJson->firstKeyValueItem->keyNode;
            if (item) {
                EXPECT_EQ(item->vtable, VTables::JsonObjectKeyVTable);
                DocumentUtils::performCodingOperation(
                        CodingOperations::AutoIndentForSpacingRule
                        , document, Cast::upcast(document->firstRootNode), Cast::upcast(&document->endOfFile));
            }


            char *treeText = DocumentUtils::getTextFromTree(document);
            EXPECT_EQ(std::string{ treeText }, std::string{ autoIndentedText });
        }

    }




    //DocumentUtils::performCodingOperation

    {
        char *text = const_cast<char *>(u8R"(
class A
{
class B
{

}
}
)");

        char *autoIndentedText = const_cast<char *>(u8R"(
class A
{
    class B
    {

    }
}
)");
        auto *document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
        DocumentUtils::parseText(document, text, strlen(text));
        DocumentUtils::performCodingOperation(
            CodingOperations::AutoIndentSelection
            , document, Cast::upcast(document->firstRootNode), Cast::upcast(&document->endOfFile)
        );


        char *treeText = DocumentUtils::getTextFromTree(document);
        EXPECT_EQ(std::string{ treeText }, std::string{ autoIndentedText });
    }




}

ENDTEST


