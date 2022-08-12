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

    using ScriptEngingContext = struct _scriptEngineContext {
        //SyntaxErrorInfo syntaxErrorInfo;

        MemBuffer memBuffer;

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

        static void deleteScriptEnv(_ScriptEnv *doc);
        static _ScriptEnv *newScriptEnv();
        TypeEntry *newTypeEntry() const;
        static ValueBase *newValueForHeap();
        static ValueBase *newValueForStack();

        void registerTypeEntry(TypeEntry* typeEntry);
    };


    void startScript(char *script, int byteLength);
    
}