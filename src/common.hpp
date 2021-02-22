#pragma once

#include <iostream>
#include <string>
#include <condition_variable>
#include <array>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>


using utf8byte = char;
using utf8chars = const utf8byte *;

using sm_byte = char;

#ifdef __x86_64__
// do x64 stuff
#elif __arm__
// do arm stuff
#endif

constexpr static int BUFFER_SIZE = 64; //25

#ifdef assert
#undef assert
#endif
#define assert(expression)  \


#define when(op) switch(op) 
#define wfor(val, handler) case val: {\
    (handler); break; \
    } \

#define welse(handler) default: {\
    (handler); break; \
    } \

template<class T>
static inline T *simpleMalloc2() {
    return (T *) malloc(sizeof(T));
}


#ifdef __ANDROID__

#include <android/log.h>

inline void console_log(const char *str) {
    __android_log_print(ANDROID_LOG_DEBUG, "aaa", ": %s", str);
}


#define PREPARE_OSTREAM \
//Foo foo{}; \

#else
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
inline void console_log(const char *str) {
    //std::cout << message;
    printf("%s", str);
}
#else
inline void console_log(const char *str) {
    //std::cout << message;
    printf("%s", str);
}
#endif

#endif


struct MemBufferBlock {
    void *list = nullptr;
    MemBufferBlock *next = nullptr;
    bool isLast = true;
    int itemCount = 0;
};

struct MemBuffer {
    static constexpr int DEFAULT_BUFFER_SIZE = 255;

    MemBufferBlock *firstBufferBlock = nullptr;
    MemBufferBlock *currentBufferBlock = nullptr;
    unsigned int currentMemOffset = DEFAULT_BUFFER_SIZE + 1;

    void init() {
        this->currentMemOffset = DEFAULT_BUFFER_SIZE + 1;
        this->firstBufferBlock = nullptr;
        this->currentBufferBlock = nullptr;
    }

    void freeAll() {
        MemBufferBlock *bufferList = this->firstBufferBlock;

        while (bufferList) {
            free(bufferList->list);

            auto *temp = bufferList;
            bufferList = bufferList->next;
            free(temp);
        }
    }

    template<typename Type>
    void tryDelete(Type *ptr) {
        auto *targetBufferList = *((MemBufferBlock **)((sm_byte*)ptr - sizeof(MemBufferBlock*)));
        targetBufferList->itemCount--;
        auto *next = targetBufferList->next;
        if (next) {
            if (next->itemCount == 0 && next->isLast == false) {
                // can delete & free
            }
        }
    }


    template<typename Type>
    Type *newMem(unsigned int count) {
        size_t bytes = sizeof(Type) * count;
        auto sizeOfPointerToBlock = sizeof(MemBufferBlock*);
        auto length = bytes + sizeOfPointerToBlock;


        if (currentMemOffset + length < DEFAULT_BUFFER_SIZE) {

        }
        else {
            unsigned int assign_size = DEFAULT_BUFFER_SIZE < length ? length : DEFAULT_BUFFER_SIZE;
            if (firstBufferBlock == nullptr) {
                firstBufferBlock = currentBufferBlock = (MemBufferBlock*)malloc(sizeof(MemBufferBlock));
                firstBufferBlock->list = (void *)malloc(assign_size);
            }
            else {
                auto *newNode = (MemBufferBlock*)malloc(sizeof(MemBufferBlock));
                newNode->list = (void *)malloc(assign_size);

                currentBufferBlock->next = newNode;
                currentBufferBlock->isLast = false;
                currentBufferBlock = newNode;
            }

            currentBufferBlock->isLast = true;
            currentBufferBlock->itemCount = 0;
            currentBufferBlock->next = nullptr;

            currentMemOffset = 0;
        }
        currentBufferBlock->itemCount++;
        Type *node = (Type*)((sm_byte*)(currentBufferBlock->list) + currentMemOffset);

        auto **address = (MemBufferBlock **)node;
        *address = currentBufferBlock;

        this->currentMemOffset += length;

        return (Type*)((sm_byte*)node + sizeOfPointerToBlock);
    }
};

