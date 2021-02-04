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

template<class T>
static inline T *simpleMalloc() {
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


template<typename NodeType>
struct CharBuffer {
    static constexpr int CHAR_BUFFER_SIZE = 255;
    NodeType *list = nullptr;//[500];
    CharBuffer<NodeType> *next = nullptr;


    CharBuffer<NodeType> *firstBufferList = nullptr;
    CharBuffer<NodeType> *currentBufferList = nullptr;
    int spaceNodeIndex = CHAR_BUFFER_SIZE + 1;
    int itemCount = 0;

    void init() {
        spaceNodeIndex = CHAR_BUFFER_SIZE + 1;
        firstBufferList = nullptr;
        currentBufferList = nullptr;
    }

    NodeType *newChars(int charLen) {

        auto sizeOfBuffer = sizeof(CharBuffer<NodeType>*);
        auto length = charLen + sizeOfBuffer;

        if (spaceNodeIndex + length < CHAR_BUFFER_SIZE) {

        } else {
            int assign_size = CHAR_BUFFER_SIZE < length ? length : CHAR_BUFFER_SIZE;
            if (firstBufferList == nullptr) {

                firstBufferList = currentBufferList = simpleMalloc<CharBuffer<NodeType>>();
                firstBufferList->list = (NodeType *) malloc(sizeof(NodeType) * assign_size);
                firstBufferList->next = nullptr;
            } else {
                auto *newList = simpleMalloc<CharBuffer<NodeType>>();
                newList->list = (NodeType *) malloc(sizeof(NodeType) * assign_size);
                newList->next = nullptr;
                currentBufferList->next = newList;
                currentBufferList = newList;
            }
            spaceNodeIndex = 0;
        }
        currentBufferList->itemCount++;
        NodeType *node = currentBufferList->list + spaceNodeIndex + sizeOfBuffer;
        node[charLen - 1] = '\0';

        auto **address  = (CharBuffer<NodeType> **)(node);
        *address = currentBufferList;

        spaceNodeIndex += length;

        return node + sizeOfBuffer;
    }

};

template<typename NodeType>
void deleteCharBuffer(CharBuffer<NodeType> *bufferList) {
    if (bufferList) {
        if (bufferList->list) {
            free(bufferList->list);
        }
        if (bufferList->next) {
            deleteCharBuffer(bufferList->next);
        }
        free(bufferList);
    }
}



template<class NodeType>
struct NodeBufferList {
    NodeType *list = nullptr;//[500];
    NodeBufferList<NodeType> *next = nullptr;


    NodeBufferList<NodeType> *firstBufferList = nullptr;
    NodeBufferList<NodeType> *currentBufferList = nullptr;
    int_fast32_t spaceNodeIndex = BUFFER_SIZE + 1;

    void init() {
        firstBufferList = nullptr;
        currentBufferList = nullptr;

        spaceNodeIndex = BUFFER_SIZE + 1;
    }

    NodeType *newNode() {
        if (spaceNodeIndex < BUFFER_SIZE) {

        } else {
            if (firstBufferList == nullptr) {
                firstBufferList = currentBufferList = simpleMalloc<NodeBufferList<NodeType>>();
                firstBufferList->list = (NodeType *) malloc(sizeof(NodeType) * BUFFER_SIZE);
                firstBufferList->next = nullptr;

            } else {
                auto *newList = simpleMalloc<NodeBufferList<NodeType>>();
                newList->list = (NodeType *) malloc(sizeof(NodeType) * BUFFER_SIZE);
                newList->next = nullptr;
                currentBufferList->next = newList;
                currentBufferList = newList;
            }
            spaceNodeIndex = 0;
        }
        NodeType *node = currentBufferList->list + spaceNodeIndex;
        spaceNodeIndex++;

        return node;
    }

};

template<typename NodeType>
void deleteNodeBufferList(NodeBufferList<NodeType> *bufferList) {
    if (bufferList) {
        if (bufferList->list) {
            free(bufferList->list);
        }
        if (bufferList->next) {
            deleteNodeBufferList(bufferList->next);
        }
        free(bufferList);
    }
}

