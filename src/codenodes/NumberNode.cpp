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

    /*
        +--------------------------+
        |                          |
        |                          |
        |      nullTokenizer       |
        |                          |
        |                          |
        +--------------------------+
    */

    int Tokenizers::nullTokenizer(TokenizerParams_parent_ch_start_context) {
        static constexpr const char null_chars[] = "null";
        return Tokenizers::WordTokenizer2(TokenizerParams_pass, Alloc::newNullNode, 'n', null_chars);
    }




    /*
        +----------------------------------------------------+
        |                          
        |      BoolNode            
        |                          
        +----------------------------------------------------+
    */

    static CodeLine *appendToLine2(BoolNodeStruct *self, CodeLine *currentCodeLine) {
        return currentCodeLine->addPrevLineBreakNode(self)->appendNode(self);
    }

    static const char *selfText2(BoolNodeStruct*self) {
        return self->text;
    }

    static int selfTextLength2(BoolNodeStruct*self) {
        return self->textLength;
    }


    int Tokenizers::boolTokenizer(TokenizerParams_parent_ch_start_context)
    {
        int result = Tokenizers::WordTokenizer2(TokenizerParams_pass,
                                                Alloc::newBoolNode
                                                ,'t', "true");
        bool isTrue = result > -1;
        if (!isTrue) {
            result = Tokenizers::WordTokenizer2(TokenizerParams_pass,
                                                Alloc::newBoolNode,
                                                'f', "false");
        }

        if (result > -1) {
            auto *boolNode = Cast::downcast<BoolNodeStruct*>(context->virtualCodeNode);
            boolNode->found = start;
            boolNode->boolValue = isTrue;
            return result;
        }

        return -1;
    }


    static constexpr const char boolNodeTypeText[] = "<bool>";
    static const node_vtable _boolVTable = CREATE_VTABLE(BoolNodeStruct, selfTextLength2,
                                                         selfText2, appendToLine2, boolNodeTypeText, NodeTypeId::Bool);

    const node_vtable *const VTables::BoolVTable = &_boolVTable;

    BoolNodeStruct* Alloc::newBoolNode(ParseContext *context, NodeBase *parentNode) {
        auto *node = context->newMem<BoolNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::BoolVTable);
        node->text = nullptr;
        node->textLength = 0;
        node->boolValue = false;
        return node;
    }













    //    +--------------------------+
    //    | Number                   |
    //    +--------------------------+

    static CodeLine *appendToLine(NumberNodeStruct *self, CodeLine *currentCodeLine)
    {
        assert(self->text != nullptr);

        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    }

    static const char *selfText(NumberNodeStruct *self)
    {
        return self->text;
    }

    static int selfTextLength(NumberNodeStruct *self)
    {
        return self->textLength;
    }


    static constexpr const char numberNodeTypeText[] = "<number>";
    int Tokenizers::numberTokenizer(TokenizerParams_parent_ch_start_context)
    {
        int found_count = 0;
        for (int_fast32_t i = start; i < context->length; i++) {
            if (!ParseUtil::isNumberLetter(context->chars[i])) {
                break;
            }

            found_count++;
        }

        if (found_count > 0) {
            auto *numberNode = Alloc::newNumberNode(context, parent);

            context->setCodeNode(numberNode);
            numberNode->text = context->memBuffer.newMem<char>(found_count + 1);
            numberNode->textLength = found_count;

            TEXT_MEMCPY(numberNode->text, context->chars + start, found_count);
            numberNode->text[found_count] = '\0';

            return start + found_count;
        }

        return -1;
    }


    static const node_vtable _numberVTable_ = CREATE_VTABLE(NumberNodeStruct, selfTextLength,
                                                            selfText,
                                                            appendToLine, numberNodeTypeText,
                                                            NodeTypeId::Number);

    const node_vtable *const VTables::NumberVTable = &_numberVTable_;



    NumberNodeStruct *Alloc::newNumberNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *node = context->newMem<NumberNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::NumberVTable);
        node->text = nullptr;
        node->textLength = 0;

        return node;
    }








    //    +--------------------------+
    //    | Parentheses value        |
    //    +--------------------------+
    static CodeLine *parentheses_appendToLine(ParenthesesNodeStruct *self, CodeLine *currentCodeLine)
    {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self)
                                         ->appendNode(self);

        if (self->valueNode) {
            int formerParentDepth = self->context->parentDepth;
            self->context->parentDepth += 1;
            currentCodeLine = VTableCall::appendToLine(self->valueNode, currentCodeLine);
            self->context->parentDepth = formerParentDepth;
        }

        currentCodeLine = VTableCall::appendToLine(&self->closeNode2, currentCodeLine);

        return currentCodeLine;
    }

    static const char *parentheses_selfText(ParenthesesNodeStruct *self)
    {
        return "(";
    }

    static int parentheses_selfTextLength(ParenthesesNodeStruct *self)
    {
        return 1;
    }


    static constexpr const char parenthesesNodeTypeText[] = "<parentheses>";

    static int inner_returnStatementTokenizerMulti(TokenizerParams_parent_ch_start_context) {
        auto *fnNode = Cast::downcast<ParenthesesNodeStruct *>(parent);

        if (ch == ')') {
            context->setCodeNode(&fnNode->closeNode2);
            context->scanEnd = true;
            return start + 1;
        } else {
            if (fnNode->valueNode != nullptr && fnNode->valueNode->found > -1) {
                context->setError(ErrorCode::expect_end_parenthesis,
                                  context->prevFoundPos);
            }
            else {
                int result;
                if (-1 < (result = Tokenizers::valueTokenizer(Cast::upcast(fnNode), ch, start,
                                                              context))) {
                    fnNode->valueNode = context->virtualCodeNode;
                    fnNode->valueNode->found = start;

                    return result;
                } else {
                    context->setError(ErrorCode::expect_end_parenthesis_for_fn_params,
                                      context->prevFoundPos);
                }
            }
        }
        return -1;
    }


    int Tokenizers::parenthesesTokenizer(TokenizerParams_parent_ch_start_context)
    {
        if ('(' == ch) {
            auto *returnNode = Alloc::newParenthesesNode(context, parent);
            int currentPos = start + 1;
            int resultPos;
            if (-1 < (resultPos = Scanner::scanMulti(returnNode,
                                                     inner_returnStatementTokenizerMulti,
                                                     currentPos, context))) {

                context->setCodeNode(returnNode);
                return resultPos;
            }
        }

        return -1;
    }


    static const node_vtable _parenthesesVTable = CREATE_VTABLE(ParenthesesNodeStruct,
                                                                parentheses_selfTextLength,
                                                                parentheses_selfText,
                                                                parentheses_appendToLine,
                                                                parenthesesNodeTypeText,
                                                            NodeTypeId::Parentheses);

    const node_vtable *const VTables::ParenthesesVTable = &_parenthesesVTable;

    ParenthesesNodeStruct *Alloc::newParenthesesNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *node = context->newMem<ParenthesesNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::ParenthesesVTable);
        node->valueNode = nullptr;

        //Init::initSymbolNode(&node->openNode, context, node, '(');
        Init::initSymbolNode(&node->closeNode2, context, node, ')');
        return node;
    }


} // namespace
