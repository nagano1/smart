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

    //--------------------------------------------------------------------------------------------
    //
    //                                        TypeEntry
    //
    //--------------------------------------------------------------------------------------------
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
        auto *emptyTypeEntry = this->context->newMem<TypeEntry>();
        emptyTypeEntry->toString = nullptr;
        return emptyTypeEntry;
    }


    static ValueBase *newValue(bool heap) {
        auto *emptyTypeEntry = (ValueBase*)malloc(sizeof(ValueBase));// this->context->newMem<TypeEntry>();
        emptyTypeEntry->ptr = nullptr;
        emptyTypeEntry->size = 0;
        emptyTypeEntry->isHeap = heap;
        return emptyTypeEntry;
    }

    ValueBase *ScriptEnv::newValueForHeap()
    {
        return newValue(true);
    }

    ValueBase *ScriptEnv::newValueForStack()
    {
        return newValue(false);
    }

    char *int32_toString(ValueBase *value)
    {
        auto * chars = (char*)malloc(sizeof(char) * 64);
        sprintf(chars, "%d", *(int*)value->ptr);
        return chars;
    }

    char* heapString_toString(ValueBase* value)
    {
        return (char*)value->ptr;
    }

    static void _registerBuiltInTypes(ScriptEnv* scriptEnv)
    {
        TypeEntry *int32Type = scriptEnv->newTypeEntry();
        int32Type->toString = int32_toString;
        scriptEnv->registerTypeEntry(int32Type);
        BuiltInTypeIndex::int32 = int32Type->typeIndex;


        TypeEntry* heapStringType = scriptEnv->newTypeEntry();
        heapStringType->toString = heapString_toString;
        scriptEnv->registerTypeEntry(heapStringType);
        BuiltInTypeIndex::heapString = heapStringType->typeIndex;

    }

    int BuiltInTypeIndex::int32 = 0;
    int BuiltInTypeIndex::heapString = 0;


    //--------------------------------------------------------------------------------------------
    //
    //                                        Script Engine
    //
    //--------------------------------------------------------------------------------------------

    ScriptEnv *ScriptEnv::newScriptEnv()
    {
        auto *scriptEnv = (ScriptEnv*)malloc(sizeof(ScriptEnv));
        if (scriptEnv) {
            auto *context = simpleMalloc2<ScriptEngingContext>();
            context->memBuffer.init();
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
        scriptEnv->context->memBuffer.freeAll();
        free(scriptEnv->context);
        free(scriptEnv);
    }


    static FuncNodeStruct* findMainFunc(DocumentStruct *document)
    {
        auto *rootNode = document->firstRootNode;
        while (rootNode != nullptr) {
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


    ValueBase *evaluateNode(NodeBase *valueNode, ValueBase *const testPointer)
    {
        assert(valueNode != nullptr);
        assert(valueNode->vtable != nullptr);

        if (valueNode->vtable == VTables::StringLiteralVTable) {
            auto* strNode = Cast::downcast<StringLiteralNodeStruct *>(valueNode);

            if (testPointer) {
                return testPointer;
            }

            auto *value = ScriptEnv::newValueForHeap();
            value->typeIndex = BuiltInTypeIndex::heapString;
            int size = (1 + strNode->strLength)*sizeof(char);
            auto *array = (char*)malloc(size);
            array[strNode->strLength] = '\0';
            memcpy(array, strNode->str, strNode->strLength);
            value->ptr = array;
            value->size = size;
            return value;
        }

        if (valueNode->vtable == VTables::NumberVTable) {
            auto* numberNode = Cast::downcast<NumberNodeStruct *>(valueNode);

            if (testPointer) {
                return testPointer;
            }

            auto *value = ScriptEnv::newValueForHeap();
            value->typeIndex = BuiltInTypeIndex::int32;
            int size = sizeof(int);
            auto *int32val = (int*)malloc(size);
            *int32val = numberNode->num;
            value->ptr = int32val;
            value->size = size;
            return value;
        }


        return nullptr;
    }

    static void executeMain(ScriptEnv* env, FuncNodeStruct* mainFunc)
    {
        auto* childNode = mainFunc->bodyNode.firstChildNode;
        while (childNode) {

            if (childNode->vtable == VTables::CallFuncVTable) {
                auto* funcCall = Cast::downcast<CallFuncNodeStruct*>(childNode);

                // printf("jfoiweio")
                auto* arg = funcCall->firstArgumentItem;
                if (arg != nullptr) {
                    while (true) {
                        printf("arg = <%s>\n", arg->exprNode->vtable->typeChars);

                        auto *valueBase = evaluateNode(arg->exprNode, nullptr);
                        auto *chars = env->typeEntryList[valueBase->typeIndex]->toString(valueBase);
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