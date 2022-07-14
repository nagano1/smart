﻿#include <cstdio>
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

    static int selfTextLength(LineBreakNodeStruct *self) {
        return self->text[1] == '\0' ? 1 : 2;
    }

    static const char *self_text(LineBreakNodeStruct *self) {
        return self->text;
    }


    static CodeLine *appendToLine(LineBreakNodeStruct *self, CodeLine *currentCodeLine) {
        auto *lineBreakNode = self; // Cast::downcast<LineBreakNodeStruct *>(self);

        auto* next = lineBreakNode;
        while (next) {
            currentCodeLine = currentCodeLine->addPrevLineBreakNode(next); // add space before break
            if (next->prevLineCommentNode) {
                currentCodeLine = VTableCall::appendToLine(next->prevLineCommentNode, currentCodeLine);
            }
            currentCodeLine->appendNode(Cast::upcast(next));

            
            auto *newNextLine = lineBreakNode->context->newCodeLine();
            newNextLine->init(lineBreakNode->context);

            currentCodeLine->nextLine = newNextLine;
            currentCodeLine = newNextLine;

            currentCodeLine->depth = self->context->parentDepth + 1;

            next = next->nextLineBreakNode;
        }

        return currentCodeLine;
    }

    static constexpr const char lineBreakTypeText[] = "<lineBreak>";

    static const node_vtable _lineBreakVTable = CREATE_VTABLE(LineBreakNodeStruct,
                                                              selfTextLength,
                                                              self_text,
                                                              appendToLine,
                                                              lineBreakTypeText,
                                                              NodeTypeId::LineBreak);

    const node_vtable *const VTables::LineBreakVTable = &_lineBreakVTable;

    LineBreakNodeStruct *Alloc::newLineBreakNode(ParseContext *context, NodeBase *parentNode) {
        auto *lineNode = context->newLineBreakNode();
        auto *node = Cast::upcast(lineNode);

        INIT_NODE(node, context, parentNode, VTables::LineBreakVTable);
        lineNode->nextLineBreakNode = nullptr;
        lineNode->prevLineCommentNode = nullptr;
        lineNode->text[0] = '\n';
        lineNode->text[1] = '\0';

        return lineNode;
    }
}