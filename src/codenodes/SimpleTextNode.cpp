﻿#include <stdio.h>
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
        //addPrevLineBreakNode(node, currentCodeLine);
        currentCodeLine->appendNode(self);
        return currentCodeLine;
    };

    static constexpr const char simpleTextTypeText[] = "<SimpleText>";

    static struct node_vtable _SIMPLE_TEXT_VTABLE = CREATE_VTABLE(SimpleTextNodeStruct,
                                                                  selfTextLength,
                                                                  self_text,
                                                                  appendToLine, simpleTextTypeText);
    const struct node_vtable *VTables::SimpleTextVTable = &_SIMPLE_TEXT_VTABLE;
    const struct node_vtable *VTables::SpaceVTable = &_SIMPLE_TEXT_VTABLE;


    SimpleTextNodeStruct *
    Alloc::newSimpleTextNode(ParseContext *context, NodeBase *parentNode) {
        auto *spaceNode = context->mallocSpaceNode();
        auto *node = Cast::upcast(spaceNode);

        INIT_NODE(node, context, parentNode, VTables::SimpleTextVTable);
        return spaceNode;
    }


    SpaceNodeStruct *Alloc::newSpaceNode(ParseContext *context, NodeBase *parentNode) {
        auto *spaceNode = context->mallocSpaceNode();
        auto *node = Cast::upcast(spaceNode);

        INIT_NODE(node, context, parentNode, VTables::SpaceVTable);
        return spaceNode;
    }
}