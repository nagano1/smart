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
        return BuiltInTypeIndex::int32;
    }

    static int selectTypeFromStringNode(ScriptEnv *env, StringLiteralNodeStruct *nodeBase)
    {
        return BuiltInTypeIndex::heapString;
    }

    static int selectTypeFromParentheses(ScriptEnv *env, ParenthesesNodeStruct *parenthesis)
    {
        if (parenthesis->valueNode) {
            return env->typeFromNode(parenthesis->valueNode);
        }
        return -1;
    }

    template<typename T>
    static void setTypeSelector(const node_vtable* vtable, int (*argToType)(ScriptEnv *, T *)) {
        ((node_vtable*)vtable)->typeSelector = reinterpret_cast<int (*)(void *, NodeBase *)>(argToType);
    }

    static void setupBuiltInTypeSelectors(ScriptEnv *env)
    {
        setTypeSelector(VTables::NumberVTable, selectTypeFromNumberNode);
        setTypeSelector(VTables::StringLiteralVTable, selectTypeFromStringNode);
        setTypeSelector(VTables::ParenthesesVTable, selectTypeFromParentheses);

        if (VTables::NumberVTable->typeSelector(env, nullptr) != -1) {
        }
    }

    //------------------------------------------------------------------------------------------
    //
    //                                      Node to Value
    //
    //------------------------------------------------------------------------------------------

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
        int lineNumber = 1;
        auto *line = docStruct->firstCodeLine;
        while (line) {
            line->lineNumber = lineNumber++;
            line = line->nextLine;
        }
    }

    void ScriptEngineContext::setErrorPositions()
    {
        reassignLineNumbers(this->scriptEnv->document);

        this->errorDetectRevision += 1;

        //auto *lineCode = this->scriptEnv->document->context->newCodeLine();

        auto *errorItem = this->logicErrorInfo.firstErrorItem;
        while (errorItem) {
            auto *node = errorItem->node;
            node->useErrorInfoRevisionIndex = this->errorDetectRevision;


            //VTableCall::callAppendToLine(node, lineCode);
            //auto *errorInfo = errorItem->codeErrorItem;
            errorItem = errorItem->next;
        }

        // DocumentUtils::regenerateCodeLines(docStruct);


        /*
         *
         *         if (currentCodeLine->context->errorDetectRevision == nodeBase->useErrorInfoRevisionIndex) {

            //nodeBase->line2;
        }
        //if (nodeBase->prevLineBreakNode)
        //static_assert(false, "oifejwoa");
         *
         */


        // *charactor = ParseUtil::utf16_length(text + lineFirstPos, currentCharactor);
        //static_assert(false, "not implemented");
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

    void ScriptEngineContext::init(ScriptEnv *scriptEnvArg)
    {
        this->scriptEnv = scriptEnvArg;
        this->logicErrorInfo.hasError = false;
        this->logicErrorInfo.firstErrorItem = nullptr;
        this->logicErrorInfo.lastErrorItem = nullptr;

        this->errorDetectRevision = 0;


        this->memBuffer.init();
        this->memBufferForMalloc2.init();
        this->memBufferForError.init();
        this->memBufferForValueBase.init();
        this->stackMemory.init();

        this->variableMap2 = this->memBuffer.newMem<VoidHashMap>(1);
        this->variableMap2->init(&this->memBuffer);

        this->typeNameMap = this->memBuffer.newMem<VoidHashMap>(1);
        this->typeNameMap->init(&this->memBuffer);

        /*
        this->logicErrorInfo.errorCode = ErrorCode::no_logical_error;
        this->logicErrorInfo.errorId = 57770000;
        this->logicErrorInfo.charPosition = -1;
        this->logicErrorInfo.charPosition2 = -1;
         */
    }


    //------------------------------------------------------------------------------------------
    //
    //                                       Validate Script
    //
    //------------------------------------------------------------------------------------------

    static int variableFound_SearchCorrespondAssignment(NodeBase *node, void *scriptcontext, void *targetVTable, void *func, void* arg, void *arg2)
    {
        auto *context2 = (ScriptEngineContext *)scriptcontext;
        auto *vari = Cast::downcast<VariableNodeStruct *>(node);
        vari->stackOffset2 = -1;

        auto *variableTop = Cast::downcast<NodeBase*>(arg);
        auto *body = Cast::downcast<BodyNodeStruct*>(variableTop->parentNode);
        // auto *funcNode = Cast::downcast<FuncNodeStruct*>(body->parentNode);

        auto *bodyChild = body->firstChildNode;
        bool varDefFound = false;

        while (bodyChild) {
            if (bodyChild == variableTop) {
                break;
            }

            if (bodyChild->vtable == VTables::AssignStatementVTable) {
                auto *assignState = Cast::downcast<AssignStatementNodeStruct *>(bodyChild);

                if (ParseUtil::equal(vari->name, vari->nameLength,
                                     assignState->nameNode.name, assignState->nameNode.nameLength)) {
                    vari->stackOffset2 = assignState->stackOffset;
                    varDefFound = true;
                }
            }

            bodyChild = bodyChild->nextNode;
        }

        if (!varDefFound) {
            context2->addErrorWithNode(ErrorCode::no_variable_defined, Cast::upcast(vari));
        }

        return varDefFound ? 1 : 0;
    }


    static int validateFunc(ScriptEngineContext *context, FuncNodeStruct *func)
    {
        auto *child = func->bodyNode.firstChildNode;
        while (child) {
            child->vtable->applyFuncToDescendants(
                    child,
                    (void*)context,
                    (void *) VTables::VariableVTable,
                                                  variableFound_SearchCorrespondAssignment,
                                                  (void *) child,
                                                  0);
            child = child->nextNode;
        }

        return 0;
    }


    int ScriptEnv::validateScript() const
    {
        assert(this->document->context->syntaxErrorInfo.hasError == false);
        int errorCode = 0;

        // search all funcs
        auto *rootNode = document->firstRootNode;
        while (rootNode != nullptr) {
            if (rootNode->vtable == VTables::FnVTable) {
                // fn
                auto *fnNode = Cast::downcast<FuncNodeStruct*>(rootNode);
                int thisErrorCode = validateFunc(this->context, fnNode);
                if (thisErrorCode > 0) {
                    errorCode = thisErrorCode;
                }
            }
            rootNode = rootNode->nextNode;
        }

        return errorCode;
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



    static void validateFunc(ScriptEnv* env, FuncNodeStruct* func) {

    }


    int setStackOffsetToVariable(NodeBase *node, void *context, void *targetVTable, void *func, void* arg, void *arg2) {
        auto *vari = Cast::downcast<VariableNodeStruct *>(node);
        auto *assignment = Cast::downcast<AssignStatementNodeStruct*>(arg);

        if (ParseUtil::equal(vari->name, vari->nameLength,
                             assignment->nameNode.name, assignment->nameNode.nameLength)) {
            vari->stackOffset2 = assignment->stackOffset;
        }
        return 0;
    }


    static void setStackOffsetToVariables(
                                ScriptEngineContext *context,
                                FuncNodeStruct *func,
                                const AssignStatementNodeStruct *assignment,
                                _NodeBase *currentStatement)
    {
        auto* statementNode = currentStatement->nextNode;
        while (statementNode) {
            statementNode->vtable->applyFuncToDescendants(Cast::upcast(statementNode),
                                                          (void*)context,
                                                          (void *) VTables::VariableVTable,
                                                          setStackOffsetToVariable,
                                                          (void *) assignment,
                                                          nullptr);

            statementNode = statementNode->nextNode;
        }
    }


    static int calcStackSizeInFunc(ScriptEnv* env, FuncNodeStruct* func) // NOLINT(readability-function-cognitive-complexity)
    {
        auto* statementNode = func->bodyNode.firstChildNode;
        int stackSize = 0;
        int currentStackOffset = 0;

        while (statementNode) { // NOLINT(altera-id-dependent-backward-branch,altera-unroll-loops)
            if (statementNode->vtable == VTables::AssignStatementVTable) {
                auto *assignment = Cast::downcast<AssignStatementNodeStruct *>(statementNode);
                if (assignment->hasTypeDecl) {
                    auto &typeName = assignment->typeOrLet.nameNode;
                    bool isLet = assignment->typeOrLet.isLet;

                    TypeEntry *typeEntry = nullptr;
                    TypeEntry *rightValueTypeEntry = nullptr;
                    // bool isInt = ParseUtil::equal(typeName.name, typeName.nameLength, "int", 3);

                    // validate
                    if (assignment->valueNode) {
                        int typeIndex = env->typeFromNode(assignment->valueNode);
                        if (typeIndex > -1) {
                            rightValueTypeEntry = env->typeEntryList[typeIndex];
                        }
                    }

                    if (isLet) {
                        if (rightValueTypeEntry) {
                            typeEntry = rightValueTypeEntry;
                        }
                        else {
                            // error
                        }
                    }
                    else {
                        // get typeEntry by type name
                        typeEntry = (TypeEntry *) env->context->typeNameMap->get(
                                typeName.name, typeName.nameLength);
                    }

                    if (typeEntry) {
                        //printf("<test %d>", typeEntry->stackSize);
                        assignment->stackOffset = currentStackOffset;
                        stackSize += typeEntry->stackSize;
                        currentStackOffset += typeEntry->stackSize;

                        setStackOffsetToVariables(env->context, func, assignment, statementNode);
                    }
                    else {
                        // type resolve error
                        env->context->addErrorWithNode(ErrorCode::type_not_found,
                                                       Cast::upcast(assignment));
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
        auto *mainFunc = findMainFunc(this->document);
        if (mainFunc) {
            printf("main Found");
            printf("<%s()>\n", mainFunc->nameNode.name);
            ret = executeMain(this, mainFunc);
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
            return env->context->logicErrorInfo.firstErrorItem->codeErrorItem.errorId;
        }

        return env->runScriptEnv();
    }
}