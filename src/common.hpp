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

struct MemBuffer {
    static constexpr int CHAR_BUFFER_SIZE = 255;
    void *list = nullptr;//[500];
    MemBuffer *next = nullptr;


    MemBuffer *firstBufferList = nullptr;
    MemBuffer *currentBufferList = nullptr;
    unsigned int spaceNodeIndex = CHAR_BUFFER_SIZE + 1;
    int itemCount = 0;
    bool isLast = true;

    void init() {
        spaceNodeIndex = CHAR_BUFFER_SIZE + 1;
        firstBufferList = nullptr;
        currentBufferList = nullptr;
        itemCount = 0;
        isLast = true;
    }

    void freeAll() {
        MemBuffer *bufferList = this->firstBufferList;

        while (bufferList) {
            free(bufferList->list);

            auto *temp = bufferList;
            bufferList = bufferList->next;
            free(temp);
        }
    }

    template<typename Type>
    void tryDelete(Type *chars) {
        auto * currentBufferList = *((MemBuffer **)((sm_byte*)chars - sizeof(MemBuffer*)));
        currentBufferList->itemCount--;
        auto *next = currentBufferList->next;
        if (next) {
            if (next->itemCount == 0 && next->isLast == false) {
                // can delete & free
            }
        }
    }


    template<typename Type>
    Type *newMem(unsigned int count) {
        size_t charLen = sizeof(Type) * count;
        auto sizeOfBuffer = sizeof(MemBuffer*);
        auto length = charLen + sizeOfBuffer;


        if (spaceNodeIndex + length < CHAR_BUFFER_SIZE) {

        }
        else {
            unsigned int assign_size = CHAR_BUFFER_SIZE < length ? length : CHAR_BUFFER_SIZE;
            if (firstBufferList == nullptr) {

                firstBufferList = currentBufferList = (MemBuffer*)malloc(sizeof(MemBuffer));
                firstBufferList->list = (void *)malloc(assign_size);
                firstBufferList->next = nullptr;
            }
            else {
                auto *newList = (MemBuffer*)malloc(sizeof(MemBuffer));
                newList->list = (void *)malloc(assign_size);
                newList->next = nullptr;
                currentBufferList->next = newList;
                currentBufferList->isLast = false;
                currentBufferList = newList;
            }
            spaceNodeIndex = 0;
        }
        currentBufferList->itemCount++;
        Type *node = (Type*)((sm_byte*)(currentBufferList->list) + spaceNodeIndex);

        auto **address = (MemBuffer **)node;
        *address = currentBufferList;

        this->spaceNodeIndex += length;

        return (Type*)((sm_byte*)node + sizeOfBuffer);
    }
};

