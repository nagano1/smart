#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <iostream>
#include <string>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>
#include <cstdint>

#include "script_runtime.hpp"

namespace smart {
    //------------------------------------------------------------------------------------------
    //
    //                                      StackMemory
    //
    //------------------------------------------------------------------------------------------

    void StackMemory::init()
    {
        this->alignBytes = 4; // 8, 16

        this->stackSize = 2 * 1024 * 1024; // 2MB
        this->chunk = (st_byte *)malloc(this->stackSize);

        this->stackPointer = this->chunk + this->stackSize;
        this->stackBasePointer = this->chunk + this->stackSize;
    }

    void StackMemory::freeAll()
    {
        if (this->chunk) {
            free(this->chunk);
        }
        this->chunk = nullptr;
    }
    
    void StackMemory::push(uint64_t bytes)
    {
        if (this->stackPointer - 8 <= this->chunk) {
            // stack overvlow
        }
        this->stackPointer -= 8;
        *(uint64_t*)this->stackPointer = bytes;
    }

    // variable
    void StackMemory::sub(int bytes)
    {
        if (this->stackPointer - bytes <= this->chunk) {
            // stack overvlow
        }

        this->stackPointer -= bytes;
    }


    uint64_t StackMemory::pop()
    {
        uint64_t data = *(uint64_t*)this->stackPointer;
        this->stackPointer += 8;
        return data;
    }

    // assign value to stack
    void StackMemory::moveTo(int offsetFromBase, uint64_t val) const
    {
        if (this->stackBasePointer + offsetFromBase <= this->chunk) {
            // stack overflow
        }
        *(uint64_t*)(this->stackBasePointer - offsetFromBase) = val;
    }

    uint64_t StackMemory::moveFrom(int offsetFromBase) const
    {
        return *(uint64_t*)(this->stackBasePointer - offsetFromBase);
    }

    void StackMemory::call()
    {
        // caller side
        this->push(23); // arg1
        this->push(5); // arg2
        this->argumentBits = 0b101;
        this->useBigStructForReturnValue = false;

        // called side
        this->stackBasePointer = this->stackPointer;
        this->push((uint64_t)this->stackBasePointer);
    }

    void StackMemory::ret()
    {
        this->returnValue = 33;
        if (this->useBigStructForReturnValue) {
            this->push(3);
        }

        this->stackBasePointer = (st_byte*)this->pop(); // NOLINT(performance-no-int-to-ptr)
    }



    //------------------------------------------------------------------------------------------
    //
    //                                        TypeEntry
    //
    //------------------------------------------------------------------------------------------

    static bool expandTypeEntryList(ScriptEnv *scriptEnv)
    {
        auto *oldListPointer = scriptEnv->typeEntryList;
        if (oldListPointer) {
            if (scriptEnv->typeEntryListNextIndex < scriptEnv->typeEntryListCapacity) {
                return true;
            }
        }

        const int newCapa = 8 + scriptEnv->typeEntryListCapacity;
        auto *newList = (TypeEntry**)malloc(newCapa * sizeof(TypeEntry*));
        if (newList) {
            scriptEnv->typeEntryList = newList;
            scriptEnv->typeEntryListCapacity = newCapa;

            if (oldListPointer) {
                for (int i = 0; i < scriptEnv->typeEntryListNextIndex; i++) { // NOLINT(altera-unroll-loops)
                    scriptEnv->typeEntryList[i] = (TypeEntry*)oldListPointer[i];
                }
                free(oldListPointer);
            }

            return true;
        }
        return false;
    }

    void ScriptEnv::registerTypeEntry(TypeEntry* typeEntry)
    {
        expandTypeEntryList(this);
        typeEntry->typeIndex = this->typeEntryListNextIndex;
        this->typeEntryList[typeEntry->typeIndex] = typeEntry;
        this->typeEntryListNextIndex++;
    }

    TypeEntry *ScriptEnv::newTypeEntry() const
    {
        auto *emptyTypeEntry = this->context->memBuffer.newMem<TypeEntry>(1);
        emptyTypeEntry->toString = nullptr;
        return emptyTypeEntry;
    }


    char* int32_toString(ScriptEngineContext *context, ValueBase *value)
    {
        auto * chars = (char*)malloc(sizeof(char) * 64);
        sprintf(chars, "%d", *(int32_t*)value->ptr);
        return chars;
    }

    ValueBase* int32_operate_add(ScriptEngineContext *context, ValueBase *leftValue, ValueBase *rightValue)
    {
        assert(leftValue->typeIndex == BuiltInTypeIndex::int32);

        if (rightValue->typeIndex == BuiltInTypeIndex::int32) {
            unsigned int size = sizeof(int32_t);
            int32_t *int32ptr;
            auto *value = context->genValueBase(BuiltInTypeIndex::int32, (int)size, &int32ptr);
            *int32ptr = *(int32_t*)leftValue->ptr + *(int32_t*)rightValue->ptr;
            return value;
        }
        return nullptr;
    }

    char* heapString_toString(ScriptEngineContext *context, ValueBase* value)
    {
        return (char*)value->ptr;
    }

    ValueBase* heapString_operate_add(ScriptEngineContext *context, ValueBase *leftValue, ValueBase *rightValue)
    {
        assert(leftValue->typeIndex == BuiltInTypeIndex::heapString);
        if (rightValue->typeIndex == BuiltInTypeIndex::heapString) {
            unsigned int size = (1 + leftValue->size + rightValue->size) * sizeof(char);
            char *chars;
            auto *value = context->genValueBase(BuiltInTypeIndex::heapString, (int)size, &chars);
            memcpy(chars, leftValue->ptr, leftValue->size);
            memcpy(chars + leftValue->size, rightValue->ptr, rightValue->size);
            chars[size-1] = '\0';

            return value;
        }

        return nullptr;
    }


    static void _registerBuiltInTypes(ScriptEnv* scriptEnv)
    {
        // int32
        TypeEntry *int32Type = scriptEnv->newTypeEntry();
        int32Type->toString = int32_toString;
        int32Type->operate_add = int32_operate_add;
        scriptEnv->registerTypeEntry(int32Type);
        BuiltInTypeIndex::int32 = int32Type->typeIndex;

        // heap string
        TypeEntry* heapStringType = scriptEnv->newTypeEntry();
        heapStringType->toString = heapString_toString;
        heapStringType->operate_add = heapString_operate_add;
        scriptEnv->registerTypeEntry(heapStringType);
        BuiltInTypeIndex::heapString = heapStringType->typeIndex;
    }

    int BuiltInTypeIndex::int32 = 0;
    int BuiltInTypeIndex::heapString = 0;


    //------------------------------------------------------------------------------------------
    //
    //                                Script Engine Context
    //
    //------------------------------------------------------------------------------------------

    static ValueBase *newValue(ScriptEngineContext *context, bool heap)
    {
        auto *valueBase = (ValueBase*)context->memBufferForValueBase.newMem<ValueBase>(1);
        valueBase->ptr = nullptr;
        valueBase->size = 0;
        valueBase->isHeap = heap;
        return valueBase;
    }

    ValueBase *ScriptEngineContext::newValueForHeap()
    {
        return newValue(this, true);
    }

    ValueBase *ScriptEngineContext::newValueForStack()
    {
        return newValue(this, false);
    }


    ValueBase *ScriptEngineContext::genValueBase(int type, int size, void *ptr)
    {
        auto *value = this->newValueForHeap();
        value->typeIndex = type;
        // value->ptr = context->memBufferForMalloc.newBytesMem(size); ////malloc(size);
        value->ptr = (void*)this->mallocItem(size);

        *(void**)ptr = value->ptr;
        value->size = size;
        return value;
    }

    //------------------------------------------------------------------------------------------
    //
    //                                       Script Engine
    //
    //------------------------------------------------------------------------------------------

    ScriptEnv *ScriptEnv::newScriptEnv()
    {
        auto *scriptEnv = (ScriptEnv*)malloc(sizeof(ScriptEnv));
        if (scriptEnv) {
            auto *context = simpleMalloc2<ScriptEngineContext>();
            context->memBuffer.init();
            context->memBufferForMalloc.init();
            context->memBufferForValueBase.init();
            context->stackMemory.init();

            context->variableMap = context->memBuffer.newMem<VoidHashMap>(1);
            context->variableMap->init(&context->memBuffer);

            scriptEnv->context = context;

            scriptEnv->typeEntryList = nullptr;
            scriptEnv->typeEntryListNextIndex = 0;
            scriptEnv->typeEntryListCapacity = 0;

            _registerBuiltInTypes(scriptEnv);
        }
        return scriptEnv;
    }

    void ScriptEnv::deleteScriptEnv(ScriptEnv *scriptEnv)
    {
        scriptEnv->context->freeAll();
        free(scriptEnv->context);
        free(scriptEnv);
    }


    static FuncNodeStruct* findMainFunc(DocumentStruct *document)
    {
        auto *rootNode = document->firstRootNode;
        while (rootNode != nullptr) { // NOLINT(altera-id-dependent-backward-branch,altera-unroll-loops)
            if (rootNode->vtable == VTables::FnVTable) {
                // fn
                auto *fnNode = Cast::downcast<FuncNodeStruct*>(rootNode);
                auto *nameNode = &fnNode->nameNode;
                if (ParseUtil::equal(nameNode->name, nameNode->nameLength, "main", 4))
                {
                    return fnNode;
                }
            }
            rootNode = rootNode->nextNode;
        }
        return nullptr;
    }

    ValueBase *ScriptEnv::evaluateExprNode(NodeBase *expressionNode)
    {
        return ScriptEnv::evaluateExprNodeOrTest(expressionNode, nullptr);
    }



    ValueBase *ScriptEnv::evaluateExprNodeOrTest(NodeBase *expressionNode, ValueBase *testPointer)
    {
        assert(expressionNode != nullptr);
        assert(expressionNode->vtable != nullptr);

        if (expressionNode->vtable == VTables::StringLiteralVTable) {
            if (testPointer) { return testPointer; }

            auto* strNode = Cast::downcast<StringLiteralNodeStruct *>(expressionNode);
            int size = (1 + strNode->strLength) * (int)sizeof(char);
            char *chars;
            auto *value = this->context->genValueBase(BuiltInTypeIndex::heapString, size, &chars);
            memcpy(chars, strNode->str, strNode->strLength);
            chars[strNode->strLength] = '\0';

            return value;
        }

        if (expressionNode->vtable == VTables::NumberVTable) {
            if (testPointer) { return testPointer; }

            auto* numberNode = Cast::downcast<NumberNodeStruct *>(expressionNode);
            int32_t *int32ptr;
            auto *value = this->context->genValueBase(BuiltInTypeIndex::int32, sizeof(int32_t), &int32ptr);
            *int32ptr = numberNode->num;
            return value;
        }

        if (expressionNode->vtable == VTables::VariableVTable) {

        }

        if (expressionNode->vtable == VTables::BinaryOperationVTable) {
            if (testPointer) { return testPointer; }

            auto* binaryNode = Cast::downcast<BinaryOperationNodeStruct *>(expressionNode);

            auto *leftValue = this->evaluateExprNode(binaryNode->leftExprNode);
            auto *rightValue = this->evaluateExprNode(binaryNode->rightExprNode);
            if (binaryNode->opNode.symbol[0] == '+') {
                auto *typeEntry = this->typeEntryList[leftValue->typeIndex];
                auto *retValue = typeEntry->operate_add(this->context, leftValue, rightValue);
                return retValue;
            }

        }


        if (expressionNode->vtable == VTables::CallFuncVTable) {
            //auto *funcCall = Cast::downcast<CallFuncNodeStruct *>(expressionNode);
            //auto *valueBase = this->evaluateExprNode(funcCall->exprNode);
            if (testPointer) { return testPointer; }

            // proceed Stack
        }

        return nullptr;
    }


    static void executeMain(ScriptEnv* env, FuncNodeStruct* mainFunc)
    {
        auto* childNode = mainFunc->bodyNode.firstChildNode;
        while (childNode) {

            if (childNode->vtable == VTables::CallFuncVTable) {
                auto* funcCall = Cast::downcast<CallFuncNodeStruct*>(childNode);
                //funcCall->exprNode

                auto* arg = funcCall->firstArgumentItem;
                if (arg != nullptr) {
                    while (true) {
                        printf("arg = <%s>\n", arg->exprNode->vtable->typeChars);

                        auto *valueBase = env->evaluateExprNode(arg->exprNode);
                        auto *chars = env->typeEntryList[valueBase->typeIndex]->toString(env->context, valueBase);
                        printf("chars = [%s]\n", chars);

                        if (arg->exprNode->vtable == VTables::StringLiteralVTable) {
                            //auto* stringArg = Cast::downcast<StringLiteralNodeStruct*>(arg->exprNode);
                        }

                        if (arg->nextNode == nullptr) {
                            break;
                        }
                        else {
                            arg = Cast::downcast<FuncArgumentItemStruct*>(arg->nextNode);
                        }

                    }
                }
            }
            childNode = childNode->nextNode;
        }
    }

    void startScript(char* script, int scriptLength)
    {
        ScriptEnv* env = ScriptEnv::newScriptEnv();

        auto* document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
        DocumentUtils::parseText(document, script, scriptLength);
        DocumentUtils::generateHashTables(document);


        auto *mainFunc = findMainFunc(document);
        if (mainFunc) {
            printf("main Found");
            printf("<%s()>\n", mainFunc->nameNode.name);
            executeMain(env, mainFunc);
        }

        ScriptEnv::deleteScriptEnv(env);

        auto* rootNode = document->firstRootNode;
        while (rootNode != nullptr) {

            // printf("%s\n", rootNode->vtable->typeChars);

            if (rootNode->vtable == VTables::ClassVTable) {
                // class
            }
            else if (rootNode->vtable == VTables::FnVTable) {
                // fn

            }

            rootNode = rootNode->nextNode;
        }
        Alloc::deleteDocument(document);

    }
}