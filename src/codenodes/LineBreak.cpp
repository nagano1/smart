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

    static int selfTextLength(LineBreakNodeStruct *self) {
        return 1;
        //return static_cast<int>(strlen(self->text));
    }

    static const char *self_text(LineBreakNodeStruct *self) {
        return self->text;
    };

    static constexpr const char lineBreakTypeText[] = "<lineBreak>";
    static int typeTextLength(LineBreakNodeStruct *self) {
        return sizeof(lineBreakTypeText) - 1;
        //return static_cast<int>(strlen(self->text));
    }

    static const char *typeText(LineBreakNodeStruct *self) {
        return lineBreakTypeText;
    };


    static CodeLine *appendToLine(LineBreakNodeStruct *self, CodeLine *currentCodeLine) {
        auto *lineBreakNode = self;//Cast::downcast<LineBreakNodeStruct *>(self);

        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        auto *newNextLine = lineBreakNode->context->mallocCodeLine();//simpleMalloc<CodeLine>();
        newNextLine->init(lineBreakNode->context);

        currentCodeLine->nextLine = newNextLine;
        currentCodeLine = newNextLine;


        auto *next = lineBreakNode->nextLineBreakNode;
        while (next) {
            currentCodeLine = currentCodeLine->addPrevLineBreakNode((next));
            currentCodeLine->appendNode(Cast::upcast(next));

            auto *newNextLine = lineBreakNode->context->mallocCodeLine();
            newNextLine->init(lineBreakNode->context);

            currentCodeLine->nextLine = newNextLine;
            currentCodeLine = newNextLine;

            next = next->nextLineBreakNode;
        }

        return currentCodeLine;
    };


    static const node_vtable _LineBreakVTable = CREATE_VTABLE(LineBreakNodeStruct,
        selfTextLength, self_text,
        appendToLine, typeTextLength, typeText);
    const node_vtable *VTables::LineBreakVTable = &_LineBreakVTable;

    LineBreakNodeStruct *Alloc::newLineBreakNode(ParseContext *context, NodeBase *parentNode) {
        auto *lineNode = context->mallocLineBreakNode();
        auto *node = Cast::upcast(lineNode);

        INIT_NODE(node, context, parentNode, VTables::LineBreakVTable);
        lineNode->nextLineBreakNode = nullptr;
        lineNode->text[0] = '\n';
        lineNode->text[1] = '\0';

        return lineNode;
    }

}