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
    //                                       StackMemory
    //
    //------------------------------------------------------------------------------------------

    void StackMemory::init()
    {
        this->alignBytes = 4; // 8, 16
        this->baseBytes = 8; // sizeof(uint64_t)

        this->isOverflowed = false;

        this->stackSize = 2 * 1024 * 1024; // 2MB
        this->chunk = (st_byte *)malloc(this->stackSize);

        this->argumentBits = 0;
        this->stackPointer = this->chunk + this->stackSize;
        this->stackBasePointer = this->chunk + this->stackSize;
    }

    void StackMemory::freeAll()
    {
        if (this->chunk) {
            free(this->chunk);
            this->chunk = nullptr;
        }
    }
    
    void StackMemory::push(uint64_t bytes)
    {
        if (this->stackPointer - this->baseBytes <= this->chunk) {
            // stack overvlow
            this->isOverflowed = true;
            return;
        }
        this->stackPointer -= this->baseBytes;
        *(uint64_t*)this->stackPointer = bytes;
    }

    // variable
    void StackMemory::localVariables(int bytes)
    {
        if (this->stackPointer - bytes <= this->chunk) {
            // stack overvlow
            this->isOverflowed = true;
            return;
        }

        this->stackPointer -= bytes;
    }

    uint64_t StackMemory::pop()
    {
        uint64_t data = *(uint64_t*)this->stackPointer;
        this->stackPointer += this->baseBytes;
        return data;
    }

    // assign value to stack
    void StackMemory::moveTo(int offsetFromBase, uint64_t val) const
    {
        if (this->stackBasePointer + offsetFromBase <= this->chunk) {
            // stack overflow
        }
        *(uint64_t*)(this->stackBasePointer + offsetFromBase) = val;
    }

    uint64_t StackMemory::moveFrom(int offsetFromBase) const
    {
        return *(uint64_t*)(this->stackBasePointer + offsetFromBase);
    }

    void StackMemory::call()
    {
        // caller side
        //this->push(23); // arg1
        //this->push(5); // arg2
        this->argumentBits = 0b101;
        this->useBigStructForReturnValue = false;

        // called side
        this->push((uint64_t)this->stackBasePointer);
        this->stackBasePointer = this->stackPointer;

    }

    void StackMemory::ret()
    {
        this->returnValue = 33;
        if (this->useBigStructForReturnValue) {
            //this->push(3);
        }

        this->stackPointer = this->stackBasePointer;
        this->stackBasePointer = (st_byte*)this->pop(); // NOLINT(performance-no-int-to-ptr)

        // sub for arguments
        // this->sub(88);
        this->argumentBits = 0;
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

    void ScriptEnv::addTypeAliasEntity(TypeEntry* typeEntry, char *f3 , int length)
    {
        this->context->typeNameMap->put(f3, length, typeEntry);
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

    static int32_t int32_value(ValueBase *value)
    {
        return  *(int32_t*)value->ptr;
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
        int32Type->initAsBuiltInType(int32_toString, int32_operate_add,
                                     "int", BuildinTypeId::Int32, 4); // 4byte
        scriptEnv->registerTypeEntry(int32Type);
        BuiltInTypeIndex::int32 = int32Type->typeIndex;
        BuiltInTypeIndex::int_ = int32Type->typeIndex;
        scriptEnv->addTypeAlias(int32Type, "int");
        scriptEnv->addTypeAlias(int32Type, "int32");


        // heap string
        TypeEntry* heapStringType = scriptEnv->newTypeEntry();
        heapStringType->initAsBuiltInType(heapString_toString, heapString_operate_add,
                                          "heapString", BuildinTypeId::HeapString, 8); //
        scriptEnv->registerTypeEntry(heapStringType);
        scriptEnv->addTypeAlias(heapStringType, "String");
        BuiltInTypeIndex::heapString = heapStringType->typeIndex;
    }

    int BuiltInTypeIndex::int32 = 0;
    int BuiltInTypeIndex::int_ = 0;
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

    void ScriptEngineContext::init()
    {
        this->logicalErrorInfo.hasError = false;
        this->logicalErrorInfo.errorCode = ErrorCode::no_logical_error;
        this->logicalErrorInfo.errorId = 57770000;
        this->logicalErrorInfo.charPosition = -1;
        this->logicalErrorInfo.charPosition2 = -1;
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

            context->variableMap2 = context->memBuffer.newMem<VoidHashMap>(1);
            context->variableMap2->init(&context->memBuffer);

            context->typeNameMap = context->memBuffer.newMem<VoidHashMap>(1);
            context->typeNameMap->init(&context->memBuffer);


            scriptEnv->context = context;
            context->init();

            scriptEnv->typeEntryList = nullptr;
            scriptEnv->typeEntryListNextIndex = 1;
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


    static constexpr int NodeToTypeLength = 128;
    static int nodeToType[NodeToTypeLength] = {-2};

    TypeEntry *ScriptEnv::typeFromNode(NodeBase *node)
    {
        int typeIndex = -1;

        if (node->vtable == VTables::NumberVTable) {
            typeIndex = BuiltInTypeIndex::int32;
        }
        else {
            if (nodeToType[0] == -2) {
                for (int i = 0; i < NodeToTypeLength; i++) { // NOLINT(modernize-loop-convert,altera-unroll-loops)
                    nodeToType[i] = -1;
                }
                nodeToType[2] = BuiltInTypeIndex::heapString;
                // nodeToType[11] = BuiltInTypeIndex::bool;
                // nodeToType[16] = BuiltInTypeIndex::NULLId;
            }

            typeIndex = nodeToType[(int)node->vtable->nodeTypeId];
        }
        /*
        EndOfDoc = 1,
        StringLiteral = 2,
        Symbol = 3,
        Class = 4,
        Name = 5,
        SimpleText = 6,


        Number = 9,
        LineBreak = 10,
        Bool = 11,


        JsonObject = 12,
        JsonObjectKey = 13,
        JsonKeyValueItem = 14,

        JsonArrayItem = 7,
        JsonArrayStruct = 8,

        Space = 15,

        Func = 17,
        NULLId = 16,

        Type = 18,
        Body = 19,
        AssignStatement = 20,
        ReturnStatement = 24,

        LineComment = 21,
        BlockComment = 22,
        BlockCommentFragment = 23,

        Variable = 25,
        Parentheses = 26,
        CallFunc = 27,
        FuncArgument = 28,
        FuncParameter = 29,

        BinaryOperation = 30
        */

        if (typeIndex > -1) {
            return this->typeEntryList[typeIndex];
        }

        return nullptr;
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

        if (expressionNode->vtable == VTables::ParenthesesVTable) {
            if (testPointer) { return testPointer; }
            auto* parentheses = Cast::downcast<ParenthesesNodeStruct *>(expressionNode);
            return evaluateExprNodeOrTest(parentheses->valueNode, testPointer);
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


    static void validateFunc(ScriptEnv* env, FuncNodeStruct* func) {

    }

    static int calcStackSizeInFunc(ScriptEnv* env, FuncNodeStruct* func)
    {
        auto* statementNode = func->bodyNode.firstChildNode;
        int stackSize = 0;
        int currentStackOffset = 0;
        while (statementNode) {
            if (statementNode->vtable == VTables::AssignStatementVTable) {
                auto *assignment = Cast::downcast<AssignStatementNodeStruct *>(statementNode);
                if (assignment->hasTypeDecl) {
                    auto &typeName = assignment->typeOrLet.nameNode;
                    bool isLet = assignment->typeOrLet.isLet;
                    // bool isInt = ParseUtil::equal(typeName.name, typeName.nameLength, "int", 3);
                    if (isLet) {

                    } else {
                        auto *typeEntry = (TypeEntry *) env->context->typeNameMap->get(
                                typeName.name, typeName.nameLength);
                        if (typeEntry) {
                            //printf("<test %d>", typeEntry->stackSize);
                            assignment->stackOffset = currentStackOffset;
                            stackSize += typeEntry->stackSize;
                            currentStackOffset += typeEntry->stackSize;
                        }
                    }
                }

            }
            statementNode = statementNode->nextNode;
        }
        return stackSize;
    }

    static int executeMain(ScriptEnv* env, FuncNodeStruct* mainFunc)
    {
        validateFunc(env, mainFunc);
        int stackSize = calcStackSizeInFunc(env, mainFunc);
        env->context->stackMemory.call();
        env->context->stackMemory.localVariables(stackSize);

        auto* statementNode = mainFunc->bodyNode.firstChildNode;
        while (statementNode) {
            if (statementNode->vtable == VTables::CallFuncVTable) {
                auto* funcCall = Cast::downcast<CallFuncNodeStruct*>(statementNode);

                auto* arg = funcCall->firstArgumentItem;
                if (arg != nullptr) {
                    while (true) {
                        printf("arg = <%s>\n", arg->exprNode->vtable->typeChars);
                        auto *valueBase = env->evaluateExprNode(arg->exprNode);
                        auto *chars = env->typeEntryList[valueBase->typeIndex]->toString(env->context, valueBase);
                        printf("chars = [%s]\n", chars);

                        if (arg->nextNode == nullptr) {
                            break;
                        }
                        else {
                            arg = Cast::downcast<FuncArgumentItemStruct*>(arg->nextNode);
                        }
                    }
                }
            }

            /*
            // assign
            // int a = 3
            if (statementNode->vtable == VTables::AssignStatementVTable) {
                auto* assignStatement = Cast::downcast<AssignStatementNodeStruct *>(statementNode);
                if (assignStatement->hasTypeDecl) {
                    auto* valueBase = env->evaluateExprNode(assignStatement->valueNode);
                    if (valueBase->typeIndex == BuiltInTypeIndex::int32) {
                        return int32_value(valueBase);
                    }
                }

            }
            */

            // return
            if (statementNode->vtable == VTables::ReturnStatementVTable) {
                auto* returnNode = Cast::downcast<ReturnStatementNodeStruct*>(statementNode);
                auto* valueBase = env->evaluateExprNode(returnNode->valueNode);
                if (valueBase->typeIndex == BuiltInTypeIndex::int32) {
                    return int32_value(valueBase);
                }
            }


            statementNode = statementNode->nextNode;
        }

        env->context->stackMemory.ret();

        return 0;
    }



    static void validateTypes(ScriptEnv *env, DocumentStruct *document)
    {
        //env->typeFromNode()

        auto *child = document->firstRootNode;
        while (child) { // NOLINT(altera-unroll-loops,altera-id-dependent-backward-branch)
            if (child->vtable == VTables::AssignStatementVTable) {

            }
            child = child->nextNode;
        }
    }

    static void *toType(NodeBase *nodeBase)
    {
        //auto *numberNode = Cast::downcast<NumberNodeStruct *>(nodeBase);

        return nullptr;
    }

    static void setupTypeFromNode()
    {
        ((node_vtable*)VTables::NumberVTable)->toType = toType;
        if(VTables::NumberVTable->toType(nullptr) != nullptr) {

        }
    }

    int ScriptEnv::startScript(char* script, int scriptLength)
    {
        int ret = 0;
        ScriptEnv* env = ScriptEnv::newScriptEnv();
        setupTypeFromNode();

        auto* document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
        DocumentUtils::parseText(document, script, scriptLength);
        DocumentUtils::generateHashTables(document);
        if (document->context->syntaxErrorInfo.hasError) {
            return - 1;
        }

        validateTypes(env, document);

        auto *mainFunc = findMainFunc(document);
        if (mainFunc) {
            printf("main Found");
            printf("<%s()>\n", mainFunc->nameNode.name);
            ret = executeMain(env, mainFunc);
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

        return ret;
    }
}