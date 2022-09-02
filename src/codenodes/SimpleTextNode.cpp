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

    static const char *self_text(SimpleTextNodeStruct *self) {
        return self->text;
    }

    static int selfTextLength(SimpleTextNodeStruct *self) {
        return self->textLength;
    }


    static CodeLine *appendToLine(SimpleTextNodeStruct *self, CodeLine *currentCodeLine) {
        return currentCodeLine->addPrevLineBreakNode(self)->appendNode(self);
    }

    static int SimpleTextNodeStruct_applyFuncToDescendants(
            SimpleTextNodeStruct *node, void *targetVTable,
            int (*func)(NodeBase *, void *, void *, void *, int )
            , void *arg, int argLen)
    {
        if (targetVTable == nullptr || node->vtable == targetVTable) {
            func(Cast::upcast(node), targetVTable, (void *)func, arg, argLen);
        }
//        if (node->leftExprNode) {
//            node->rightExprNode->vtable->setStackOffsetToVariables(node->rightExprNode,
//                                                                targetVTable, func, arg, argLen);
//        }
        return 0;
    }


    static constexpr const char simpleTextTypeText[] = "<SimpleText>";

    static struct node_vtable simpleTextVTABLE = CREATE_VTABLE(SimpleTextNodeStruct,
                                                                selfTextLength,
                                                                self_text,
                                                                appendToLine,
                                                               SimpleTextNodeStruct_applyFuncToDescendants,
                                                                simpleTextTypeText
                                                                  , NodeTypeId::SimpleText);
    const struct node_vtable *VTables::SimpleTextVTable = &simpleTextVTABLE;





    static constexpr const char spaceTextTypeText[] = "<SpaceText>";

    static node_vtable _spaceVTable = CREATE_VTABLE(SpaceNodeStruct,
                                                            selfTextLength,
                                                            self_text,
                                                            appendToLine,
                                                    SimpleTextNodeStruct_applyFuncToDescendants,
                                                            spaceTextTypeText, NodeTypeId::Space
    );
    const struct node_vtable *VTables::SpaceVTable = &_spaceVTable;


    static node_vtable _nullVTable = CREATE_VTABLE(NullNodeStruct,
                                                           selfTextLength,
                                                           self_text,
                                                           appendToLine,
                                                   SimpleTextNodeStruct_applyFuncToDescendants,
                                                           "<NULL>", NodeTypeId::NULLId
    );

    const struct node_vtable *VTables::NullVTable = &_nullVTable;


    static node_vtable _lineCommentVTable = CREATE_VTABLE(LineCommentNodeStruct,
                                                           selfTextLength,
                                                           self_text,
                                                           appendToLine,
                                                          SimpleTextNodeStruct_applyFuncToDescendants,
                                                           "<Line Comment>", NodeTypeId::LineComment
    );

    const struct node_vtable *VTables::LineCommentVTable = &_lineCommentVTable;


    static node_vtable _blockCommentFragmentVTable = CREATE_VTABLE(BlockCommentFragmentStruct,
                                                                 selfTextLength,
                                                                 self_text,
                                                                 appendToLine,
                                                                                SimpleTextNodeStruct_applyFuncToDescendants,
                                                                 "<Comment Fragment>",
                                                                NodeTypeId::BlockCommentFragment);

    const struct node_vtable *VTables::BlockCommentFragmentVTable = &_blockCommentFragmentVTable;




    static const char *self_text_blockcomment(BlockCommentNodeStruct *self) {
        return "";
    }

    static int selfTextLength_blockcomment(BlockCommentNodeStruct *self) {
        return 0; //self->textLength;
    }


    static CodeLine *appendToLineForBlockComment(BlockCommentNodeStruct *self, CodeLine *currentCodeLine)
    {
        currentCodeLine =  currentCodeLine->addPrevLineBreakNode(self)->appendNode(self);

        auto *commentFragment = self->firstCommentFragment;
        while (commentFragment) {
            currentCodeLine = VTableCall::appendToLine(commentFragment, currentCodeLine);

            commentFragment = Cast::downcast<BlockCommentFragmentStruct*>(commentFragment->nextNode);
        }
        return currentCodeLine;
    }


    static int BlockCommentNodeStruct_applyFuncToDescendants(
            BlockCommentNodeStruct *node, void *targetVTable,
            int (*func)(NodeBase *, void *, void *, void *, int )
            , void *arg, int argLen)
    {
        if (targetVTable == nullptr || node->vtable == targetVTable) {
            func(Cast::upcast(node), targetVTable, (void *)func, arg, argLen);
        }
//        if (node->leftExprNode) {
//            node->rightExprNode->vtable->setStackOffsetToVariables(node->rightExprNode,
//                                                                targetVTable, func, arg, argLen);
//        }
        return 0;
    }

    static node_vtable _blockCommentVTable = CREATE_VTABLE(BlockCommentNodeStruct,
                                                                  selfTextLength_blockcomment,
                                                                  self_text_blockcomment,
                                                                 appendToLineForBlockComment,
                                                           BlockCommentNodeStruct_applyFuncToDescendants, "<BlockComment>", NodeTypeId::BlockComment
    );

    const struct node_vtable *VTables::BlockCommentVTable = &_blockCommentVTable;











    SimpleTextNodeStruct *Alloc::newSimpleTextNode(ParseContext *context, NodeBase *parentNode) {
        auto *node = context->newMemForNode<SimpleTextNodeStruct>();
        Init::initSimpleTextNode(node, context, parentNode, 0);
        return node;
    }


    void Init::initSimpleTextNode(SimpleTextNodeStruct *textNode, ParseContext *context, void *parentNode, int charLen)
    {
        INIT_NODE(textNode, context, parentNode, VTables::SimpleTextVTable);

        textNode->text = context->memBuffer.newMem<char>(charLen + 1);
        textNode->textLength = charLen;

        //TEXT_MEMCPY(boolNode->text, context->chars + start, length);
        textNode->text[charLen] = '\0';
    }

    void Init::assignText_SimpleTextNode(SimpleTextNodeStruct *textNode, ParseContext *context, int pos, int charLen)
    {
        textNode->text = context->memBuffer.newMem<char>(charLen + 1);
        textNode->textLength = charLen;

        if (charLen > 0) {
            TEXT_MEMCPY(textNode->text, context->chars + pos, charLen);
        }
        textNode->text[charLen] = '\0';
    }

    LineCommentNodeStruct *Alloc::newLineCommentNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *lineComment = context->newMemForNode<LineCommentNodeStruct>();
        auto *node = Cast::upcast(lineComment);

        INIT_NODE(node, context, parentNode, VTables::LineCommentVTable);
        return lineComment;
    }


    BlockCommentFragmentStruct *Alloc::newBlockCommentFragmentNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *comment = context->newMemForNode<BlockCommentFragmentStruct>();
        auto *node = Cast::upcast(comment);

        INIT_NODE(node, context, parentNode, VTables::BlockCommentFragmentVTable);
        return comment;
    }

    BlockCommentNodeStruct *Alloc::newBlockCommentNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *node = context->newMem<BlockCommentNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::BlockCommentVTable);

        node->firstCommentFragment = nullptr;
        return node;
    }

    NullNodeStruct *Alloc::newNullNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *nullNode = context->newMemForNode<NullNodeStruct>();
        auto *node = Cast::upcast(nullNode);

        INIT_NODE(node, context, parentNode, VTables::NullVTable);
        return nullNode;
    }
}