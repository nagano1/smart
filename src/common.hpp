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

using st_size = unsigned long;
using st_uint = unsigned long;
using st_int = long;

//using st_textlen = long;
using st_byte = unsigned char;

#define UNUSED(x) (void)(x)

#define st_size_of(T) ((int)sizeof(T))
//#define st_size_of(T) ((st_size)sizeof(T))

#ifdef __x86_64__
// do x64 stuff
#elif __arm__
// do arm stuff
#endif

constexpr static int BUFFER_SIZE = 64; //25

#ifdef assert
#undef assert
#endif
#define assert(expression) static_assert(true, "ok")



#define when(op) switch(op) 
#define wfor_noop(val) case val: {\
    break; \
    } \

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
    __android_log_print(ANDROID_LOG_DEBUG, "aaaa", ": %s", str);
}


#define PREPARE_OSTREAM \
//Foo foo{};

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
    void *chunk = nullptr;
    MemBufferBlock *next = nullptr;
    MemBufferBlock *prev = nullptr;
    bool isLast = true;
    int itemCount = 0;
};

struct MemBuffer {
    static constexpr st_size DEFAULT_BUFFER_SIZE = 255;

    MemBufferBlock *firstBufferBlock = nullptr;
    MemBufferBlock *currentBufferBlock = nullptr;
    st_uint currentMemOffset = DEFAULT_BUFFER_SIZE + 1;

    void init() {
        this->currentMemOffset = DEFAULT_BUFFER_SIZE + 1;
        this->firstBufferBlock = nullptr;
        this->currentBufferBlock = nullptr;
    }

    void freeAll() {
        MemBufferBlock *bufferList = this->firstBufferBlock;

        while (bufferList) {
            free(bufferList->chunk);

            auto *temp = bufferList;
            bufferList = bufferList->next;
            free(temp);
        }
    }

    template<typename Type>
    void tryDelete(Type *ptr) {
        if (this->firstBufferBlock == nullptr) {
            return;
        }

        MemBufferBlock *targetBufferList = *((MemBufferBlock **) ((st_byte *) ptr - sizeof(MemBufferBlock *)));
        targetBufferList->itemCount--;

        this->tryFreeMemoryBlock(targetBufferList);
    }

    inline void tryFreeMemoryBlock(MemBufferBlock *memBufferBlock)
    {
        assert(memBufferBlock != nullptr);

        if (memBufferBlock->itemCount == 0 && !memBufferBlock->isLast) {
            assert(memBufferBlock->next != nullptr);

            // can delete & free
            auto *prev = memBufferBlock->prev;
            if (prev == nullptr) {
                assert(targetBufferList == this->firstBufferBlock);

                this->firstBufferBlock = memBufferBlock->next;
                memBufferBlock->next->prev = nullptr;
            } else {
                prev->next = memBufferBlock->next;
                memBufferBlock->next->prev = prev;
            }

            free(memBufferBlock->chunk);
            free(memBufferBlock);
        }
    }


    template<typename Type>
    Type *newMem(unsigned int count) {
        auto bytes = st_size_of(Type) * count;
        auto sizeOfPointerToBlock = st_size_of(MemBufferBlock*);
        auto length = bytes + sizeOfPointerToBlock;


        if (currentMemOffset + length < DEFAULT_BUFFER_SIZE) {

        }
        else {
            MemBufferBlock* tryDeleteBlock = nullptr;

            st_size assign_size = (DEFAULT_BUFFER_SIZE < length ? length : DEFAULT_BUFFER_SIZE);
            if (firstBufferBlock == nullptr) {
                firstBufferBlock = currentBufferBlock = (MemBufferBlock*)malloc(sizeof(MemBufferBlock));
                firstBufferBlock->chunk = (void *)calloc(assign_size, 1);
                firstBufferBlock->prev = nullptr;
            }
            else {
                auto *newNode = (MemBufferBlock*)malloc(sizeof(MemBufferBlock));
                newNode->chunk = (void *)calloc(assign_size, 1);

                currentBufferBlock->next = newNode;
                newNode->prev = currentBufferBlock;
                if (currentBufferBlock->itemCount == 0) {
                    tryDeleteBlock = currentBufferBlock;
                }

                currentBufferBlock->isLast = false;
                currentBufferBlock = newNode;
            }

            currentBufferBlock->isLast = true;
            currentBufferBlock->itemCount = 0;
            currentBufferBlock->next = nullptr;

            currentMemOffset = 0;

            if (tryDeleteBlock) {
                this->tryFreeMemoryBlock(tryDeleteBlock);
            }
        }
        currentBufferBlock->itemCount++;
        Type *node = (Type*)((st_byte*)(currentBufferBlock->chunk) + currentMemOffset);

        auto **address = (MemBufferBlock **)node;
        *address = currentBufferBlock;

        this->currentMemOffset += length;

        return (Type*)((st_byte*)node + sizeOfPointerToBlock);
    }
};

