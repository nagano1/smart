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
    void StackMemory::moveTo(int offsetFromBase, int byteCount, st_byte*ptr) const
    {
        if (this->stackBasePointer + offsetFromBase <= this->chunk) {
            assert(false);
            // stack overflow
        }

        if (byteCount == 1) { // 8bit
            *(uint8_t*)(this->stackBasePointer + offsetFromBase) = *(uint8_t*)ptr;
        }
        else if (byteCount == 2) { // 16bit
            *(uint16_t*)(this->stackBasePointer + offsetFromBase) = *(uint16_t*)ptr;
        }
        else if (byteCount == 4) { // 32bit
            auto vl = *(uint32_t*)ptr;
            *(uint32_t*)(this->stackBasePointer + offsetFromBase) = vl;//*(uint32_t*)ptr;
        }
        else if (byteCount == 8) { // 64bit
            *(uint64_t*)(this->stackBasePointer + offsetFromBase) = *(uint64_t*)ptr;
        }
        else {
            memcpy(this->stackBasePointer + offsetFromBase, ptr, byteCount);
        }
    }


    void StackMemory::moveFrom(int offsetFromBase, int byteCount, st_byte* ptr) const
    {
        if (byteCount == 1) { // 8bit
            *(uint8_t*)(ptr) = *(uint8_t*)(this->stackBasePointer + offsetFromBase);
        }
        else if (byteCount == 2) { // 16bit
            *(uint16_t*)(ptr) = *(uint16_t*)(this->stackBasePointer + offsetFromBase);
        }
        else if (byteCount == 4) { // 32bit
            auto vl = *(uint32_t*)(this->stackBasePointer + offsetFromBase);
            *(uint32_t*)(ptr) = vl;//*(uint32_t*)(this->stackBasePointer + offsetFromBase);
        }
        else if (byteCount == 8) { // 64bit
            *(uint64_t*)(ptr) = *(uint64_t*)(this->stackBasePointer + offsetFromBase);
        }
        else {
            memcpy(ptr, this->stackBasePointer + offsetFromBase, byteCount);
        }
        //return *(uint64_t*)(this->stackBasePointer + offsetFromBase);
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
    //                                TypeEntry / Built-in Types
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

    char* int64_toString(ScriptEngineContext *context, ValueBase *value)
    {
        auto * chars = (char*)malloc(sizeof(char) * 128);
        sprintf(chars, "%llu", *(int64_t*)value->ptr);
        return chars;
    }


    /*
    static int32_t int32_value(ValueBase *value)
    {
        return  *(int32_t*)value->ptr;
    }
     */

    /*
     *
     *         if (expressionNode->vtable == VTables::StringLiteralVTable) {
            //auto* strNode = Cast::downcast<StringLiteralNodeStruct *>(expressionNode);
            //int size = (1 + strNode->strLength) * (int)sizeof(char);
//          memcpy(chars, strNode->str, strNode->strLength);
//          chars[strNode->strLength] = '\0';
            return;
        }

        if (expressionNode->vtable == VTables::ParenthesesVTable) {
            auto* parentheses = Cast::downcast<ParenthesesNodeStruct *>(expressionNode);
            evaluateExprNode(parentheses->valueNode);
            return;
        }


        if (expressionNode->vtable == VTables::NumberVTable) {
            auto* numberNode = Cast::downcast<NumberNodeStruct *>(expressionNode);
            if (numberNode->unit == 64) {
                *(int64_t*)numberNode->calcReg = numberNode->num;
            }
            else {
                *(int32_t*)numberNode->calcReg = (int32_t)numberNode->num;
            }
            return;
        }

     *
     *
     */

    void int32_evaluateNode(ScriptEngineContext *context, NumberNodeStruct *numberNode) {
        *(int32_t*)numberNode->calcReg = (int32_t)numberNode->num;
    }

    void int64_evaluateNode(ScriptEngineContext *context, NumberNodeStruct *numberNode) {
        *(int64_t*)numberNode->calcReg = numberNode->num;
    }

    int int32_binary_operate(ScriptEngineContext *context,
                           BinaryOperationNodeStruct *binaryNode, bool typeCheck)
    {
        // heap is not supported
        if (binaryNode->rightExprNode->typeAtHeap2 ||
            binaryNode->leftExprNode->typeAtHeap2) {
            return -1;
        }

        if (typeCheck) {
            return BuiltInTypeIndex::int32;
        }

        int32_t left32 = *(int32_t*)binaryNode->leftExprNode->calcReg;
        int32_t right32 = 0;
        if (binaryNode->rightExprNode->typeIndex2 == BuiltInTypeIndex::int32) {
            right32 = *(int32_t*)binaryNode->rightExprNode->calcReg;
        }
        else if (binaryNode->rightExprNode->typeIndex2 == BuiltInTypeIndex::int64) {
            right32 = (int32_t)(*(int64_t*)binaryNode->rightExprNode->calcReg);
        }
        switch (binaryNode->opNode.symbol[0]) {
            case '+': {
                *(int32_t*)binaryNode->calcReg = left32 + right32;
                break;
            }
            case '-': {
                *(int32_t*)binaryNode->calcReg = left32 - right32;
                break;
            }
        }

        return BuiltInTypeIndex::int32;
    }

    int int64_binary_operate(ScriptEngineContext *context, BinaryOperationNodeStruct *binaryNode, bool typeCheck)
    {
        // heap is not supported
        if (binaryNode->rightExprNode->typeAtHeap2 || binaryNode->leftExprNode->typeAtHeap2) {
            return -1;
        }

        if (typeCheck) {
            return BuiltInTypeIndex::int64;
        }

        int64_t left64 = *(int64_t*)binaryNode->leftExprNode->calcReg;
        int64_t right64 = 0;
        if (binaryNode->rightExprNode->typeIndex2 == BuiltInTypeIndex::int32) {
            right64 = *(int32_t *) binaryNode->rightExprNode->calcReg;
        }
        else if (binaryNode->rightExprNode->typeIndex2 == BuiltInTypeIndex::int64) {
            right64 = *(int64_t *) binaryNode->rightExprNode->calcReg;
        }

        switch (binaryNode->opNode.symbol[0]) {
            case '+': {
                *(int64_t*)binaryNode->calcReg = left64 + right64;
                break;
            }
            case '-': {
                *(int64_t*)binaryNode->calcReg = left64 - right64;
                break;
            }
        }
        return 0;
    }


    void heapString_evaluateNode(ScriptEngineContext *context, StringLiteralNodeStruct *node)
    {
        char *chars;
        int size = (1 + node->strLength) * (int)sizeof(char);
        auto *value = context->genValueBase(BuiltInTypeIndex::heapString, size, &chars);
        memcpy(chars, node->str, node->strLength);
        chars[node->strLength] = '\0';
        *(ValueBase **)node->calcReg = value;
    }

    char* heapString_toString(ScriptEngineContext *context, ValueBase* value)
    {
        return (char*)value->ptr;
    }


    int heapString_binary_operate(ScriptEngineContext *context, BinaryOperationNodeStruct *binaryNode, bool typeCheck)
    {
        if (!binaryNode->rightExprNode->typeAtHeap2 || !binaryNode->leftExprNode->typeAtHeap2) {
            return -1;
        }

        if (typeCheck) {
            return BuiltInTypeIndex::heapString;
        }

        /*
        assert(leftValue->typeIndex == BuiltInTypeIndex::heapString);
        if (rightValue->typeIndex == BuiltInTypeIndex::heapString) {
            unsigned int size = (1 + leftValue->size + rightValue->size) * sizeof(char);
            char *chars;
            auto *value = context->genValueBase(BuiltInTypeIndex::heapString, (int)size, &chars);
            memcpy(chars, leftValue->ptr, leftValue->size);
            memcpy(chars + leftValue->size, rightValue->ptr, rightValue->size);
            chars[size-1] = '\0';
        }
        */

        return 0;
    }


    char* null_toString(ScriptEngineContext *context, ValueBase* value)
    {
        return (char*)"null";
    }

    int null_binary_operate(ScriptEngineContext *context, BinaryOperationNodeStruct *binaryNode, bool typeCheck)
    {
        if (typeCheck) {
            return BuiltInTypeIndex::null;
        }

        return 0;
    }

    void null_evaluateNode(ScriptEngineContext *context, NullNodeStruct *node) {
        *(int64_t*)node->calcReg = 0;
    }

    static void _registerBuiltInTypes(ScriptEnv* scriptEnv)
    {
        // int
        {
            TypeEntry *int32Type = scriptEnv->newTypeEntry();
            int32Type->initAsBuiltInType(int32_toString, int32_binary_operate,
                                         int32_evaluateNode,
                                         "int", BuildinTypeId::Int32, 4, false); // 4byte
            scriptEnv->registerTypeEntry(int32Type);
            BuiltInTypeIndex::int32 = int32Type->typeIndex;
            scriptEnv->addTypeAlias(int32Type, "int");
            scriptEnv->addTypeAlias(int32Type, "i32");
        }

        {
            // i64
            TypeEntry *int64Type = scriptEnv->newTypeEntry();
            int64Type->initAsBuiltInType(int64_toString, int64_binary_operate,
                                         int64_evaluateNode,
                                         "i64", BuildinTypeId::Int64, 8, false); // 4byte
            scriptEnv->registerTypeEntry(int64Type);
            BuiltInTypeIndex::int64 = int64Type->typeIndex;
            scriptEnv->addTypeAlias(int64Type, "i64");
        }

        {
            // heap string
            TypeEntry *heapStringType = scriptEnv->newTypeEntry();
            heapStringType->initAsBuiltInType(heapString_toString,
                                              heapString_binary_operate,
                                              heapString_evaluateNode,
                                              "heapString", BuildinTypeId::HeapString, 8, /*heap only*/true); //
            scriptEnv->registerTypeEntry(heapStringType);
            scriptEnv->addTypeAlias(heapStringType, "String");
            BuiltInTypeIndex::heapString = heapStringType->typeIndex;
        }

        {
            // null
            TypeEntry *nullTypeEntry = scriptEnv->newTypeEntry();
            nullTypeEntry->initAsBuiltInType(null_toString,
                                              null_binary_operate, null_evaluateNode,
                                              "null", BuildinTypeId::Null, 8, /*heap only*/true); //
            scriptEnv->registerTypeEntry(nullTypeEntry);
            scriptEnv->addTypeAlias(nullTypeEntry, "Null");
            BuiltInTypeIndex::null = nullTypeEntry->typeIndex;
        }
    }

    int BuiltInTypeIndex::int32 = 0;
    int BuiltInTypeIndex::int64 = 0;
    int BuiltInTypeIndex::heapString = 0;
    int BuiltInTypeIndex::null = 0;



    //------------------------------------------------------------------------------------------
    //
    //                                 Type Selector from Node (static)
    //
    //------------------------------------------------------------------------------------------

    int ScriptEnv::typeFromNode(NodeBase *node)
    {
        if (node->vtable->typeSelector != nullptr) {
            return node->vtable->typeSelector(this, node);
        }
        return -1;
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
        //return nullptr;
    }

    static int selectTypeFromNumberNode(ScriptEnv *env, NumberNodeStruct *numberNode)
    {
        numberNode->typeAtHeap2 = false;
        if (numberNode->unit == 64) {
            numberNode->typeIndex2 = BuiltInTypeIndex::int64;
        }
        else {
            numberNode->typeIndex2 = BuiltInTypeIndex::int32;
        }

        return numberNode->typeIndex2;
    }

    static int selectTypeFromStringNode(ScriptEnv *env, StringLiteralNodeStruct *nodeBase)
    {
        nodeBase->typeAtHeap2 = true;
        return nodeBase->typeIndex2 = BuiltInTypeIndex::heapString;
    }

    static int selectTypeFromNullNode(ScriptEnv *env, NullNodeStruct *nodeBase)
    {
        nodeBase->typeAtHeap2 = true;
        return nodeBase->typeIndex2 = BuiltInTypeIndex::null;
    }

    template<typename T>
    static void setTypeSelector(const node_vtable* vtable, int (*argToType)(ScriptEnv *, T *)) {
        ((node_vtable*)vtable)->typeSelector = reinterpret_cast<int (*)(void *, NodeBase *)>(argToType);
    }

    static void setupBuiltInTypeSelectors(ScriptEnv *env)
    {
        setTypeSelector(VTables::NumberVTable, selectTypeFromNumberNode);
        setTypeSelector(VTables::StringLiteralVTable, selectTypeFromStringNode);
        setTypeSelector(VTables::NullVTable, selectTypeFromNullNode);
        /*
        if (VTables::NumberVTable->typeSelector(env, nullptr) != -1) {

        }
        */
    }



    //------------------------------------------------------------------------------------------
    //
    //                                Script Engine Context
    //
    //------------------------------------------------------------------------------------------

    ValueBase *ScriptEngineContext::newValueForHeap()
    {
        auto *valueBase = (ValueBase *) memBufferForValueBase.newMem<ValueBase>(1);
        valueBase->ptr = nullptr;
        valueBase->size = 0;
        return valueBase;
    }


    static void reassignLineNumbers(DocumentStruct *docStruct) {
        int lineNumber = 0;
        auto *line = docStruct->firstCodeLine;
        while (line) {
            line->lineNumber = lineNumber++;
            line = line->nextLine;
        }
    }


    static NodeBase* findFirstNodeInLine(CodeLine *firstLine, CodeLine *lastLine)
    {
        CodeLine *currentLine = firstLine;
        while (currentLine) {
            NodeBase *node = currentLine->firstNode;
            if (node) {
                return node;
            }

            currentLine = currentLine->nextLine;
        }

        return nullptr;
    }

    static NodeBase* findLastNodeInLine(CodeLine *firstLine, CodeLine *lastLine)
    {
        NodeBase *returnNode = nullptr;
        CodeLine *currentLine = firstLine;
        while (currentLine) {
            returnNode = currentLine->lastNode;
            currentLine = currentLine->nextLine;
        }

        return returnNode;
    }


    // return: utf16
    static int getPosInLine(NodeBase *node, bool beginningPos)
    {
        auto *codeLine = node->codeLine;

        int utf16Pos = 0;
        assert(codeLine);

        NodeBase *currenNode = codeLine->firstNode;
        while (currenNode) {

            utf16Pos += currenNode->prev_chars;

            if (currenNode == node) {
                if (beginningPos) {
                    break;
                }
            }

            int textLength = VTableCall::selfTextLength(currenNode);
            char *text = (char*)VTableCall::selfText(currenNode);

            utf16Pos += ParseUtil::utf16_length(text, textLength);

            if (currenNode == node) {
                break;
            }

            currenNode = currenNode->nextNodeInLine;
        }

        return utf16Pos;
    }


    void ScriptEngineContext::setErrorPositions()
    {
        reassignLineNumbers(this->scriptEnv->document);

        this->scriptEnv->document->context->appendLineMode = AppendLineMode::DetectErrorSpanNodes;

        auto *errorItem = this->logicErrorInfo.firstErrorItem;
        while (errorItem) {
            auto *node = errorItem->node;

            auto* firstCodeLine = this->scriptEnv->document->context->newCodeLine();
            firstCodeLine->init(this->scriptEnv->document->context);
            auto *lastCodeLine = VTableCall::callAppendToLine(node, firstCodeLine);

            auto *firstNode = findFirstNodeInLine(firstCodeLine, lastCodeLine);
            auto *lastNode = findLastNodeInLine(firstCodeLine, lastCodeLine);

            int a = getPosInLine(firstNode, true);
            int b = getPosInLine(lastNode, false);

            errorItem->codeErrorItem.charPos1 = a;
            errorItem->codeErrorItem.charPos2 = b;
            errorItem->codeErrorItem.charPosition = a;
            errorItem->codeErrorItem.charPosition2 = b;
            errorItem->codeErrorItem.linePos1 = firstNode->codeLine->lineNumber;
            errorItem->codeErrorItem.linePos2 = lastNode->codeLine->lineNumber;

            errorItem = errorItem->next;
        }

        // DocumentUtils::regenerateCodeLines(docStruct);
        static_assert(true, "not implemented");
    }

    // //int32_t *int32ptr;
    //            //auto *value = this->context->genValueBase(BuiltInTypeIndex::int32, sizeof(int32_t), &int32ptr);
    //            //*int32ptr = numberNode->num;
    //            \param type
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

    void ScriptEngineContext::init(ScriptEnv *scriptEnvArg)
    {
        this->scriptEnv = scriptEnvArg;
        this->logicErrorInfo.hasError = false;
        this->logicErrorInfo.count = 0;
        this->logicErrorInfo.firstErrorItem = nullptr;
        this->logicErrorInfo.lastErrorItem = nullptr;


        this->memBuffer.init();
        this->memBufferForMalloc2.init();
        this->memBufferForError.init();
        this->memBufferForValueBase.init();
        this->stackMemory.init();

        this->variableMap2 = this->memBuffer.newMem<VoidHashMap>(1);
        this->variableMap2->init(&this->memBuffer);

        this->typeNameMap = this->memBuffer.newMem<VoidHashMap>(1);
        this->typeNameMap->init(&this->memBuffer);
        this->cpuRegister = CPURegister{};
    }




    //------------------------------------------------------------------------------------------
    //
    //                                       Validate Script
    //
    //------------------------------------------------------------------------------------------

    static PrimitiveCalcRegisterEnum findUnusedReg1(PrimitiveCalcRegisterEnum reg1) {
        if (reg1 != PrimitiveCalcRegisterEnum::eax) {
            return PrimitiveCalcRegisterEnum::eax;
        }
        else  {
            return PrimitiveCalcRegisterEnum::ebx;
        }
    }

    static PrimitiveCalcRegisterEnum findUnusedReg2(PrimitiveCalcRegisterEnum reg1, PrimitiveCalcRegisterEnum reg2) {
        if (reg1 != PrimitiveCalcRegisterEnum::eax && reg2 != PrimitiveCalcRegisterEnum::eax) {
            return PrimitiveCalcRegisterEnum::eax;
        }
        else if (reg1 != PrimitiveCalcRegisterEnum::ebx && reg2 != PrimitiveCalcRegisterEnum::ebx) {
            return PrimitiveCalcRegisterEnum::ebx;
        }
        else if (reg1 != PrimitiveCalcRegisterEnum::ecx && reg2 != PrimitiveCalcRegisterEnum::ecx) {
            return PrimitiveCalcRegisterEnum::ecx;
        }

        //assert(reg1 != PrimitiveCalcRegister::edx && reg2 != PrimitiveCalcRegister::edx);
        return PrimitiveCalcRegisterEnum::edx;
    }

    inline void setCalcRegToNode(NodeBase *node, const ScriptEngineContext *context) {
        int typeIndex = node->typeIndex2;
        if (typeIndex == -1) {
            typeIndex = BuiltInTypeIndex::int64;
        }

        auto *typeEntry = context->scriptEnv->typeEntryList[typeIndex];
        auto dataSize = typeEntry->dataSize;
        if (node->typeAtHeap2) {
            dataSize = 8;
        }

        if (dataSize == 4) {
            if (node->calcRegEnum == PrimitiveCalcRegisterEnum::eax) {
                node->calcReg = (st_byte *) &__EX(&context->cpuRegister.rax);
            }
            else if (node->calcRegEnum == PrimitiveCalcRegisterEnum::ebx) {
                node->calcReg = (st_byte *) &__EX(&context->cpuRegister.rbx);
            }
            else if (node->calcRegEnum == PrimitiveCalcRegisterEnum::ecx) {
                node->calcReg = (st_byte *) &__EX(&context->cpuRegister.rcx);
            }
            else if (node->calcRegEnum == PrimitiveCalcRegisterEnum::edx) {
                node->calcReg = (st_byte *) &__EX(&context->cpuRegister.rdx);
            }
        }
        else if (dataSize == 8) {
            if (node->calcRegEnum == PrimitiveCalcRegisterEnum::eax) {
                node->calcReg = (st_byte *) &__RX(&context->cpuRegister.rax);
            }
            else if (node->calcRegEnum == PrimitiveCalcRegisterEnum::ebx) {
                node->calcReg = (st_byte *) &__RX(&context->cpuRegister.rbx);
            }
            else if (node->calcRegEnum == PrimitiveCalcRegisterEnum::ecx) {
                node->calcReg = (st_byte *) &__RX(&context->cpuRegister.rcx);
            }
            else if (node->calcRegEnum == PrimitiveCalcRegisterEnum::edx) {
                node->calcReg = (st_byte *) &__RX(&context->cpuRegister.rdx);
            }
        }
    }

    int applyFunc_assignCalcOpRegister(NodeBase *node, ApplyFunc_params2)
    {
        auto context = (ScriptEngineContext*)scriptEngineContext;

        if (node->vtable == VTables::BinaryOperationVTable) {
            auto *binary = Cast::downcast<BinaryOperationNodeStruct *>(node);

            binary->leftExprNode->calcRegEnum = findUnusedReg1(binary->calcRegEnum);
            setCalcRegToNode(binary->leftExprNode, context);

            binary->rightExprNode->calcRegEnum = findUnusedReg2(binary->calcRegEnum, binary->leftExprNode->calcRegEnum);
            setCalcRegToNode(binary->rightExprNode, context);
        }

        if (node->vtable == VTables::ParenthesesVTable) {
            auto *parentheses = Cast::downcast<ParenthesesNodeStruct *>(node);

            assert(parentheses->valueNode != nullptr);
            parentheses->valueNode->calcRegEnum = parentheses->calcRegEnum;
            setCalcRegToNode(parentheses->valueNode, context);
        }

        if (node->vtable == VTables::AssignStatementVTable) {
            auto *assign = Cast::downcast<AssignStatementNodeStruct *>(node);
            if (assign->valueNode != nullptr) {
                setCalcRegToNode(assign->valueNode, context);
            }
        }

        if (node->vtable == VTables::ReturnStatementVTable) {
            auto *returnState = Cast::downcast<ReturnStatementNodeStruct *>(node);

            if (returnState->valueNode) {
                setCalcRegToNode(returnState->valueNode, context);
            }
        }

/*
        if (node->vtable == VTables::VariableVTable) {
            //node->calcRegEnum = PrimitiveCalcRegisterEnum::eax;
//            node->calcReg = (st_byte*)&__EX(&context->cpuRegister.rax);

            setCalcRegToNode(node, context);
        }
*/
        /*
         * TODO: make calcFunc always returns with same register
        if (node->vtable == VTables::CallFuncVTable) {
            auto *callFunc = Cast::downcast<CallFuncNodeStruct *>(node);

            node->calcResultReg = PrimitiveCalcRegister::eax;
        }
        */
        return 0;
    }


    static void assignCalcOpRegister(ScriptEngineContext *context,
                                     FuncNodeStruct *func)
    {
        func->bodyNode.vtable->applyFuncToDescendants(Cast::upcast(&func->bodyNode),
                                                         (void*)context,
                                                         nullptr,
                                                         applyFunc_assignCalcOpRegister,
                                                         /*parent first*/true,
                                                         nullptr,
                                                         nullptr);
    }
    
    static inline int determineChildTypeIndex(ScriptEnv *env, NodeBase *node) {
        int typeIndex = node->typeIndex2;
        if (typeIndex == -1) {
            typeIndex = env->typeFromNode(node);
            node->typeIndex2 = typeIndex;
        }
        return node->typeIndex2;
    }

    static void validateAssignmentNode(NodeBase *node, void *arg, ScriptEngineContext *context) {
        auto *assign = Cast::downcast<AssignStatementNodeStruct *>(node);
        TypeEntry *typeEntry = nullptr;
        if (assign->hasTypeDecl && !assign->typeOrLet.isLet) {
            char *typeName = NodeUtils::getTypeName(&assign->typeOrLet);
            int typeNameLength = NodeUtils::getTypeNameLength(&assign->typeOrLet);
            typeEntry = (TypeEntry *) context->typeNameMap->get(typeName, typeNameLength);
            if (typeEntry) {
                assign->typeIndex2 = typeEntry->typeIndex;
            }
            else { // NotDefinedType a
                // error no type found
                context->addErrorWithNode(ErrorCode::no_variable_defined, &assign->typeOrLet);
            }
        }
        assign->typeAtHeap2 = assign->pointerAsterisk.found > -1;

        if (assign->hasTypeDecl) {
            if (assign->valueNode) { // int b = 8, let b = 8
                int childTypeIndex = determineChildTypeIndex(context->scriptEnv, assign->valueNode);
                assign->typeAtHeap2 = assign->valueNode->typeAtHeap2;

                if (assign->typeOrLet.isLet) { // let b = 8
                    assign->typeIndex2 = childTypeIndex;

                    if (assign->pointerAsterisk.found > -1) {
                        if (assign->valueNode->typeAtHeap2) {
                        }
                        else {
                            // error: int *b = 8
                        }
                    }
                    else {
                        if (assign->valueNode->typeAtHeap2) {
                            // error: String str = "jfoiwjio"

                        }
                        else {

                        }
                        assign->typeIndex2 = childTypeIndex;
                    }
                }
                else { // int b = 8
                    if (typeEntry) {
                        if (assign->pointerAsterisk.found > -1) { // int *b = null
                            if (typeEntry->typeIndex != childTypeIndex) {
                                if (childTypeIndex == BuiltInTypeIndex::null){
                                    if (assign->typeOrLet.hasNullableMark) {

                                    }
                                    else {
                                        context->addErrorWithNode(ErrorCode::assign_null_to_unnullable, assign);
                                    }
                                }
                                else {
                                    // check assignable
                                    context->addErrorWithNode(ErrorCode::no_variable_defined, assign);
                                }
                            }
                            else {

                            }
                            // error: wrong type
                            //    context->addErrorWithNode(ErrorCode::no_variable_defined, assign);
                        }
                        else {
                            if (typeEntry->typeIndex != childTypeIndex) { // int b = 3.4
                                // error: wrong type
                                context->addErrorWithNode(ErrorCode::no_variable_defined, assign);
                            }
                        }

                    }
                }
            } else {
                if (assign->typeOrLet.isLet) { // let b
                    context->addErrorWithNode(ErrorCode::no_variable_defined, assign);
                }
                else {} // int b
            }
        }
        else { // b = 4
            auto *currentStatement = Cast::downcast<NodeBase*>(arg);
            auto *bodyNode = Cast::downcast<BodyNodeStruct *>(currentStatement->parentNode);
            assert(bodyNode->vtable == VTables::BodyVTable);
            assert(assign->valueNode);

            int childTypeIndex = determineChildTypeIndex(context->scriptEnv, assign->valueNode);
            assign->typeIndex2 = childTypeIndex;
            assign->typeAtHeap2 = assign->valueNode->typeAtHeap2;

            auto *child = bodyNode->firstChildNode;
            bool hit = false;
            while (child) {
                if (child == currentStatement) {
                    break;
                }
                if (child->vtable == VTables::AssignStatementVTable) {
                    auto *declAssign = Cast::downcast<AssignStatementNodeStruct *>(child);
                    if (declAssign->hasTypeDecl) {
                        if (ParseUtil::equal(assign->nameNode.name, assign->nameNode.nameLength,
                                             declAssign->nameNode.name, declAssign->nameNode.nameLength)) {
                            if (!declAssign->typeOrLet.hasMutMark) {
                                context->addErrorWithNode(ErrorCode::assign_to_immutable, assign);
                            }
                            assign->stackOffset = declAssign->stackOffset;
                            hit = true;
                            if (childTypeIndex != declAssign->typeIndex2) {
                                // error
                                context->addErrorWithNode(ErrorCode::no_variable_defined, assign);
                            }
                            if (assign->valueNode->typeAtHeap2 != declAssign->typeAtHeap2) {
                                // error
                                context->addErrorWithNode(ErrorCode::no_variable_defined, assign);
                            }
                        }
                    }
                }
                child = child->nextNode;
            }

            if (!hit) {
                // error: no decl found
                context->addErrorWithNode(ErrorCode::no_variable_defined, assign);
            }
        }
    }

    // children come first
    static int callTypeSelectorOnExpressions(NodeBase *node, ApplyFunc_params2)
    {
        auto *context = (ScriptEngineContext *)scriptEngineContext;

        if (node->vtable == VTables::BinaryOperationVTable) {
            auto *binary = Cast::downcast<BinaryOperationNodeStruct *>(node);

            int leftTypeIndex = determineChildTypeIndex(context->scriptEnv, binary->leftExprNode);
            int rightTypeIndex = determineChildTypeIndex(context->scriptEnv, binary->rightExprNode);
            if (leftTypeIndex > -1 && rightTypeIndex > -1) {
            }
                //auto *leftTypeEntry = context->scriptEnv->typeEntryList[leftTypeIndex];
                //auto *rightTypeEntry = context->scriptEnv->typeEntryList[rightTypeIndex];

//                if (leftTypeIndex == rightTypeIndex) {
                    // currently supports only int32 + int32
            binary->typeIndex2 = leftTypeIndex;
            binary->typeAtHeap2 = binary->leftExprNode->typeAtHeap2;
                //}
        }

        if (node->vtable == VTables::ParenthesesVTable) {
            auto *parentheses = Cast::downcast<ParenthesesNodeStruct *>(node);
            if (parentheses->valueNode) {
                parentheses->typeIndex2 = determineChildTypeIndex(context->scriptEnv, parentheses->valueNode);
                parentheses->typeAtHeap2 = parentheses->valueNode->typeAtHeap2;
            }
        }

        if (node->vtable == VTables::ReturnStatementVTable) {
            auto* returnState = Cast::downcast<ReturnStatementNodeStruct*>(node);
            if (returnState->valueNode) {
                returnState->typeIndex2 = determineChildTypeIndex(context->scriptEnv, returnState->valueNode);
                returnState->typeAtHeap2 = returnState->valueNode->typeAtHeap2;
            }
        }

        if (node->vtable == VTables::AssignStatementVTable) {
            validateAssignmentNode(node, arg, context);

        }

        if (node->vtable == VTables::VariableVTable) {
            auto *currentStatement = Cast::downcast<NodeBase*>(arg);
            auto *bodyNode = Cast::downcast<BodyNodeStruct *>(currentStatement->parentNode);
            assert(bodyNode->vtable == VTables::BodyVTable);

            auto *vari = Cast::downcast<VariableNodeStruct*>(node);

            auto *child = bodyNode->firstChildNode;
            while (child) {
                if (child == currentStatement) {
                    break;
                }
                if (child->vtable == VTables::AssignStatementVTable) {
                    auto *declAssign = Cast::downcast<AssignStatementNodeStruct *>(child);
                    if (declAssign->hasTypeDecl) {
                        if (ParseUtil::equal(vari->name, vari->nameLength,
                                             declAssign->nameNode.name, declAssign->nameNode.nameLength)) {
                            vari->stackOffset = declAssign->stackOffset;
                            vari->typeIndex2 = declAssign->typeIndex2;
                            vari->typeAtHeap2 = declAssign->typeAtHeap2;
                        }
                    }
                }
                child = child->nextNode;
            }
        }

        return 0; // varDefFound ? 1 : 0;
    }

    static void callTypeSelectorsOnExpressions2(ScriptEngineContext *context, FuncNodeStruct *func)
    {
        auto *child = func->bodyNode.firstChildNode;
        int currentStackOffset = 0;
        while (child) {
            child->vtable->applyFuncToDescendants(
                    child,
                    (void*)context,
                    nullptr,
                    callTypeSelectorOnExpressions,
                    /*children first*/false,
                    (void *) child,
                    nullptr);

            if (child->vtable == VTables::AssignStatementVTable) {
                auto* assign = Cast::downcast<AssignStatementNodeStruct*>(child);
                if (assign->hasTypeDecl && assign->typeIndex2 > -1) {
                    if (assign->typeAtHeap2) {
                        currentStackOffset -= 8;
                    }
                    else {
                        currentStackOffset -= context->scriptEnv->typeEntryList[assign->typeIndex2]->dataSize;
                    }
                    assign->stackOffset = currentStackOffset;
                }
            }

            child = child->nextNode;
        }
        func->stackSize = -currentStackOffset;
    }

    static FuncNodeStruct* findMainFunc(DocumentStruct* document)
    {
        auto* rootNode = document->firstRootNode;
        while (rootNode != nullptr) {
            if (rootNode->vtable == VTables::FnVTable) {
                // fn
                auto* fnNode = Cast::downcast<FuncNodeStruct*>(rootNode);
                auto* nameNode = &fnNode->nameNode;
                if (ParseUtil::equal(nameNode->name, nameNode->nameLength, "main", 4))
                {
                    return fnNode;
                }
            }
            rootNode = rootNode->nextNode;
        }
        return nullptr;
    }



    void ScriptEnv::validateScript()
    {
        assert(this->document->context->syntaxErrorInfo.hasError == false);

        // search all funcs
        auto *rootNode = document->firstRootNode;
        while (rootNode != nullptr) {
            if (rootNode->vtable == VTables::FnVTable) {
                // fn
                auto *fnNode = Cast::downcast<FuncNodeStruct*>(rootNode);
                int errorCount = this->context->logicErrorInfo.count;

                // set typeIndex to all expressions and assignments
                callTypeSelectorsOnExpressions2(context, fnNode);

                if (errorCount == this->context->logicErrorInfo.count) {
                    assignCalcOpRegister(context, fnNode);
                }
            }
            rootNode = rootNode->nextNode;
        }

        this->mainFunc = findMainFunc(this->document);
    }



    //------------------------------------------------------------------------------------------
    //
    //                                      evaluateExprNode
    //
    //------------------------------------------------------------------------------------------

    void ScriptEngineContext::setCalcRegister(NodeBase* expressionNode) {

    }

    void ScriptEngineContext::evaluateExprNode(NodeBase *expressionNode)
    {
        assert(expressionNode != nullptr);
        assert(expressionNode->vtable != nullptr);

        if (expressionNode->vtable == VTables::VariableVTable) {
            auto* variableNode = Cast::downcast<VariableNodeStruct *>(expressionNode);
            if (variableNode->typeAtHeap2) {
                this->stackMemory.moveFrom(variableNode->stackOffset, 8, variableNode->calcReg);
            } else {
                auto dataSize = this->scriptEnv->typeEntryList[variableNode->typeIndex2]->dataSize;
                this->stackMemory.moveFrom(variableNode->stackOffset, dataSize, variableNode->calcReg);
            }
            return;
        }
        if (expressionNode->vtable == VTables::ParenthesesVTable) {
            auto* parentheses = Cast::downcast<ParenthesesNodeStruct *>(expressionNode);
            evaluateExprNode(parentheses->valueNode);
            return;
        }

        // a + (b + c)
        if (expressionNode->vtable == VTables::BinaryOperationVTable) {
            auto* binaryNode = Cast::downcast<BinaryOperationNodeStruct *>(expressionNode);

            this->evaluateExprNode(binaryNode->rightExprNode);
            uint64_t saved = *(uint64_t*)binaryNode->rightExprNode->calcReg;
            this->evaluateExprNode(binaryNode->leftExprNode);
            *(uint64_t*)binaryNode->rightExprNode->calcReg = saved;

            auto *leftTypeEntry = this->scriptEnv->typeEntryList[binaryNode->leftExprNode->typeIndex2];
            leftTypeEntry->binary_operate(this, binaryNode, false);
            return;
        }

        TypeEntry *typeEntry = nullptr;
        int typeIndex = expressionNode->typeIndex2;
        if (typeIndex > 0) {
            typeEntry = this->scriptEnv->typeEntryList[typeIndex];
        }

        if (typeEntry) {
            typeEntry->evaluateNode(this, expressionNode);
        }

        /*
        if (expressionNode->vtable == VTables::StringLiteralVTable) {
            //auto* strNode = Cast::downcast<StringLiteralNodeStruct *>(expressionNode);
            //int size = (1 + strNode->strLength) * (int)sizeof(char);
//          memcpy(chars, strNode->str, strNode->strLength);
//          chars[strNode->strLength] = '\0';
            return;
        }
        if (expressionNode->vtable == VTables::NumberVTable) {
            auto* numberNode = Cast::downcast<NumberNodeStruct *>(expressionNode);
            if (numberNode->unit == 64) {
                *(int64_t*)numberNode->calcReg = numberNode->num;
            }
            else {
                *(int32_t*)numberNode->calcReg = (int32_t)numberNode->num;
            }
            return;
        }

        if (expressionNode->vtable == VTables::NullVTable) {
            auto* nullNode = Cast::downcast<NullNodeStruct *>(expressionNode);
            *(int64_t*)nullNode->calcReg = 0;
            return;
        }
*/






        if (expressionNode->vtable == VTables::CallFuncVTable) {
            //auto *funcCall = Cast::downcast<CallFuncNodeStruct *>(expressionNode);
            //auto *valueBase = this->evaluateExprNode(funcCall->exprNode);
            // proceed Stack
        }
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

            scriptEnv->context = context;
            context->init(scriptEnv);

            scriptEnv->mainFunc = nullptr;
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


    static int executeMain(ScriptEnv* env, FuncNodeStruct* mainFunc)
    {
        int stackSize = mainFunc->stackSize;
        env->context->stackMemory.call();
        env->context->stackMemory.localVariables(stackSize);

        auto* statementNode = mainFunc->bodyNode.firstChildNode;
        while (statementNode)
        {
            // call func: funcA(100)
            if (statementNode->vtable == VTables::CallFuncVTable) {
                auto* funcCall = Cast::downcast<CallFuncNodeStruct*>(statementNode);
                auto* arg = funcCall->firstArgumentItem;
                if (arg != nullptr) {
                    while (true) {
                        printf("arg = <%s>\n", arg->exprNode->vtable->typeChars);
                        env->context->evaluateExprNode(arg->exprNode);
                        //auto *chars = env->typeEntryList[valueBase->typeIndex]->toString(env->context, valueBase);
                        //printf("chars = [%s]\n", chars);

                        if (arg->nextNode == nullptr) {
                            break;
                        }
                        arg = Cast::downcast<FuncArgumentItemStruct*>(arg->nextNode);
                    }
                }
            }

            // assign: int a = 3
            if (statementNode->vtable == VTables::AssignStatementVTable) {
                auto* assignStatement = Cast::downcast<AssignStatementNodeStruct *>(statementNode);
                if (assignStatement->valueNode) {
                    env->context->evaluateExprNode(assignStatement->valueNode);
                    // if (valueBase->typeIndex == BuiltInTypeIndex::int32) {}
                    auto *typeEntry = env->typeEntryList[assignStatement->typeIndex2];
                    auto dataSize = typeEntry->dataSize; // env->typeEntryList[valueBase->typeIndex]->dataSize;
                    if (assignStatement->typeAtHeap2) {
                        dataSize = 8;
                    }

                    //env->context->stackMemory.moveTo(assignStatement->stackOffset, dataSize,  (char*)valueBase->ptr);
                    env->context->stackMemory.moveTo(assignStatement->stackOffset, dataSize,
                                                     assignStatement->valueNode->calcReg);
                }
            }

            // return 3
            if (statementNode->vtable == VTables::ReturnStatementVTable) {
                auto* returnNode = Cast::downcast<ReturnStatementNodeStruct*>(statementNode);
                env->context->evaluateExprNode(returnNode->valueNode);
                auto* typeEntry = env->typeEntryList[returnNode->valueNode->typeIndex2];

                if (typeEntry->dataSize == 8 || returnNode->valueNode->typeAtHeap2) {
                    int64_t v = *(int64_t*)returnNode->valueNode->calcReg;
                    return (int32_t)v;
                }
                else {
                    return *(int32_t*)returnNode->valueNode->calcReg;
                }
            }

            statementNode = statementNode->nextNode;
        }

        env->context->stackMemory.ret();

        return 0;
    }


    _ScriptEnv* ScriptEnv::loadScript(char* script, int byteLength)
    {
        ScriptEnv* env = ScriptEnv::newScriptEnv();
        setupBuiltInTypeSelectors(env);

        auto* document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
        DocumentUtils::parseText(document, script, byteLength);
        DocumentUtils::generateHashTables(document);
        env->document = document;

        return env;
    }



    int ScriptEnv::runScriptEnv()
    {
        assert(this->document->context->syntaxErrorInfo.hasError == false);
        assert(this->context->logicErrorInfo.hasError == false);

        int ret = 0;
        auto* mainFunc2 = this->mainFunc;
        if (mainFunc2) {
            printf("main Found");
            printf("<%s()>\n", mainFunc2->nameNode.name);
            ret = executeMain(this, mainFunc2);
        }

        auto* rootNode = this->document->firstRootNode;
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

        Alloc::deleteDocument(this->document);
        ScriptEnv::deleteScriptEnv(this);
        return ret;
    }


    int ScriptEnv::startScript(char* script, int scriptLength)
    {
        ScriptEnv *env = ScriptEnv::loadScript(script, scriptLength);
        if (env->document->context->syntaxErrorInfo.hasError) {
            return env->document->context->syntaxErrorInfo.errorItem.errorId;
        }

        env->validateScript();
        if (env->context->logicErrorInfo.hasError) {
            env->context->setErrorPositions();
            return env->context->logicErrorInfo.firstErrorItem->codeErrorItem.errorId;
        }

        return env->runScriptEnv();
    }
}
