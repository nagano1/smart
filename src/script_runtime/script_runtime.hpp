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


namespace smart {


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

    using ScriptEngingContext = struct _scriptEngineContext {
        MemBuffer memBuffer;
        MemBuffer memBufferForMalloc;

        template<typename T>
        T *newMem() {
            return (T *) memBuffer.newMem<T>(1);
        }

        template<typename T>
        void tryDelete(T *m) {
            return (T *) memBuffer.tryDelete(m);
        }

        template<typename T>
        T *newMemArray(st_size len) {
            return (T *) memBuffer.newMem<T>(len);
        }

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

        /* this can be use only for same size item*/
        void freeAll() {
            this->memBuffer.freeAll();

            auto *block = this->memBufferForMalloc.firstBufferBlock;
            while (block) {
                if (block->itemCount > 0) {
                    int offset = 0;
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
        }
    };

    using TypeEntry = struct _typeEntry {
        int typeIndex;
        char *(*toString)(ValueBase* value);
    };

    using ScriptEnv = struct _ScriptEnv {
        TypeEntry **typeEntryList;
        int typeEntryListCapacity;
        int typeEntryListNextIndex;

        ScriptEngingContext *context;

        ValueBase *evaluateExprNode(NodeBase* expressionNode);
        ValueBase *evaluateExprNodeOrTest(NodeBase *expressionNode, ValueBase *testPointer);


        static void deleteScriptEnv(_ScriptEnv *doc);
        static _ScriptEnv *newScriptEnv();
        TypeEntry *newTypeEntry() const;

        template<typename T>
        T *newMem() {
            return (T *) context->memBufferForMalloc.newMem<T>(1);
        }

        void deleteMem(void *ptr) {
            return context->memBufferForMalloc.tryDelete(ptr);// newMem<T>(1);
        }


        static ValueBase *newValueForHeap();
        static ValueBase *newValueForStack();

        void registerTypeEntry(TypeEntry* typeEntry);
    };


    void startScript(char *script, int byteLength);
    
}