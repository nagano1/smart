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

#include "./LSPMain.hpp"
#include "./LSPLocalServer.hpp"

#include "code_nodes.hpp"

#include "common.hpp"

using namespace smart;
/*
Content-Length: 200

{"jsonrpc":"2.0","method":"initialized","params":{}}
 */





void LSPManager::LSP_main() {
    LSPManager lspManager;
    std::thread{ []() {
        LSPHttpServer::LSP_server();

    } };
    //th.get_id();
    //th2->detach();

    

    while (true) {
        int n = 0;
        scanf("Content-Length: %d", &n);

        fprintf(stderr, "n: %d\n", n);
        fflush(stderr);

        if (n > 0) {
            // skip remain headers
            bool lineBreak = false;
            int c1;
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
                lspManager.nextRequest(chars, (st_textlen)rsize);
            }

            free(chars);
        }
        else {
            fprintf(stderr, "FAILED!!!\n");
            fflush(stderr);
            int stop = getchar();
            if (EOF == stop) {
                return;
            }
        }
    }
}


static bool getLineAndPos(int pos, const utf8byte *text, size_t textLength, int &line, int &charactor) {
    int currentLine = 0;
    int currentCharactor = 0;
    int lineFirstPos = 0;

    for (uint32_t i = 0; i < textLength; i++) {

        if (i == pos) {
            line = currentLine;
            charactor = ParseUtil::utf16_length(text + lineFirstPos, currentCharactor);
            return true;
        }
        
        currentCharactor++;

        utf8byte ch = text[i];
        if (ParseUtil::isBreakLine(ch)) {
            currentCharactor = 0;
            currentLine++;
            lineFirstPos = i;
        }
    }

    
    return false;
}

static void validateJson(const char *text, st_textlen textLength) {
    auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
    DocumentUtils::parseText(document, text, textLength);

    if (document->context->syntaxErrorInfo.hasError) {

        int line = 0;
        int charactor = 0;
        bool ok = getLineAndPos(document->context->syntaxErrorInfo.charPosition, text, textLength, line, charactor);
        if (!ok) {
            line = 0;
        }

        char moji[1024];
        sprintf(moji, u8R"({"jsonrpc": "2.0","method": "textDocument/publishDiagnostics","params": {"uri":"file:///c:/Users/wikihow/Desktop/AAA.txt","diagnostics": [{"severity": 1,"range": { "start": { "character": %d, "line": %d }, "end": { "character": %d, "line": %d } },"message": "%s","source": "ex"}]}})", charactor, line, 0, line+1, document->context->syntaxErrorInfo.reason);
        fprintf(stderr, "\n\n[%s]\n\n", moji);
        fflush(stderr);

        std::string responseMessage = std::string{ "Content-Length:" } +std::to_string(+strlen(moji)) + "\n\n" + std::string{ moji };

        fprintf(stdout, "%s", responseMessage.c_str());
        fflush(stdout);
    }
    else {
        const char body[] = u8R"({"jsonrpc": "2.0","method": "textDocument/publishDiagnostics","params": {"uri":"file:///c%3A/Users/wikihow/Desktop/AAA.txt","diagnostics": []}})";

        std::string responseMessage = std::string{ "Content-Length:" } +std::to_string(+strlen(body)) + "\n\n" + std::string{ body };

        fprintf(stdout, "%s", responseMessage.c_str());
        fflush(stdout);
    }

    Alloc::deleteDocument(document);

    /*
    char *treeText = DocumentUtils::getTextFromTree(document);
    fprintf(stderr, "\ntext:\n[%s]\n\n", text);
    fflush(stderr);

    fprintf(stderr, "\ntreeText:\n[%s]\n\n", treeText);
    fflush(stderr);
    */
}


void LSPManager::nextRequest(char *chars, st_textlen length) {
    fprintf(stderr, "req: \n%s", chars);
    fflush(stderr);

    auto *document = Alloc::newDocument(DocumentType::JsonDocument, nullptr);
    DocumentUtils::parseText(document, chars, length);

    /*
        DocumentUtils::performCodingOperation(
            CodingOperations::IndentSelection
            , document, Cast::upcast(document->firstRootNode), Cast::upcast(&document->endOfFile)
        );
    */


    /*
    {"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"file:///c%3A/Users/wikihow/Desktop/AAA.txt","languageId":"plaintext","version":1,"text":"AAA\r\n\r\nBBB\r\nCCC\r\nAAA\r\nBBB\r\n\r\nCCC\r\nDDD\r\nEEE\r\nCCC"}}}
    */

    char *treeText = DocumentUtils::getTextFromTree(document);
    DocumentUtils::generateHashTables(document);

    auto *rootJson = Cast::downcast<JsonObjectStruct*>(document->firstRootNode);
    fprintf(stderr, "type: %s", rootJson->vtable->typeChars);
    fflush(stderr);

    if (rootJson) {
        auto *item = rootJson->hashMap->get2("method");
        if (item) {
            auto *strNode = Cast::downcast<StringLiteralNodeStruct*>(item);

            fprintf(stderr, "here5: [%s]", strNode->str);
            fflush(stderr);

            if (strNode->textLength > 0 && 0 == strcmp(strNode->str, "initialize")) {
                const char body[] = u8R"({"jsonrpc": "2.0","id" : "0","result" : {"capabilities": {"textDocumentSync": 1,"completionProvider": { "resolveProvider": true }}}})";

                std::string responseMessage = std::string{ "Content-Length: " } +std::to_string( + strlen(body)) + std::string{ "\n\n" }+std::string{ body };

                fprintf(stderr, "[%s]", responseMessage.c_str()); fflush(stderr);

                fprintf(stdout, "%s", responseMessage.c_str());
                fflush(stdout);
                return;
            }

            if (strNode->textLength > 0 && 0 == strcmp(strNode->str, "textDocument/didChange")) {
                auto *item2 = Cast::downcast<JsonObjectStruct*>(rootJson->hashMap->get2("params"));
                auto *item3 = Cast::downcast<JsonArrayStruct*>(item2->hashMap->get2("contentChanges"));
                auto *item5 = Cast::downcast<JsonObjectStruct*>(item3->firstItem->valueNode);
                auto *item4 = Cast::downcast<StringLiteralNodeStruct*>(item5->hashMap->get2("text"));


                validateJson(item4->str, item4->strLength);
                
                return;
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
