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

    static int BoolNodeStruct_applyFuncToDescendants(
            BoolNodeStruct *node, ApplyFunc_params)
    {
        if (targetVTable == nullptr || node->vtable == targetVTable) {
            func(Cast::upcast(node), ApplyFunc_pass);
        }

        return 0;
    }



    static constexpr const char boolNodeTypeText[] = "<bool>";
    static node_vtable _boolVTable = CREATE_VTABLE(BoolNodeStruct, selfTextLength2,
                                                         selfText2, appendToLine2,
                                                   BoolNodeStruct_applyFuncToDescendants,
                                                         boolNodeTypeText, NodeTypeId::Bool);

    const node_vtable *VTables::BoolVTable = &_boolVTable;

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

            numberNode->num = atoi(numberNode->text);

            return start + found_count;
        }

        return -1;
    }


    static int NumberNodeStruct_applyFuncToDescendants(
            NumberNodeStruct *node, ApplyFunc_params)
    {
        if (targetVTable == nullptr || node->vtable == targetVTable) {
            func(Cast::upcast(node), ApplyFunc_pass);
        }

        return 0;
    }


    static node_vtable _numberVTable_ = CREATE_VTABLE(NumberNodeStruct, selfTextLength,
                                                            selfText,
                                                            appendToLine,
                                                      NumberNodeStruct_applyFuncToDescendants,
                                                            numberNodeTypeText,
                                                            NodeTypeId::Number);

    const node_vtable *VTables::NumberVTable = &_numberVTable_;



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
        // (
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self)
                                         ->appendNode(self);

        auto *openCodeLine = currentCodeLine;
        int formerDepth = currentCodeLine->depth;


        if (self->valueNode) {
            int formerParentDepth = self->context->parentDepth;
            int formerArithmeticDepth = self->context->arithmeticBaseDepth;

            self->context->arithmeticBaseDepth = -1;

            int diff = currentCodeLine->depth == self->context->parentDepth ? 0 : 1;
            self->context->parentDepth += diff;
            currentCodeLine = VTableCall::callAppendToLine(self->valueNode, currentCodeLine);

            self->context->arithmeticBaseDepth = formerArithmeticDepth;
            self->context->parentDepth = formerParentDepth;
        }


        // )
        currentCodeLine = VTableCall::callAppendToLine(&self->closeNode, currentCodeLine);

        if (currentCodeLine != openCodeLine) {
            bool hasNonBracketEntity = false;
            auto *node = currentCodeLine->firstNode;
            while (node) { // NOLINT(altera-id-dependent-backward-branch,altera-unroll-loops)
                if (node->vtable == VTables::SymbolVTable) {
                    auto *symbol = Cast::downcast<SymbolStruct *>(node);
                    bool end = symbol->symbol[0] == ')' || symbol->symbol[0] == '}';
                    if (!end) {
                        hasNonBracketEntity = true;
                        break;
                    }
                } else if (node->vtable != VTables::LineBreakVTable
                           && node->vtable != VTables::LineCommentVTable
                           && node->vtable != VTables::BlockCommentFragmentVTable
                           && node->vtable != VTables::BlockCommentVTable
                           && node->vtable != VTables::EndOfFileVTable
                        ) {
                    hasNonBracketEntity = true;
                    break;
                }
                node = node->nextNode;
            }
            if (!hasNonBracketEntity) {
                currentCodeLine->depth = formerDepth;
            }
        }

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
            context->setCodeNode(&fnNode->closeNode);
            context->scanEnd = true;
            return start + 1;
        } else {
            if (fnNode->valueNode != nullptr && fnNode->valueNode->found > -1) {
                context->setError(ErrorCode::expect_end_parenthesis,
                                  context->prevFoundPos);
            }
            else {
                int result;
                if (-1 < (result = Tokenizers::expressionTokenizer(Cast::upcast(fnNode), ch, start,
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

    static int parentheses_applyFuncToDescendants(ParenthesesNodeStruct *node, ApplyFunc_params) {

        if (node->valueNode) {
            node->valueNode->vtable->applyFuncToDescendants(node->valueNode, ApplyFunc_pass2);
        }
        return 0;
    }

    static node_vtable _parenthesesVTable = CREATE_VTABLE(ParenthesesNodeStruct,
                                                                parentheses_selfTextLength,
                                                                parentheses_selfText,
                                                                parentheses_appendToLine,
                                                          parentheses_applyFuncToDescendants,
                                                                parenthesesNodeTypeText,
                                                            NodeTypeId::Parentheses);

    const node_vtable *VTables::ParenthesesVTable = &_parenthesesVTable;

    ParenthesesNodeStruct *Alloc::newParenthesesNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *node = context->newMem<ParenthesesNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::ParenthesesVTable);
        node->valueNode = nullptr;

        //Init::initSymbolNode(&node->openNode, context, node, '(');
        Init::initSymbolNode(&node->closeNode, context, node, ')');
        return node;
    }








    //    +--------------------------+
    //    |  Binary Operation        |
    //    +--------------------------+

    static CodeLine *binaryop_appendToLine(BinaryOperationNodeStruct *self, CodeLine *currentCodeLine)
    {

        int formerParentDepth = self->context->parentDepth;

        if (self->leftExprNode) {
            // leftExpr
            currentCodeLine = VTableCall::callAppendToLine(self->leftExprNode, currentCodeLine);
        }

        int formerArithmeticDepth = self->context->arithmeticBaseDepth;

        int diff = currentCodeLine->depth == self->context->parentDepth ? 0 : 1;

        int newDepth = self->context->arithmeticBaseDepth > -1 ?
                       self->context->arithmeticBaseDepth : formerParentDepth + diff;

        self->context->arithmeticBaseDepth = newDepth;
        self->context->parentDepth = newDepth;

        // operator +
        currentCodeLine = VTableCall::callAppendToLine(&self->opNode, currentCodeLine);


        if (self->rightExprNode) {
            // rightExpr
            currentCodeLine = VTableCall::callAppendToLine(self->rightExprNode, currentCodeLine);
        }

        self->context->parentDepth = formerParentDepth;
        self->context->arithmeticBaseDepth = formerArithmeticDepth;

        return currentCodeLine;
    }

    static const char *binaryop_selfText(BinaryOperationNodeStruct *self)
    {
        return "";
    }

    static int binaryop_selfTextLength(BinaryOperationNodeStruct *self)
    {
        return 0;
    }

    static int BinaryOperationNodeStruct_applyFuncToDescendants(
            BinaryOperationNodeStruct *node, ApplyFunc_params)
    {
        if (targetVTable == nullptr || node->vtable == targetVTable) {
            func(Cast::upcast(node), ApplyFunc_pass);
        }

        if (node->leftExprNode) {
            node->leftExprNode->vtable->applyFuncToDescendants(node->leftExprNode,
                                                           ApplyFunc_pass2);
        }

        if (node->rightExprNode) {
            node->rightExprNode->vtable->applyFuncToDescendants(node->rightExprNode,
                                                            ApplyFunc_pass2);
        }
        return 0;
    }


    static constexpr const char binaryop_NodeTypeText[] = "<binary op>";

    static node_vtable binaryop_VTable = CREATE_VTABLE(BinaryOperationNodeStruct ,
                                                             binaryop_selfTextLength,
                                                             binaryop_selfText,
                                                             binaryop_appendToLine,
                                                       BinaryOperationNodeStruct_applyFuncToDescendants,
                                                             binaryop_NodeTypeText,
                                                                NodeTypeId::BinaryOperation);

    const node_vtable *VTables::BinaryOperationVTable = &binaryop_VTable;



    static int inner_op_binaryOpTokenizer(TokenizerParams_parent_ch_start_context) {

        if (ch == '+' || ch == '*' || ch == '-' || ch == '/' || ch == '%'
            || ch == '&' || ch == '|') {

            auto *binaryOpNode = Alloc::newBinaryOperationNode(context, parent, ch);

            context->leftNode = Cast::upcast(&binaryOpNode->opNode);
            context->virtualCodeNode = Cast::upcast(binaryOpNode);
            return start + 1;
        }

        return -1;
    }


    int Tokenizers::binaryOperationTokenizer(TokenizerParams_parent_ch_start_context)
    {
        assert(context->virtualCodeNode != nullptr);

        auto *virtualNode = context->virtualCodeNode;
        auto *leftNode = context->leftNode;

        int resultPos = Scanner::scanOnce(parent, inner_op_binaryOpTokenizer, start, context);
        context->leftNode = leftNode;

        if (resultPos > -1) {
            auto* binaryOpNode = Cast::downcast<BinaryOperationNodeStruct*>(context->virtualCodeNode);
            binaryOpNode->leftExprNode = virtualNode;
            binaryOpNode->leftExprNode->parentNode = Cast::upcast(binaryOpNode);

            if (-1 < (resultPos = Scanner::scanOnce(binaryOpNode,
                                                    Tokenizers::expressionTokenizer,
                                                    resultPos, context))) {
                binaryOpNode->rightExprNode = context->virtualCodeNode;
                context->virtualCodeNode = Cast::upcast(binaryOpNode);
                context->leftNode = leftNode;
                return resultPos;
            }
        }
        return -1;
    }


    BinaryOperationNodeStruct *Alloc::newBinaryOperationNode(ParseContext *context, NodeBase *parentNode, char op)
    {
        auto *node = context->newMem<BinaryOperationNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::BinaryOperationVTable);

        node->leftExprNode = nullptr;
        node->rightExprNode = nullptr;

        Init::initSymbolNode(&node->opNode, context, node, op);

        return node;
    }








} // namespace
