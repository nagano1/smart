#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>
//#include <emmintrin.h>

#include "LSPMain.hpp"
#include "code_nodes.hpp"

#include "common.hpp"

using namespace smart;
/*
Content-Length: 200

{"jsonrpc":"2.0","method":"initialized","params":{}}
 */





void LSPManager::LSP_main() {
    LSPManager lspManager;

    while (true) {
        int n = 0;
        scanf("Content-Length: %d", &n);

        fprintf(stderr, "n: %d\n", n);
        fflush(stderr);

        if (n > 0) {
            // skip remain headers
            bool lineBreak = false;
            char c1;
            while (EOF != (c1 = getchar())) {
                if (c1 == '\n') {
                    if (lineBreak) break;
                    lineBreak = true;
                }
                else lineBreak = false;
            }

            auto *chars = (char *)malloc(n + 1);

            auto rsize = fread(chars, 1, n, stdin);
            if (rsize > 0) {
                chars[rsize] = '\0';
                lspManager.nextRequest(chars, rsize);
            }

            free(chars);
        }
        else {
            fprintf(stderr, "FAILED!!!\n");
            fflush(stderr);
            char stop = getchar();
            if (EOF == stop) {
                return;
            }
        }
    }
}


void LSPManager::nextRequest(char *chars, size_t length) {

    fprintf(stderr, "req: \n%s", chars);
    fflush(stderr);


    auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
    DocumentUtils::parseText(document, chars, length);

    /*
    auto *document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
        DocumentUtils::parseText(document, text, strlen(text));
        DocumentUtils::performCodingOperation(
            CodingOperations::IndentSelection
            , document, Cast::upcast(document->firstRootNode), Cast::upcast(&document->endOfFile)
        );


        char *treeText = DocumentUtils::getTextFromTree(document);
        EXPECT_EQ(std::string{ treeText }, std::string{ autoIndentedText });
    */


    /*
    {"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"file:///c%3A/Users/wikihow/Desktop/AAA.txt","languageId":"plaintext","version":1,"text":"AAA\r\n\r\nBBB\r\nCCC\r\nAAA\r\nBBB\r\n\r\nCCC\r\nDDD\r\nEEE\r\nCCC"}}}
    */

    //char *typeText = DocumentUtils::getTypeTextFromTree(document);
    //    if (typeText != nullptr) {
            //EXPECT_EQ(std::string{ typeText }, std::string{ "fjow" });
        //}

    char *treeText = DocumentUtils::getTextFromTree(document);
    DocumentUtils::generateHashTables(document);

    auto *rootJson = Cast::downcast<JsonObjectStruct*>(document->firstRootNode);
    //fprintf(stderr, "type: %s", rootJson->vtable->typeChars);
    //fflush(stderr);

    //fprintf(stderr, "item2: %d", rootJson);
    if (rootJson) {
        auto *item = rootJson->hashMap->get2("method");
        if (item) {
            auto *strNode = Cast::downcast<StringLiteralNodeStruct*>(item);
            auto *method = strNode->text;

            if (strNode->textLength > 0 && 0 == strcmp(strNode->text, "\"textDocument/didOpen\"")) {

                fprintf(stderr, "method: %s", strNode->text);
                fflush(stderr);

                auto *item2 = Cast::downcast<JsonObjectStruct*>(rootJson->hashMap->get2("params"));
                auto *item3 = Cast::downcast<JsonObjectStruct*>(item2->hashMap->get2("textDocument"));
                auto *item4 = Cast::downcast<StringLiteralNodeStruct*>(item3->hashMap->get2("text"));
                fprintf(stderr, "text: %s", item4->text);
                fflush(stderr);

            }
        }


    }


    if (std::string{ chars } == std::string{ treeText }) {
        fprintf(stderr, "same\n"); fflush(stderr);
    }
    else {
        fprintf(stderr, "different \n"); fflush(stderr);
    }

    /*
    export const None = 0;
    export const Full = 1;
    export const Incremental = 2;
    */

    /*
    export declare namespace DiagnosticSeverity {
    const Error : 1;
    const Warning : 2;
    const Information : 3;
    const Hint : 4;
    }

    let diagnosic: Diagnostic = {
        severity: DiagnosticSeverity.Warning,
        range: {
            start: textDocument.positionAt(m.index),
            end: textDocument.positionAt(m.index + m[0].length)
        },
        message: `${m[0]} is all uppercase.`,
        source: 'ex'
    };


    */
    const char *reqMessage = u8R"(Content-Length: 231

{
    "jsonrpc": "2.0",
    "method" : "textDocument/publishDiagnostics",
    "params" : {
        "uri": "",
        "diagnostics": [
            {


            }
        ]
    }
}









)";


    const char *body = u8R"(
{
    "jsonrpc": "2.0",
    "id" : "0",
    "result" : {
    "capabilities": {
        "textDocumentSync": {
            "openClose": true,
                "change" : 1
        }
    }
}
)";


    const char *responseMessage = u8R"(Content-Length: 231



)";


    puts(responseMessage);
    fflush(stdout);

}


//static constexpr int CHAR_BUFFER_SIZE = 1;
//
//struct CharBufferItem {
//    utf8byte stack_buffer[CHAR_BUFFER_SIZE];
//    int size = 0;
//    CharBufferItem *next;
//};
//
///**
// * main entry for Language Server Protocol
// */
//void LSP_main3() {
//
//    int offset = 0;
//
//    auto *firstBufferItem = simpleMalloc<CharBufferItem>();
//    auto *currentBufferItem = firstBufferItem;
//
//    while (true) {
//        int ch = getchar();
//        if (ch == EOF) {
//            currentBufferItem->size = offset;
//            currentBufferItem->next = nullptr;
//            break;
//        }
//
//        if (offset >= CHAR_BUFFER_SIZE) {
//            auto *newBufferItem = simpleMalloc<CharBufferItem>();
//
//            currentBufferItem->next = newBufferItem;
//            currentBufferItem->size = offset;
//
//            currentBufferItem = newBufferItem;
//            offset = 0;
//        }
//
//        currentBufferItem->stack_buffer[offset] = static_cast<utf8byte>(ch);
//        offset++;
//    }
//
//
//    int totalCount = 0;
//    CharBufferItem *target = firstBufferItem;
//    while (target) {
//        totalCount += target->size;
//        for (int i = 0; i < target->size; i++) {
//            printf("[%c]", target->stack_buffer[i]);
//        }
//        target = target->next;
//    }
//    printf("\n totalCount = %d", totalCount);;
//}
