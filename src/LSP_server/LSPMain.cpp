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
        scanf_s("Content-Length: %d", &n);

        fprintf(stderr, "n: %d", n);
        fflush(stderr);

        if (n > 0) {
            // skip remain headers
            bool lineBreak = false;
            char c1;
            while (EOF != (c1 = getchar())) {
                if (c1 == '\n') {
                    if (lineBreak) break;
                    lineBreak = true;
                } else lineBreak = false;
            }

            auto *chars = (char *) malloc(n + 1);

            auto rsize = fread(chars, 1, n, stdin);
            if (rsize > 0) {
                chars[rsize] = '\0';
                lspManager.nextRequest(chars, rsize);
            }

            free(chars);
        } else {
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

    /*
    export const None = 0;
    export const Full = 1;
    export const Incremental = 2;
    */

    const char *responseMessage = u8R"(Content-Length: 231

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
}









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
