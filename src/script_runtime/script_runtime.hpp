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

    struct _ScriptEnv;

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



        void moveTo(int offsetFromBase, int byteCount, char* ptr) const;

        void moveFrom(int offsetFromBase, int byteCount, char* ptr) const;

    };


    using ValueBase = struct _valueBase {
        int typeIndex;
        void* ptr;
        unsigned int size; // byte size
    };


    struct BuiltInTypeIndex {
        static int int32;
        static int int_;
        static int heapString;
    };

    using MallocItem = struct Item {
        void *ptr{nullptr};
        bool freed{false};
    };


    // logic error is with NodeBase
    using LogicErrorItem = struct _ErrorNodeItem {
        _NodeBase *node;
        _ErrorNodeItem *next;

        CodeErrorItem codeErrorItem;
    };

    struct _typeEntry;
    using LogicalErrorInfo = struct _logicalErrorInfo {
        bool hasError{false};
        LogicErrorItem *firstErrorItem;
        LogicErrorItem *lastErrorItem;
        static const int SYNTAX_ERROR_RETURN = -1;
    };

    using ScriptEngineContext = struct _scriptEngineContext {
        _ScriptEnv* scriptEnv;
        LogicalErrorInfo logicErrorInfo;

        MemBuffer memBuffer; // for TypeEntry, variable->value map

        MemBuffer memBufferForValueBase; // for value base
        MemBuffer memBufferForMalloc2; // for value
        MemBuffer memBufferForError; // for value

        VoidHashMap *variableMap2;
        VoidHashMap *typeNameMap;
        StackMemory stackMemory;

        ValueBase *newValueForHeap();
        ValueBase *genValueBase(int type, int size, void *ptr);

        void init(_ScriptEnv *scriptEnv);

        void* mallocItem(int bytes) {
            auto *mallocItem = this->memBufferForMalloc2.newMem<MallocItem>(1);
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
            this->memBufferForMalloc2.tryDelete<MallocItem>(item);
        }

        void freeAll()
        {
            auto *block = this->memBufferForMalloc2.firstBufferBlock;
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
            this->memBufferForMalloc2.freeAll();
            this->memBufferForValueBase.freeAll();
            this->memBufferForError.freeAll();
            this->memBuffer.freeAll();
            this->stackMemory.freeAll();
        }

        void setErrorPositions();

        void addErrorWithNode(ErrorCode errorCode, NodeBase* node) {
            auto &errorInfo = this->logicErrorInfo;
            errorInfo.hasError = true;
            auto *mem = this->memBufferForError.newMem<LogicErrorItem>(1);
            mem->node = node;
            mem->codeErrorItem.errorCode = errorCode;
            mem->codeErrorItem.linePos1 = -1;
            mem->next = nullptr;
            if (errorInfo.firstErrorItem == nullptr) {
                errorInfo.firstErrorItem = mem;
            }

            if (errorInfo.lastErrorItem == nullptr) {
                errorInfo.lastErrorItem = mem;
            }
            else {
                errorInfo.lastErrorItem->next = mem;
                errorInfo.lastErrorItem = mem;
            }

            mem->codeErrorItem.errorId = getErrorId(errorCode);
            const char* reason = getErrorMessage(errorCode);
            if (reason == nullptr) {
                reason = "";
            }
            int len = (int)strlen(reason);
            mem->codeErrorItem.reasonLength = len < MAX_REASON_LENGTH ? len : MAX_REASON_LENGTH;
            memcpy(mem->codeErrorItem.reason, reason, mem->codeErrorItem.reasonLength);
            mem->codeErrorItem.reason[mem->codeErrorItem.reasonLength] = '\0';
        }

        static bool getLineAndPos(int pos, const utf8byte *text, int textLength, int *line, int *charactor) {
            int currentLine = 0;
            int currentCharactor = 0;
            int lineFirstPos = 0;

            for (int32_t i = 0; i < textLength; i++) {

                if (i == pos) {
                    *line = currentLine;
                    *charactor = ParseUtil::utf16_length(text + lineFirstPos, currentCharactor);
                    return true;
                }

                currentCharactor++;

                utf8byte ch = text[i];
                //if (ParseUtil::isBreakLine(ch)) {
                if ('\n' == ch) {
                    currentCharactor = 0;
                    currentLine++;
                    lineFirstPos = i;
                }
            }

            return false;
        }
    };



    enum class BuildinTypeId {
        Int32 = 1,
        HeapString = 2
    };

    using TypeEntry = struct _typeEntry {
        int typeIndex;
        int stackSize;
        char *(*toString)(ScriptEngineContext *context, ValueBase* value);
        ValueBase* (*operate_add)(ScriptEngineContext *context, ValueBase* leftValue, ValueBase* rightValue);
        char *typeChars;
        int typeCharsLength;
        BuildinTypeId typeId;
        bool isBuiltIn;

        template<std::size_t SIZE>
        void initAsBuiltInType(decltype(toString) f1, decltype(operate_add) f2,
                               const char(&f3)[SIZE], decltype(typeId) f4, decltype(stackSize) f5
        ) {
            this->toString = f1;
            this->operate_add = f2;
            this->typeChars = (char*)f3;
            this->typeCharsLength = SIZE;
            this->typeId = f4;
            this->stackSize = f5;
            this->isBuiltIn = true;
        }
    };


    using ScriptEnv = struct _ScriptEnv {

        DocumentStruct* document;
        TypeEntry **typeEntryList;
        int typeEntryListCapacity;
        int typeEntryListNextIndex;

        ScriptEngineContext *context;

        ValueBase *evaluateExprNode(NodeBase* expressionNode);
        ValueBase *evaluateExprNodeOrTest(NodeBase *expressionNode, ValueBase *testPointer);

        int typeFromNode(NodeBase *expressionNode);

        static void deleteScriptEnv(_ScriptEnv *doc);
        static _ScriptEnv *newScriptEnv();
        TypeEntry *newTypeEntry() const;

        static int startScript(char* script, int byteLength);

        static _ScriptEnv* loadScript(char* script, int byteLength);
        int validateScript() const;
        int runScriptEnv();

        void registerTypeEntry(TypeEntry* typeEntry);

        void addTypeAliasEntity(TypeEntry* typeEntry, char *f3 , int length);
        template<std::size_t SIZE>
        void addTypeAlias(TypeEntry* typeEntry, const char(&f3)[SIZE]) {
            this->addTypeAliasEntity(typeEntry , (char*)f3, SIZE-1);
        }
    };



}