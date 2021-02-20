#include <stdio.h>
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
    };

    static int selfTextLength(SimpleTextNodeStruct *self) {
        return self->textLength;
    }

    static CodeLine *appendToLine(SimpleTextNodeStruct *self, CodeLine *currentCodeLine) {
        return currentCodeLine->addPrevLineBreakNode(self)->appendNode(self);
    };

    static constexpr const char simpleTextTypeText[] = "<SimpleText>";

    static struct node_vtable _SIMPLE_TEXT_VTABLE = CREATE_VTABLE(SimpleTextNodeStruct,
                                                                  selfTextLength,
                                                                  self_text,
                                                                  appendToLine, simpleTextTypeText
                                                                  ,false);

    const struct node_vtable *VTables::SimpleTextVTable = &_SIMPLE_TEXT_VTABLE;
    const struct node_vtable *VTables::SpaceVTable = &_SIMPLE_TEXT_VTABLE;
    const struct node_vtable *VTables::NullVTable = &_SIMPLE_TEXT_VTABLE;


    SimpleTextNodeStruct *
    Alloc::newSimpleTextNode(ParseContext *context, NodeBase *parentNode) {
        auto *spaceNode = context->newSpaceNode();
        auto *node = Cast::upcast(spaceNode);

        INIT_NODE(node, context, parentNode, VTables::SimpleTextVTable);
        return spaceNode;
    }


    SpaceNodeStruct *Alloc::newSpaceNode(ParseContext *context, NodeBase *parentNode) {
        auto *spaceNode = context->newSpaceNode();
        auto *node = Cast::upcast(spaceNode);

        INIT_NODE(node, context, parentNode, VTables::SpaceVTable);
        return spaceNode;
    }

    NullNodeStruct *Alloc::newNullNode(ParseContext *context, NodeBase *parentNode) {
        auto *nullNode = context->newSpaceNode();
        auto *node = Cast::upcast(nullNode);

        INIT_NODE(node, context, parentNode, VTables::NullVTable);
        return nullNode;
    }
}