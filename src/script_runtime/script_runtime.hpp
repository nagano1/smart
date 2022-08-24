#pragma once

#include <stdlib.h>
//#include <array>
//
//#include <cstdlib>
//#include <cassert>
//#include <cstdio>
//#include <chrono>
//#include <unordered_map>
//
//#include <cstdint> // uint64_t, int_fast32_t
//#include <ctime>
//
//#include <string.h> // memcpy
//
#include "parse_util.hpp"
#include "common.hpp"
#include "code_nodes.hpp"
//


namespace smart
{

    /*
     *
    // 11111111 11111111 11111111 11111111
    static constexpr unsigned char BYTE_BIT_COUNTS[256]{
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
    };

    // EXPECT_EQ(4, GetSetBitsCount(0b1111));

    int GetSetBitsCount(uint32_t n)
    {
        auto counts = BYTE_BIT_COUNTS;
        return n <= 0xff ? counts[n]
            : n <= 0xffff ? counts[n & 0xff] + counts[n >> 8]
            : n <= 0xffffff ? counts[n & 0xff] + counts[(n >> 8) & 0xff] + counts[(n >> 16) & 0xff]
            : counts[n & 0xff] + counts[(n >> 8) & 0xff] + counts[(n >> 16) & 0xff] + counts[(n >> 24) & 0xff];
    }
     *
     */
    using StackMemory = struct _StackMemory {

        int alignBytes; // 8, 16
        int baseBytes; // 8

        st_byte *chunk;
        int stackSize; // 2MB

        bool isOverflowed;

        st_byte *stackPointer; // esp, stack pointer
        st_byte *stackBasePointer; // ebp, stack base pointer

        // func(55, c:48) 0b101000...  for func(int a, int b = 32, int c = 8) // 32
        uint32_t argumentBits;

        uint64_t returnValue{6}; // EAX, Accumulator Register
        bool useBigStructForReturnValue{false};

        // methods
        void init();
        void freeAll();
        void push(uint64_t bytes);
        void localVariables(int bytes); // assign variable on local
        uint64_t pop();
        void call();
        void ret();

        void moveTo(int offsetFromBase, uint64_t val) const;
        uint64_t moveFrom(int offsetFromBase) const;
    };


    using ValueBase = struct _valueBase {
        int typeIndex;
        void* ptr;
        unsigned int size; // byte size
        bool isHeap;
    };


    struct BuiltInTypeIndex {
        static int int32;
        static int heapString;
    };

    using MallocItem = struct Item {
        void *ptr{nullptr};
        bool freed{false};
    };


    using ScriptEngineContext = struct _scriptEngineContext {
        MemBuffer memBuffer; // for TypeEntry, variable->value map

        MemBuffer memBufferForValueBase; // for value base
        MemBuffer memBufferForMalloc; // for value

        VoidHashMap *variableMap;
        StackMemory stackMemory;

        ValueBase *newValueForHeap();
        ValueBase *newValueForStack();
        ValueBase *genValueBase(int type, int size, void *ptr);



        void* mallocItem(int bytes) {
            auto *mallocItem = this->memBufferForMalloc.newMem<MallocItem>(1);
            mallocItem->freed = false;
            mallocItem->ptr = malloc(bytes + sizeof(MallocItem*));
            
            MallocItem** addressPtr = (MallocItem**)mallocItem->ptr;
            *addressPtr = mallocItem;

            return ((char*)mallocItem->ptr) + sizeof(MallocItem*);
        }

        void freeItem(void *ptr) {
            MallocItem *item = *(MallocItem**)((char*)ptr - sizeof(MallocItem*));
            assert(item != nullptr);
            if (!item->freed) {
                free(item->ptr);
                item->freed = true;
            }
            this->memBufferForMalloc.tryDelete<MallocItem>(item);
        }

        void freeAll() {
            
            auto *block = this->memBufferForMalloc.firstBufferBlock;
            while (block) {
                if (block->itemCount > 0) {
                    int offset = 0;
                    // this strategy can be used only for same size items
                    while (true) {
                        MemBufferBlock* item = *(MemBufferBlock**)((char*)block->chunk + offset);
                        if (item == block) {
                            MallocItem* item2 = (MallocItem*)((char*)block->chunk + offset + sizeof(MemBufferBlock*));
                            if (!item2->freed) {
                                free(item2->ptr);
                                item2->freed = true;
                            }
                            offset += sizeof(MemBufferBlock*) + sizeof(MallocItem);
                        }
                        else {
                            break;
                        }
                    }
                }
                block = block->next;
            }
            this->memBufferForMalloc.freeAll();
            this->memBufferForValueBase.freeAll();
            this->memBuffer.freeAll();
            this->stackMemory.freeAll();
        }
    };

    using TypeEntry = struct _typeEntry {
        int typeIndex;
        char *(*toString)(ScriptEngineContext *context, ValueBase* value);
        ValueBase* (*operate_add)(ScriptEngineContext *context, ValueBase* leftValue, ValueBase* rightValue);
    };

    using ScriptEnv = struct _ScriptEnv {
        TypeEntry **typeEntryList;
        int typeEntryListCapacity;
        int typeEntryListNextIndex;

        ScriptEngineContext *context;

        ValueBase *evaluateExprNode(NodeBase* expressionNode);
        ValueBase *evaluateExprNodeOrTest(NodeBase *expressionNode, ValueBase *testPointer);

        static void deleteScriptEnv(_ScriptEnv *doc);
        static _ScriptEnv *newScriptEnv();
        TypeEntry *newTypeEntry() const;
/*
        template<typename T>
        T *newMem() {
            return (T *) context->memBufferForMalloc.newMem<T>(1);
        }

        void deleteMem(void *ptr) {
            return context->memBufferForMalloc.tryDelete(ptr);
        }
*/

        void registerTypeEntry(TypeEntry* typeEntry);
    };


    void startScript(char *script, int byteLength);
    
}