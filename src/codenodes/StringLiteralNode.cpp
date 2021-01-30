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

    static CodeLine *appendToLine(StringLiteralNodeStruct *self, CodeLine *currentCodeLine) {
        auto *node = self;
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    }

    static const utf8byte *self_text(StringLiteralNodeStruct *self) {
        return self->text;
    }

    static int selfTextLength(StringLiteralNodeStruct *self) {
        return self->textLength;
    }


    int Tokenizers::stringLiteralTokenizer(TokenizerParams_parent_ch_start_context) {
        unsigned int found_count = 0;

        // starts with "
        bool startsWithDQuote = false;
        if (context->chars[start] == '"') {
            startsWithDQuote = true;
            found_count++;
        }

        int letterStart = startsWithDQuote ? start + 1 : start;
        for (uint_fast32_t i = letterStart; i < context->length; i++) {
            if (ParseUtil::isIdentifierLetter(context->chars[i])) {
                found_count++;
            }
            else if (startsWithDQuote) {
                found_count++;

                if (context->chars[i] == '"') {
                    break;
                }
            } else {
                break;
            }
        }

        if (found_count > 0) {
            auto *nameNode = simpleMalloc<StringLiteralNodeStruct>();
            Init::initStringLiteralNode(nameNode, context, parent);

            //auto *nameNode = Cast::downcast<NameNodeStruct *>(parent);

            context->codeNode = Cast::upcast(nameNode);
            nameNode->text = context->charBuffer.newChars(found_count + 1);
            nameNode->textLength = found_count;

            memcpy(nameNode->text, context->chars + start, found_count);
            nameNode->text[found_count] = '\0';

            return start + found_count;
        }

        return -1;

    };

    static constexpr const char nameTypeText[] = "<string>";

    static const node_vtable _VTable = CREATE_VTABLE(StringLiteralNodeStruct, selfTextLength,
                                                          self_text,
                                                          appendToLine, nameTypeText);
    const node_vtable *VTables::StringLiteralVTable = &_VTable;

    void Init::initStringLiteralNode(StringLiteralNodeStruct *name, ParseContext *context, NodeBase *parentNode) {
        INIT_NODE(name, context, parentNode, VTables::StringLiteralVTable);
        name->text = nullptr;
        name->textLength = 0;

        name->strValue = nullptr;
        name->strValueLength = 0;
    }

    /*
    Not used
    NameNodeStruct *Allocator::newNameNode(ParseContext *context, NodeBase *parentNode) {
        auto *node = (NameNodeStruct *) malloc(sizeof(NameNodeStruct));
        INIT_NODE(node, context, VTables::NameVTable);
        node->parentNode = parentNode;
        return node;
    }
    */

}