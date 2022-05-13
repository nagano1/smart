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

    // Line Break Node implementation

    static st_textlen selfTextLength(LineBreakNodeStruct *self) {
        return 1;
    }

    static const char *self_text(LineBreakNodeStruct *self) {
        return self->text;
    };


    static CodeLine *appendToLine(LineBreakNodeStruct *self, CodeLine *currentCodeLine) {
        auto *lineBreakNode = self;//Cast::downcast<LineBreakNodeStruct *>(self);

        auto* next = lineBreakNode;
        while (next) {
            currentCodeLine = currentCodeLine->addPrevLineBreakNode(next);
            currentCodeLine->appendNode(Cast::upcast(next));
            
            auto *newNextLine = lineBreakNode->context->newCodeLine();
            newNextLine->init(lineBreakNode->context);

            currentCodeLine->nextLine = newNextLine;
            currentCodeLine = newNextLine;

            currentCodeLine->depth = self->context->parentDepth + 1;

            next = next->nextLineBreakNode;
        }

        return currentCodeLine;
    };

    static constexpr const char lineBreakTypeText[] = "<lineBreak>";

    static const node_vtable _LineBreakVTable = CREATE_VTABLE(LineBreakNodeStruct,
        selfTextLength, self_text,
        appendToLine, lineBreakTypeText);

    const node_vtable *VTables::LineBreakVTable = &_LineBreakVTable;

    LineBreakNodeStruct *Alloc::newLineBreakNode(ParseContext *context, NodeBase *parentNode) {
        auto *lineNode = context->newLineBreakNode();
        auto *node = Cast::upcast(lineNode);

        INIT_NODE(node, context, parentNode, VTables::LineBreakVTable);
        lineNode->nextLineBreakNode = nullptr;
        lineNode->text[0] = '\n';
        lineNode->text[1] = '\0';

        return lineNode;
    }

}