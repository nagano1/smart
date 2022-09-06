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
#include "script_runtime/script_runtime.hpp"

#include "common.hpp"

using namespace smart;
/*
Content-Length: 200

{"jsonrpc":"2.0","method":"initialized","params":{}}
 */

static const bool debugLog = true;




void LSPManager::LSP_main() {
    LSPManager lspManager;
    auto &&th = std::thread{ []() {
        LSPHttpServer::LSP_server();

    } };
    th.get_id();
    //th2->detach();

    

    while (true) {
        int n = 0;
        scanf("Content-Length: %d", &n);

        if (debugLog) {
            fprintf(stderr, "n: %d\n", n);
            fflush(stderr);
        }
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
                lspManager.nextRequest(chars, (int)rsize);
                LSPHttpServer::passText(chars, rsize);
            }

            free(chars);
        }
        else {
            if (debugLog) {
                fprintf(stderr, "FAILED!!!\n");
                fflush(stderr);
            }
            int stop = getchar();
            if (EOF == stop) {
                return;
            }
        }
    }


    //LSPHttpServer::close();
    //th.detach();
}

static DocumentStruct* latestDocument = nullptr;




static void sendMessageToClientHead(char* text, int len, int lenWithExtra)
{
    //std::this_thread::sleep_for(std::chrono::milliseconds{ 200 });

    printf("Content-Length:%d\r\n\r\n", lenWithExtra);
    fwrite(text, sizeof(char), len, stdout); // fprintf(stdout, "%s", text);
    fflush(stdout);

    // debug
    if (debugLog) {
        fprintf(stderr, "\nsent message = [%s%s", text, len == lenWithExtra ? "]" : "");
        fflush(stderr);
    }
}

static void sendMessageToClientExtra(char* text, int len)
{
    //std::this_thread::sleep_for(std::chrono::milliseconds{ 200 });

    fwrite(text, sizeof(char), len, stdout);
    fflush(stdout);
    
    // debug
    if (debugLog) {
        fprintf(stderr, "%s", text);
    }
}

static void sendMessageToClient(char* text, int len)
{
    sendMessageToClientHead(text, len, len);
}




constexpr char tail[] = u8R"(
]}
})";
constexpr int tailLength = sizeof(tail) - 1;


static void publishSemanticTokens(char *idText, int idTextLen, const char* const filePath, int filePathLength
    ,int line0, int char0, int line1, int char1
)
{
    if (latestDocument == nullptr || latestDocument->context->syntaxErrorInfo.hasError) {
        char moji[128];
        // "id": % s, "error" : { "code": 4124321, "message" : "has syntax error"}
        int len = sprintf(moji, u8R"(
    {
        "id": %s, "result": { "data": []}
    })", idText);

        sendMessageToClient(moji, len);
        return;
    }

    /*
　      { line: 2, startChar:  5, length: 3, tokenType: 0, tokenModifiers: 3 },
    */

    auto* document = latestDocument;
    int tokensLen;
    auto* semanticTokens = DocumentUtils::getSemanticTokensTextFromTree(document, &tokensLen, line0, line1);

    char moji[128];
    int len = sprintf(moji, u8R"(
{
    "id": %s
    , "result": {"data":[)", idText);


    sendMessageToClientHead(moji, len, len + tokensLen + tailLength);
    sendMessageToClientExtra(semanticTokens, tokensLen);
    sendMessageToClientExtra((char*)tail, tailLength);
}

static int makeErrorDiagnostics(char *error2, int offset, CodeErrorItem &errorItem, bool b) {

    unsigned long line = errorItem.linePos1;
    unsigned long charactor = errorItem.charPos1;
    unsigned long line2, charactor2;
    line2 = errorItem.linePos2;
    charactor2 = errorItem.charPos2;

    int len2 = sprintf(error2 + offset, R"(
        %s{
            "severity": 1
            ,"range": {
                "start": { "character": %lu, "line": %lu }
                , "end": { "character": %lu, "line": %lu }
            }
            ,"message": "%s"
            ,"source": "ex"
        }
)", b ? ",": "", charactor, line, charactor2, line2, errorItem.reason);
    error2[offset + len2] = '\0';

    fprintf(stderr, "[%s]", error2 + offset);
    fflush(stderr);

    return len2;
}

static void publishDiagnostics(const char *text, int textLength, const char * const filePath, int filePathLength)
{
    char diagnosticsText[1024 * 100]; //100KB
    diagnosticsText[0] = '\0';
    int diagnosticsTextLength = 0;



    //auto isCode= ParseUtil::endsWith2(filePath, filePathLength, ".smt"); // pls
    //auto isJson = ParseUtil::endsWith2(filePath, filePathLength, ".txt");

    ScriptEnv* env = ScriptEnv::loadScript((char*)text, textLength);
    if (!env->document->context->syntaxErrorInfo.hasError) {
        env->validateScript();
    }

    if (env->context->logicErrorInfo.hasError) {

        fprintf(stderr, "[logicErrorInfo.hasError]");
        fflush(stderr);
        env->context->setErrorPositions();

        auto* errorItem = env->context->logicErrorInfo.firstErrorItem;
        while (errorItem) {
            if (errorItem->codeErrorItem.linePos1 == -1) {
                break;
            }
            fprintf(stderr, "[logicErrorInfo.hasError.item]");
            fflush(stderr);
            int thislen = makeErrorDiagnostics((char*)diagnosticsText, diagnosticsTextLength, errorItem->codeErrorItem, diagnosticsTextLength > 0);
            diagnosticsTextLength += thislen;

            errorItem = errorItem->next;
        }

    }

    //auto *document = Alloc::newDocument(isCode ? DocumentType::CodeDocument : DocumentType::JsonDocument, nullptr);
    //DocumentUtils::parseText(document, text, textLength);

    auto* document = env->document;
    if (document->context->syntaxErrorInfo.hasError) {
        auto errorItem = document->context->syntaxErrorInfo.errorItem;
        if (errorItem.linePos2 > -1) {
        } else {
            errorItem.linePos2 = errorItem.linePos1 + 1;
            errorItem.charPos2 = 0;
        }

        int thislen = makeErrorDiagnostics(diagnosticsText, diagnosticsTextLength, document->context->syntaxErrorInfo.errorItem, diagnosticsTextLength > 0);
        diagnosticsTextLength += thislen;
    }

    if (diagnosticsTextLength > 0) {
        
/*

        int char1 = charactor;
        int char2 = 0;
        int line1 = line;
        int line2 = line + 1;
        if (document->context->syntaxErrorInfo.errorCode == ErrorCode::indent_error) {
            line2 = line;
            char2 = char1;
            char1 = 0;
        }
*/
        char moji[1024 * 100];
        int len = sprintf(moji, R"(
{
    "jsonrpc": "2.0"
    ,"method": "textDocument/publishDiagnostics"
    ,"params": {
        "uri":"%s"
        ,"diagnostics": [
            %s
        ]
    }
})", filePath , diagnosticsText);

        sendMessageToClient((char*)moji, len);
    }
    else {

        char* treeText = DocumentUtils::getTextFromTree(document);

        if (strcmp(treeText, text) != 0) {

            if (debugLog) {
                fprintf(stderr, "[%s] != ", treeText);
                fflush(stderr);
                fprintf(stderr, "[%s]", text);
                fflush(stderr);
            }
            // check equality

            free(treeText);
            exit(0);
            // EXPECT_EQ(strlen(treeText), strlen(chars));
        }
        free(treeText);


        char moji[512];
        int len = sprintf(moji, u8R"(
{
    "jsonrpc": "2.0"
    , "method": "textDocument/publishDiagnostics"
    , "params": {"uri":"%s","diagnostics": []}
})", filePath);


        sendMessageToClient((char*)moji, len);

        /*
        char reqRefreshSemanticTokens[512];
        int refreshLen = sprintf(reqRefreshSemanticTokens, u8R"(
{
    "jsonrpc": "2.0"
    , "method": "workspace/semanticTokens/refresh"
    , "params": {}
})");
        sendMessageToClient((char*)reqRefreshSemanticTokens, refreshLen);
        */
    }

    if (latestDocument != nullptr) {
        Alloc::deleteDocument(latestDocument);
    }
    latestDocument = document;

    /*
    char *treeText = DocumentUtils::getTextFromTree(document);
    fprintf(stderr, "\ntext:\n[%s]\n\n", text);
    fflush(stderr);

    fprintf(stderr, "\ntreeText:\n[%s]\n\n", treeText);
    fflush(stderr);
    */


}


void LSPManager::nextRequest(char *chars, int length) {
    if (debugLog) {
        fprintf(stderr, "\nreceived msg = \n[%s]", chars);
        fflush(stderr);
    }
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
    if (debugLog) {
        fprintf(stderr, "type: %s", rootJson->vtable->typeChars);
        fflush(stderr);
    }
    if (rootJson) {
        auto *item = (NodeBase*)rootJson->hashMap->get2("method");
        if (item) {
            auto *methodNode = Cast::downcast<StringLiteralNodeStruct*>(item);
            //if (debugLog) {
                fprintf(stderr, "\nmethod: [%s]", methodNode->str);
                fflush(stderr);
            //}
            char semanticTokens[512];
            char tokenModifiersText[512];
            for (int i = 0, pos = 0; tokenTypes[i] != nullptr; i++) {
                int len = sprintf(semanticTokens + pos, i== 0 ? "\"%s\"" : ",\"%s\"", tokenTypes[i]);
                pos += len;
            } 

            for (int i = 0, pos = 0; tokenModifiers[i] != nullptr; i++) {
                int len = sprintf(tokenModifiersText + pos, i == 0 ? "\"%s\"" : ",\"%s\"", tokenModifiers[i]);
                pos += len;
            }

            if (methodNode->textLength > 0 && 0 == strcmp(methodNode->str, "initialize")) {
                char body[1024];
                // , "completionProvider": { "resolveProvider": true }
                int len = sprintf(body, u8R"(
{
    "jsonrpc": "2.0"
    ,"id" : "0"
    ,"result" : {
        "capabilities": {
            "textDocumentSync": 1
            ,"semanticTokensProvider": {
                "legend": {
                    "tokenTypes": [%s],
                    "tokenModifiers": [%s]
                }, "range": true
                , "full": {"delta": false}
            }
        }
    }
})", semanticTokens, tokenModifiersText);

                sendMessageToClient((char*)body, len);
            }

            if (methodNode->textLength > 0) {
                auto didChange = 0 == strcmp(methodNode->str, "textDocument/didChange");
                auto didOpen = 0 == strcmp(methodNode->str, "textDocument/didOpen");
                auto isReqOfFullSemanticTokens = 0 == strcmp(methodNode->str, "textDocument/semanticTokens/full");
                auto rangeSemanticTokens = 0 == strcmp(methodNode->str, "textDocument/semanticTokens/range");

                auto* params = Cast::downcast<JsonObjectStruct*>((NodeBase*)rootJson->hashMap->get2("params"));

                if (didOpen || didChange) {
                    auto* textDocument = Cast::downcast<JsonObjectStruct*>(params->hashMap->get2("textDocument"));
                    auto* fileUri = Cast::downcast<StringLiteralNodeStruct*>(textDocument->hashMap->get2("uri"));

                    if (debugLog) {
                        fprintf(stderr, "file = %s\n", fileUri->str);
                        fflush(stderr);
                    }

                    if (didChange) {
                        auto* item3 = Cast::downcast<JsonArrayStruct*>(params->hashMap->get2("contentChanges"));
                        auto* item5 = Cast::downcast<JsonObjectStruct*>(item3->firstItem->valueNode);
                        auto* item4 = Cast::downcast<StringLiteralNodeStruct*>(item5->hashMap->get2("text"));

                        publishDiagnostics(item4->str, item4->strLength, fileUri->str, fileUri->strLength);
                    }
                    else if (didOpen) {
                        auto* item4 = Cast::downcast<StringLiteralNodeStruct*>(textDocument->hashMap->get2("text"));

                        publishDiagnostics(item4->str, item4->strLength, fileUri->str, fileUri->strLength);
                    }
                }

                if (isReqOfFullSemanticTokens || rangeSemanticTokens) {
                    auto* item = rootJson->hashMap->get2("id");
                    auto* idNode = Cast::downcast<StringLiteralNodeStruct*>(item);

                    auto* textDocument = Cast::downcast<JsonObjectStruct*>(params->hashMap->get2("textDocument"));
                    auto* fileUri = Cast::downcast<StringLiteralNodeStruct*>(textDocument->hashMap->get2("uri"));



                    int line0 = -1, char0 = -1, line1 = -1, char1 = -1;
                    if (params->hashMap->has("range", 5)) {
                        auto* range = Cast::downcast<JsonObjectStruct*>(params->hashMap->get2("range"));
                        auto* start = Cast::downcast<JsonObjectStruct*>(range->hashMap->get2("start"));
                        auto* end = Cast::downcast<JsonObjectStruct*>(range->hashMap->get2("end"));

                        line0 = atoi(Cast::downcast<NumberNodeStruct*>(start->hashMap->get2("line"))->text);
                        char0 = atoi(Cast::downcast<NumberNodeStruct*>(start->hashMap->get2("character"))->text);
                        line1 = atoi(Cast::downcast<NumberNodeStruct*>(end->hashMap->get2("line"))->text);
                        char1 = atoi(Cast::downcast<NumberNodeStruct*>(end->hashMap->get2("character"))->text);
                        if (debugLog) {
                            fprintf(stderr, "line0 = %d, line1 = %d", line0, line1);
                        }
                    }
                    /*
                    */

                    /*
                    "range":{"start":{"line":0,"character":0},"end":{"line":80,"character":1}}}
                    */

                    /*:
                    {"jsonrpc":"2.0","id":1,"method":"textDocument/semanticTokens/full"
                    ,"params":{"textDocument":{"uri":"file:///c%3A/GitProjects/doorlang_root/visual_studio_console_sln/testClass.smt"}}
                    */

                    publishSemanticTokens(idNode->text, idNode->textLength, fileUri->str, fileUri->strLength
                    ,line0, char0, line1, char1);
                    
                }
            }

        }
    }


    if (debugLog) {
        if (strcmp(chars, treeText) == 0) {
            fprintf(stderr, "same\n"); fflush(stderr);
        }
        else {
            fprintf(stderr, "different \n"); fflush(stderr);
        }
    }
    free(treeText);
    Alloc::deleteDocument(document);

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
