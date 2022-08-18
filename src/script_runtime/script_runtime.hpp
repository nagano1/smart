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
        //SyntaxErrorInfo syntaxErrorInfo;

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
            mallocItem->ptr = malloc(bytes);
            return (void*)mallocItem->ptr;
        }

        void freeItem(void *ptr) {
            MallocItem *item = (MallocItem*)ptr;
            this->memBufferForMalloc.tryDelete<MallocItem>(item);
            if (!item->freed) {
                free(item->ptr);
                item->freed = true;
            }
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