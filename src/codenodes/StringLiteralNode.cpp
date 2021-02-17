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
        bool endsWithDQuote = false;
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
                    endsWithDQuote = true;
                    break;
                }
            } else {
                break;
            }
        }

        if (startsWithDQuote && !endsWithDQuote) {
            context->syntaxErrorInfo.hasError = true;
            context->syntaxErrorInfo.charPosition = start;
            context->syntaxErrorInfo.reason = (char*)"no end quote";
            context->syntaxErrorInfo.errorCode = 21390;

            return -1;
        }

        if (found_count > 0) {
            auto *strLiteralNode = simpleMalloc<StringLiteralNodeStruct>();
            Init::initStringLiteralNode(strLiteralNode, context, parent);

            context->codeNode = Cast::upcast(strLiteralNode);
            strLiteralNode->text = context->charBuffer.newChars(found_count + 1);
            strLiteralNode->textLength = found_count;

            memcpy(strLiteralNode->text, context->chars + start, found_count);
            strLiteralNode->text[found_count] = '\0';

            return start + found_count;
        }

        return -1;

    };

    static constexpr const char nameTypeText[] = "<string>";

    static const node_vtable _VTable = CREATE_VTABLE(StringLiteralNodeStruct, selfTextLength,
                                                          self_text,
                                                          appendToLine, nameTypeText, false);
    const node_vtable *VTables::StringLiteralVTable = &_VTable;

    void Init::initStringLiteralNode(StringLiteralNodeStruct *name, ParseContext *context, NodeBase *parentNode) {
        INIT_NODE(name, context, parentNode, VTables::StringLiteralVTable);
        name->text = nullptr;
        name->textLength = 0;

        name->strValue = nullptr;
        name->strValueLength = 0;
    }
}