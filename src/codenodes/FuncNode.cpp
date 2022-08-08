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

#include "code_nodes.hpp"

namespace smart {

    static constexpr const char fn_chars[] = "fn";
    static constexpr const char fn_first_char = fn_chars[0];
    static constexpr unsigned int size_of_fn = sizeof(fn_chars) - 1;





    // -----------------------------------------------------------------------------------
    //
    //                              FuncArgumentItemStruct
    //
    // -----------------------------------------------------------------------------------
    static CodeLine *FuncArgument_appendToLine2(FuncArgumentItemStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);

        currentCodeLine->appendNode(self);

        if (self->valueNode) {
            currentCodeLine = VTableCall::appendToLine(self->valueNode, currentCodeLine);
        }

        if (self->hasComma) {
            currentCodeLine = VTableCall::appendToLine(&self->follwingComma, currentCodeLine);
        }

        return currentCodeLine;
    };


    static const utf8byte *FuncArgument_selfText(FuncArgumentItemStruct *) {
        return "";
    }

    static int FuncArgument_selfTextLength2(FuncArgumentItemStruct *) {
        return 0;
    }


    static const node_vtable _funcArgumentItemVTable = CREATE_VTABLE(FuncArgumentItemStruct,
                                                                     FuncArgument_selfTextLength2,
                                                                     FuncArgument_selfText,
                                                                     FuncArgument_appendToLine2,
                                                                     "<FuncArgument>",
                                                                     NodeTypeId::FuncArgument);

    const struct node_vtable *const VTables::FuncArgumentVTable  = &_funcArgumentItemVTable;

    FuncArgumentItemStruct *Alloc::newFuncArgumentItem(ParseContext *context, NodeBase *parentNode) {
        auto *keyValueItem = context->newMem<FuncArgumentItemStruct>();

        INIT_NODE(keyValueItem, context, parentNode, &_funcArgumentItemVTable);

        Init::initSymbolNode(&keyValueItem->follwingComma, context, keyValueItem, ',');

        keyValueItem->hasComma = false;
        keyValueItem->valueNode = nullptr;

        return keyValueItem;
    }





    //    +--------------------------+
    //    | CallFunc Node            |
    //    +--------------------------+
    static CodeLine *callfunc_appendToLine(CallFuncNodeStruct *self, CodeLine *currentCodeLine)
    {
        // currentCodeLine = currentCodeLine->addPrevLineBreakNode(self->valueNode);

        if (self->valueNode) {
            currentCodeLine = VTableCall::appendToLine(self->valueNode, currentCodeLine);
        }

        //int formerParentDepth = self->context->parentDepth;
        //self->context->parentDepth += 1;
        //self->context->parentDepth = formerParentDepth;

        currentCodeLine = VTableCall::appendToLine(&self->openNode, currentCodeLine);

        self->context->parentDepth += 1;

        auto *item = self->firstArgumentItem;
        while (item != nullptr) { // NOLINT(altera-unroll-loops,altera-id-dependent-backward-branch)
            currentCodeLine = VTableCall::appendToLine(item, currentCodeLine);
            item = Cast::downcast<FuncArgumentItemStruct *>(item->nextNode);
        }

        self->context->parentDepth -= 1;


        currentCodeLine = VTableCall::appendToLine(&self->closeNode2, currentCodeLine);

        return currentCodeLine;
    }


    // virtual node
    static const char *callfunc_selfText(CallFuncNodeStruct *self)
    {
        return "";
    }

    static int callfun_selfTextLength(CallFuncNodeStruct *self)
    {
        return 0;
    }


    static constexpr const char callfuncNodeTypeText[] = "<FuncCall>";


    static inline void appendRootNode(CallFuncNodeStruct *arr, FuncArgumentItemStruct *arrayItem) {
        assert(arr != nullptr && arrayItem != nullptr);

        if (arr->firstArgumentItem == nullptr) {
            arr->firstArgumentItem = arrayItem;
        }
        if (arr->lastArgumentItem != nullptr) {
            arr->lastArgumentItem->nextNode = Cast::upcast(arrayItem);
        }
        arr->lastArgumentItem = arrayItem;
    }


    enum phase {
        EXPECT_VALUE = 0,
        EXPECT_COMMA = 3
    };



    static inline int parseNextValue(TokenizerParams_parent_ch_start_context, CallFuncNodeStruct* funcCallNode)
    {
        int result;
        if (-1 < (result = Tokenizers::jsonValueTokenizer2(TokenizerParams_pass))) {
            auto *nextItem = Alloc::newFuncArgumentItem(context, parent);

            nextItem->valueNode = context->virtualCodeNode;
            appendRootNode(funcCallNode, nextItem);
            funcCallNode->parsePhase = phase::EXPECT_COMMA;
            return result;
        }
        return -1;
    }


    static int inner_returnStatementTokenizerMulti(TokenizerParams_parent_ch_start_context) {
        auto *funcCallNode = Cast::downcast<CallFuncNodeStruct*>(parent);

        if (ch == ')') {
            context->setCodeNode(&funcCallNode->closeNode2);
            context->scanEnd = true;
            return start + 1;
        }

        if (funcCallNode->parsePhase == phase::EXPECT_VALUE) {
            return parseNextValue(TokenizerParams_pass, funcCallNode);
        }

        auto *currentKeyValueItem = funcCallNode->lastArgumentItem;

        if (funcCallNode->parsePhase == phase::EXPECT_COMMA) {
            if (ch == ',') { // try to find ',' which leads to next key-value
                currentKeyValueItem->hasComma = true;
                context->setCodeNode(&currentKeyValueItem->follwingComma);
                funcCallNode->parsePhase = phase::EXPECT_VALUE;
                return start + 1;
            } else if (context->afterLineBreak) {
                // comma is not required after a line break
                return parseNextValue(TokenizerParams_pass, funcCallNode);
            }
            return -1;
        }
        return -1;
    }


    int Tokenizers::funcCallTokenizer(TokenizerParams_parent_ch_start_context)
    {
        if ('(' == ch) {
            assert(context->virtualCodeNode != null);

            auto *funcCallNode = Alloc::newFuncCallNode(context, parent);

            funcCallNode->valueNode = context->virtualCodeNode;
            funcCallNode->valueNode->parentNode = Cast::upcast(funcCallNode);

            auto *leftNode = context->leftNode;

            int currentPos = start + 1;
            int resultPos;
            if (-1 < (resultPos = Scanner::scanMulti(funcCallNode,
                                                     inner_returnStatementTokenizerMulti,
                                                     currentPos, context))) {

                context->virtualCodeNode = Cast::upcast(funcCallNode);
                context->leftNode = leftNode;
                return resultPos;
            }
        }

        return -1;
    }


    static const node_vtable _callfuncVTable = CREATE_VTABLE(CallFuncNodeStruct,
                                                             callfun_selfTextLength,
                                                             callfunc_selfText,
                                                             callfunc_appendToLine,
                                                             callfuncNodeTypeText,
                                                             NodeTypeId::CallFunc);

    const node_vtable *const VTables::CallFuncVTable = &_callfuncVTable;


    CallFuncNodeStruct *Alloc::newFuncCallNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *node = context->newMem<CallFuncNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::CallFuncVTable);
        node->valueNode = nullptr;
        node->parsePhase = phase::EXPECT_VALUE;

        Init::initSymbolNode(&node->openNode, context, node, '(');
        Init::initSymbolNode(&node->closeNode2, context, node, ')');

        node->firstArgumentItem = nullptr;
        node->lastArgumentItem = nullptr;

        return node;
    }


























    //    +--------------------------+
    //    | BodyNode                 |
    //    +--------------------------+
    static int selfTextLength2(BodyNodeStruct *) {
        return 1;
    }

    static const utf8byte *selfText2(BodyNodeStruct *) {
        return "{";
    }

    static CodeLine *appendToLine2(BodyNodeStruct *self, CodeLine *currentCodeLine) {
        auto *classNode = self;


        currentCodeLine = currentCodeLine->addPrevLineBreakNode(classNode);

        currentCodeLine->appendNode(self);


        auto formerParentDepth = self->context->parentDepth;
        self->context->parentDepth += 1;

        {
            auto *child = classNode->firstChildNode;
            while (child) {
                currentCodeLine = VTableCall::appendToLine(child, currentCodeLine);
                child = child->nextNode;
            }
        }


        auto *prevCodeLine = currentCodeLine;
        currentCodeLine = VTableCall::appendToLine(&classNode->endBodyNode, currentCodeLine);

        if (prevCodeLine != currentCodeLine) {
            currentCodeLine->depth = formerParentDepth + 1;
        }

        self->context->parentDepth = formerParentDepth;


        return currentCodeLine;
    }


    static constexpr const char bodyTypeText[] = "<body>";

/*
 * static low fn A<T>(a: int, b: String) {
 *
 * }
 */
    static const node_vtable _bodyVTable = CREATE_VTABLE(BodyNodeStruct,
                                                         selfTextLength2,
                                                         selfText2,
                                                         appendToLine2,
                                                         bodyTypeText, NodeTypeId::Body);

    const struct node_vtable *const VTables::BodyVTable = &_bodyVTable;

    void Init::initBodyNode(BodyNodeStruct *node, ParseContext *context, void *parentNode) {
        INIT_NODE(node, context, parentNode, VTables::BodyVTable);

        node->lastChildNode = nullptr;
        node->firstChildNode = nullptr;
        node->childCount = 0;
        node->startFound = false;
        node->firstStatementFound = false;

        Init::initSymbolNode(&node->bodyStartNode, context, node, '{');
        Init::initSymbolNode(&node->endBodyNode, context, node, '}');
    }


    static void appendChildNode(BodyNodeStruct *body, NodeBase *node) {
        if (body->firstChildNode == nullptr) {
            body->firstChildNode = node;
        }
        if (body->lastChildNode != nullptr) {
            body->lastChildNode->nextNode = node;
        }
        body->lastChildNode = node;
        body->childCount++;
    }

    static int inner_bodyTokenizerMulti(TokenizerParams_parent_ch_start_context) {
        auto *body = Cast::downcast<BodyNodeStruct *>(parent);
        if (ch == '}') {
            context->scanEnd = true;
            context->setCodeNode(&body->endBodyNode);
            return start + 1;
        } else {
            if (!body->firstStatementFound || context->afterLineBreak) {
                body->firstStatementFound = true;
                int nextPos;
                if (-1 <
                    (nextPos = Tokenizers::assignStatementTokenizer(parent, ch, start, context))) {
                    appendChildNode(body, context->virtualCodeNode);
                    return nextPos;
                }
                else if (-1 <
                           (nextPos = Tokenizers::assignStatementWithoutLetTokenizer(parent, ch,
                                                                                     start,
                                                                                     context))) {
                    appendChildNode(body, context->virtualCodeNode);
                    return nextPos;
                }
                else if (-1 <
                         (nextPos = Tokenizers::returnStatementTokenizer(parent, ch, start, context))) {
                    appendChildNode(body, context->virtualCodeNode);
                    return nextPos;
                }
                else {
                    // value as a statement
                    int result;
                    if (-1 < (result = Tokenizers::valueTokenizer(TokenizerParams_pass))) {
                        appendChildNode(body, context->virtualCodeNode);
                        return result;
                    }
                }
            } else {
                context->setError(ErrorCode::should_break_line, start);
            }
        }

        context->setError(ErrorCode::syntax_error2, start);
        context->scanEnd = true;
        return -1;
    }

    int Tokenizers::bodyTokenizer(TokenizerParams_parent_ch_start_context) {
        auto *bodyNode = Cast::downcast<BodyNodeStruct *>(parent);

        if (ch == '{') {
            int returnPosition = start + 1;
            int result = Scanner::scanMulti(bodyNode,
                                            inner_bodyTokenizerMulti,
                                            returnPosition,
                                            context);

            if (result > -1) {
                context->setCodeNode(bodyNode);
                return result;
            }
        } else {
            context->setError(ErrorCode::expect_bracket_for_fn_body, context->prevFoundPos);
        }
        return -1;
    };



















    // --------------------- Defines FuncNode VTable ---------------------- /




    // -----------------------------------------------------------------------------------
    //
    //                              FuncCallParameterItemStruct
    //
    // -----------------------------------------------------------------------------------
    /*
    static CodeLine *appendToLine2(FuncParameterItemStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);

        currentCodeLine->appendNode(self);

        if (self->valueNode) {
            currentCodeLine = VTableCall::appendToLine(self->valueNode, currentCodeLine);
        }

        if (self->hasComma) {
            currentCodeLine = VTableCall::appendToLine(&self->follwingComma, currentCodeLine);
        }

        return currentCodeLine;
    };


    // virtual
    static const utf8byte *selfText_JsonKeyValueItemStruct(FuncParameterItemStruct *) {
        return "";
    }

    static int selfTextLength2(FuncParameterItemStruct *) {
        return 0;
    }


    static const node_vtable _jsonArrayItemVTable = CREATE_VTABLE(FuncParameterItemStruct,
                                                                  selfTextLength2,
                                                                  selfText_JsonKeyValueItemStruct,
                                                                  appendToLine2,
                                                                  "<FuncParameterItem>",
                                                                  NodeTypeId::FuncParameter);

    const struct node_vtable *const VTables::JsonArrayItemVTable = &_jsonArrayItemVTable;


    FuncParameterItemStruct *Alloc::newFuncParameterItem(ParseContext *context, NodeBase *parentNode) {
        auto *keyValueItem = context->newMem<FuncParameterItemStruct>();

        INIT_NODE(keyValueItem, context, parentNode, &_jsonArrayItemVTable);

        Init::initSymbolNode(&keyValueItem->follwingComma, context, keyValueItem, ',');

        keyValueItem->hasComma = false;
        keyValueItem->valueNode = nullptr;

        return keyValueItem;
    }
*/
















    //    +--------------------------+
    //    | FuncDefNode                 |
    //    +--------------------------+

    static int selfTextLength(FuncNodeStruct *) {
        return size_of_fn;
    }

    static const utf8byte *selfText(FuncNodeStruct *) {
        return fn_chars;
    }

    static CodeLine *appendToLine(FuncNodeStruct *self, CodeLine *currentCodeLine) {
        auto *classNode = self;


        currentCodeLine = currentCodeLine->addPrevLineBreakNode(classNode);

        currentCodeLine->appendNode(classNode);


        auto formerParentDepth = self->context->parentDepth;
        self->context->parentDepth += 1;
        currentCodeLine = VTableCall::appendToLine(&classNode->nameNode, currentCodeLine);
        self->context->parentDepth = formerParentDepth;




        currentCodeLine = VTableCall::appendToLine(&classNode->parameterStartNode, currentCodeLine);
        currentCodeLine = VTableCall::appendToLine(&classNode->parameterEndNode, currentCodeLine);

        currentCodeLine = VTableCall::appendToLine(&classNode->bodyNode, currentCodeLine);

        return currentCodeLine;
    };


    static constexpr const char fnTypeText[] = "<fn>";

/*
 * static low fn A<T>(a: int, b: String) {
 *
 *
 * }
 */
    static const node_vtable _fnVTable = CREATE_VTABLE(FuncNodeStruct,
                                                       selfTextLength,
                                                       selfText,
                                                       appendToLine,
                                                       fnTypeText, NodeTypeId::Func);

    const struct node_vtable *const VTables::FnVTable = &_fnVTable;

    FuncNodeStruct* Alloc::newFuncNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *funcNode = context->newMem<FuncNodeStruct>();

        INIT_NODE(funcNode, context, parentNode, &_fnVTable);

        funcNode->lastChildParameterNode = nullptr;
        funcNode->firstChildParameterNode = nullptr;


        Init::initNameNode(&funcNode->nameNode, context, funcNode);

        Init::initSymbolNode(&funcNode->parameterStartNode, context, funcNode, '(');
        Init::initSymbolNode(&funcNode->parameterEndNode, context, funcNode, ')');

        Init::initBodyNode(&funcNode->bodyNode, context, funcNode);

        return funcNode;
    }


    // --------------------- Implements ClassNode Parser ----------------------
/*
    static void appendChildBodyNode(FuncNodeStruct *fnNode, NodeBase *node) {
        if (fnNode->firstChildBodyNode == nullptr) {
            fnNode->firstChildBodyNode = node;
        }
        if (fnNode->lastChildBodyNode != nullptr) {
            fnNode->lastChildBodyNode->nextNode = node;
        }
        fnNode->lastChildBodyNode = node;
        fnNode->childCount++;
    }
*/

/*
    int internal_parameterListTokenizer(TokenizerParams_parent_ch_start_context) {
        auto *jsonArray = Cast::downcast<JsonArrayStruct *>(parent);

        if (ch == ']') {
            context->scanEnd = true;
            context->codeNode = Cast::upcast(&jsonArray->endBodyNode);
            return start + 1;
        }

        if (jsonArray->parsePhase == phase::EXPECT_VALUE) {
            return parseNextValue(TokenizerParams_pass, jsonArray);
        }

        auto *currentKeyValueItem = jsonArray->lastItem;

        if (jsonArray->parsePhase == phase::EXPECT_COMMA) {
            if (ch == ',') { // try to find ',' which leads to next key-value
                currentKeyValueItem->hasComma = true;
                context->codeNode = Cast::upcast(&currentKeyValueItem->follwingComma);
                jsonArray->parsePhase = phase::EXPECT_VALUE;
                return start + 1;
            } else if (context->afterLineBreak) {
                // comma is not needed after a line break
                return parseNextValue(TokenizerParams_pass, jsonArray);
            }
            return -1;
        }

        return -1;
    }
 */

    static int inner_fnBodyTokenizerMulti(TokenizerParams_parent_ch_start_context) {
        auto *fnNode = Cast::downcast<FuncNodeStruct *>(parent);

        //console_log(std::string(""+ch).c_str());
        //console_log((std::string{"==,"} + std::string{ch} + std::to_string(ch)).c_str());

        if (fnNode->parameterStartNode.found == -1) {
            if (ch == '(') {
                fnNode->parameterStartNode.found = start;
                context->setCodeNode(&fnNode->parameterStartNode);
                return start + 1;
            } else {
                context->setError(ErrorCode::expect_parenthesis_for_fn_params, context->prevFoundPos);
            }
        } else {
            if (fnNode->parameterEndNode.found == -1) {
                if (ch == ')') {
                    fnNode->parameterEndNode.found = start;
                    context->setCodeNode(&fnNode->parameterEndNode);
                    return start + 1;
                } else {
                    context->setError(ErrorCode::expect_end_parenthesis_for_fn_params, context->prevFoundPos);

                    /*
                        int result;
                        if (-1 < (result = Tokenizers::classTokenizer(parent, ch, start, context))) {
                            auto *innerClassNode = Cast::downcast<ClassNodeStruct *>(parent);
                            appendChildNode(innerClassNode, context->codeNode);
                            return result;
                        }
                    */

                }
            } else {
                int result;
                if (-1 < (result = Tokenizers::bodyTokenizer(Cast::upcast(&fnNode->bodyNode), ch, start, context))) {
                    context->scanEnd = true;
                    return result;
                }
            }
        }
        return -1;
    }


    int Tokenizers::fnTokenizer(TokenizerParams_parent_ch_start_context) {
        if (fn_first_char == ch) {
            // fn
            auto idx = ParseUtil::matchAt(context->chars, context->length, start, fn_chars);
            if (idx > -1) {
                int currentPos = idx + size_of_fn;
                int resultPos = -1;

                // "fn " came here
                auto *fnNode = Alloc::newFuncNode(context, parent);
                {
                    resultPos = Scanner::scanOnce(&fnNode->nameNode,
                                              Tokenizers::nameTokenizer,
                                              currentPos,
                                              context);

                    if (resultPos == -1) {
                        // the class should have a class name
                        //console_log(std::string(classNode->nameNode.name).c_str());
                        context->setError(ErrorCode::invalid_fn_name, start);

                        context->setCodeNode(fnNode);
                        return currentPos;
                    }
                }

                // Parse body
                currentPos = resultPos;
                if (-1 == (resultPos = Scanner::scanMulti(fnNode, inner_fnBodyTokenizerMulti,
                                                          currentPos, context))) {

                    context->setError(ErrorCode::invalid_fn_name, context->prevFoundPos);

                    context->setCodeNode(fnNode);
                    return currentPos;
                }

                context->setCodeNode(fnNode);
                return resultPos;
            }
        }
        return -1;
    }






















} // ok