﻿#include <cstdio>
#include <iostream>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <vector>

#include <cstdint>
#include <ctime>
#include <cstring>

#include "code_nodes.hpp"


namespace smart {

    // --------------------- Defines ReturnStatement VTable ---------------------- /

    static int selfTextLength(ReturnStatementNodeStruct *) {
        // virtual node
        return 0;
    }

    static const utf8byte *selfText(ReturnStatementNodeStruct*) {
        // virtual node
        return "";
    }

    static CodeLine *appendToLine(ReturnStatementNodeStruct*self, CodeLine *currentCodeLine) {
        currentCodeLine = VTableCall::appendToLine(&self->returnText, currentCodeLine);

        auto *prevCodeLine = currentCodeLine;
        auto formerParentDepth = self->context->parentDepth;

        if (self->valueNode) {
            currentCodeLine = VTableCall::appendToLine(self->valueNode, currentCodeLine);

            if (prevCodeLine != currentCodeLine) {
                currentCodeLine->depth = formerParentDepth + 1;
            }
        }

        self->context->parentDepth = formerParentDepth;

        return currentCodeLine;
    }


    static constexpr const char assignTypeText[] = "<ReturnStatement>";

    /*
     * return "jfoaw"
     */
    static const node_vtable _returnVTable = CREATE_VTABLE(ReturnStatementNodeStruct,
                                                          selfTextLength,
                                                          selfText,
                                                          appendToLine,
                                                          assignTypeText
                                                          , NodeTypeId::ReturnStatement);

    const struct node_vtable *const VTables::ReturnStatementVTable = &_returnVTable;


    // -------------------- Implements AssignStatement Allocator --------------------- //
    ReturnStatementNodeStruct *Alloc::newReturnStatement(ParseContext *context, NodeBase *parentNode) {
        auto *returnStatement = context->newMem<ReturnStatementNodeStruct>();

        returnStatement->valueNode = nullptr;

        Init::initReturnStatement(context, parentNode, returnStatement);
        return returnStatement;
    }

    constexpr char returnText[] = "return";
    constexpr int returnTextSize = sizeof(returnText) - 1;

    void Init::initReturnStatement(ParseContext *context, NodeBase *parentNode, ReturnStatementNodeStruct *returnStatement) {
        INIT_NODE(returnStatement, context, parentNode, &_returnVTable);

        returnStatement->valueNode = nullptr;

        Init::initSimpleTextNode(&returnStatement->returnText, context, returnStatement, returnTextSize);
    }

    // --------------------- Implements ClassNode Parser ----------------------
    static int inner_returnStatementTokenizerMulti(TokenizerParams_parent_ch_start_context) {
        auto *returnNode = Cast::downcast<ReturnStatementNodeStruct *>(parent);
        int result;
        if (-1 < (result = Tokenizers::jsonValueTokenizer(Cast::upcast(returnNode), ch,
                                                          start, context))) {
            returnNode->valueNode = context->codeNode;
            context->scanEnd = true;

            return result;
        } else {
            //context->scanEnd = true;
            //context->setError(ErrorCode::syntax_error, start);
        }

        return -1;
    }

    // return 1234
    int Tokenizers::returnStatementTokenizer(TokenizerParams_parent_ch_start_context) {
        // return
        if ('r' == ch) {
            auto idx = ParseUtil::matchAt(context->chars, context->length, start, returnText);
            if (idx > -1) {
                auto *returnNode = Alloc::newReturnStatement(context, parent);
                Init::assignText_SimpleTextNode(&returnNode->returnText, context, start, returnTextSize);

                context->codeNode = Cast::upcast(&returnNode->returnText);
                context->virtualCodeNode = Cast::upcast(returnNode);


                int currentPos = idx + returnTextSize;
                // return only
                if (!ParseUtil::hasCharBeforeLineBreak(context->chars, context->length, currentPos)) {
                    return currentPos;
                }

                // return 3421
                int resultPos;
                if (-1 < (resultPos = Scanner::scanMulti(returnNode,
                                                         inner_returnStatementTokenizerMulti,
                                                         currentPos, context))) {
                    context->codeNode = Cast::upcast(&returnNode->returnText);
                    context->virtualCodeNode = Cast::upcast(returnNode);
                    return resultPos;
                }
            }
        }

        return -1;
    }
}



