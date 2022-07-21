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

    static constexpr const char simpleTextTypeText[] = "<SimpleText>";

    static const struct node_vtable simpleTextVTABLE = CREATE_VTABLE2(SimpleTextNodeStruct,
                                                                selfTextLength,
                                                                self_text,
                                                                appendToLine, simpleTextTypeText
                                                                  , NodeTypeId::SimpleText, 0);
    const struct node_vtable *const VTables::SimpleTextVTable = &simpleTextVTABLE;





    static constexpr const char spaceTextTypeText[] = "<SpaceText>";

    static const node_vtable _spaceVTable = CREATE_VTABLE2(SpaceNodeStruct,
                                                            selfTextLength,
                                                            self_text,
                                                            appendToLine, spaceTextTypeText, NodeTypeId::Space, 1
    );
    const struct node_vtable *const VTables::SpaceVTable = &_spaceVTable;


    static const node_vtable _nullVTable = CREATE_VTABLE2(NullNodeStruct,
                                                           selfTextLength,
                                                           self_text,
                                                           appendToLine, "<NULL>", NodeTypeId::NULLId, 2
    );

    const struct node_vtable *const VTables::NullVTable = &_nullVTable;


    static const node_vtable _lineCommentVTable = CREATE_VTABLE2(LineCommentNodeStruct,
                                                           selfTextLength,
                                                           self_text,
                                                           appendToLine, "<NULL>", NodeTypeId::NULLId, 3
    );

    const struct node_vtable *const VTables::LineCommentVTable = &_lineCommentVTable;

    static const node_vtable _blockCommentVTable = CREATE_VTABLE2(LineCommentNodeStruct,
                                                                 selfTextLength,
                                                                 self_text,
                                                                 appendToLine, "<NULL>", NodeTypeId::NULLId, 4
    );

    const struct node_vtable *const VTables::BlockCommentVTable = &_blockCommentVTable;


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

        TEXT_MEMCPY(textNode->text, context->chars + pos, charLen);
        textNode->text[charLen] = '\0';
    }

    LineCommentNodeStruct *Alloc::newLineCommentNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *lineComment = context->newMemForNode<LineCommentNodeStruct>();
        auto *node = Cast::upcast(lineComment);

        INIT_NODE(node, context, parentNode, VTables::LineCommentVTable);
        return lineComment;
    }

    LineCommentNodeStruct *Alloc::newBlockCommentNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *lineComment = context->newMemForNode<LineCommentNodeStruct>();
        auto *node = Cast::upcast(lineComment);

        INIT_NODE(node, context, parentNode, VTables::BlockCommentVTable);
        return lineComment;
    }

    NullNodeStruct *Alloc::newNullNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *nullNode = context->newMemForNode<NullNodeStruct>();
        auto *node = Cast::upcast(nullNode);

        INIT_NODE(node, context, parentNode, VTables::NullVTable);
        return nullNode;
    }
}