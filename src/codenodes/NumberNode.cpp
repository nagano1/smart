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

    /*
        +--------------------------+
        |                          |
        |                          |
        |      BoolNode            |       
        |                          |
        |                          |
        +--------------------------+
    */

    static CodeLine *appendToLine2(BoolNodeStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);
        return currentCodeLine;
    }

    static const char *selfText2(BoolNodeStruct*self) {
        return self->text;
    }

    static int selfTextLength2(BoolNodeStruct*self) {
        return self->textLength;
    }

    static constexpr const char boolNodeTypeText[] = "<bool>";

    int Tokenizers::boolTokenizer(TokenizerParams_parent_ch_start_context) {

        static constexpr const char true_chars[] = "true";
        static constexpr const char false_chars[] = "false";

        bool hit = false;
        bool boolValue = false;
        int length = 0;

        if ('t' == ch) {
            auto idx = Tokenizer::matchFirstWithTrim(context->chars, true_chars, start);
            if (idx > -1) {
                hit = true;
                boolValue = true;
                length = sizeof(true_chars) - 1;
            }
        }
        else if ('f' == ch) {
            auto idx = Tokenizer::matchFirstWithTrim(context->chars, false_chars, start);
            if (idx > -1) {
                hit = true;
                length = sizeof(false_chars) - 1;
            }
        }


        if (hit) {
            if (start + length == context->length // allowed to be the last char of the file
                || Tokenizer::isNonIdentifierChar(context->chars[start + length])) { // otherwise, 

                //context->scanEnd = true;
                auto *boolNode = Alloc::newBoolNode(context, parent);

                boolNode->text = context->charBuffer.newChars(length + 1);
                boolNode->textLength = length;
                boolNode->boolValue = boolValue;

                TEXT_MEMCPY(boolNode->text, context->chars + start, length);
                boolNode->text[length] = '\0';

                context->codeNode = Cast::upcast(boolNode);
                return start + length;

            }
        }

        return -1;
    };


    static const node_vtable _Bool_VTable = CREATE_VTABLE(BoolNodeStruct, selfTextLength2,
        selfText2, appendToLine2, boolNodeTypeText);

    const node_vtable *VTables::BoolVTable = &_Bool_VTable;

    BoolNodeStruct* Alloc::newBoolNode(ParseContext *context, NodeBase *parentNode) {
        auto *node = (BoolNodeStruct *)malloc(sizeof(BoolNodeStruct));
        INIT_NODE(node, context, parentNode, VTables::BoolVTable);
        node->text = nullptr;
        node->textLength = 0;
        node->boolValue = false;

        return node;
    }


    //    +--------------------------+
    //    | Number                   |
    //    +--------------------------+

    static CodeLine *appendToLine(NumberNodeStruct *self, CodeLine *currentCodeLine) {
        assert(self->text != nullptr);

        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    }

    static const char *selfText(NumberNodeStruct *self) {
        return self->text;
    }

    static int selfTextLength(NumberNodeStruct *self) {
        return self->textLength;
    }


    static constexpr const char numberNodeTypeText[] = "<number>";
    int Tokenizers::numberTokenizer(TokenizerParams_parent_ch_start_context) {
        unsigned int found_count = 0;
        for (uint_fast32_t i = start; i < context->length; i++) {
            if (!Tokenizer::isNumberLetter(context->chars[i])) {
                break;
            }
            found_count++;
        }

        if (found_count > 0) {
            //context->scanEnd = true;
            auto *numberNode = Alloc::newNumberNode(context, parent);

            context->codeNode = Cast::upcast(numberNode);
            numberNode->text = context->charBuffer.newChars(found_count + 1);
            numberNode->textLength = found_count;

            TEXT_MEMCPY(numberNode->text, context->chars + start, found_count);
            numberNode->text[found_count] = '\0';

            return start + found_count;
        }

        return -1;
    };


    static const node_vtable _Number_VTable = CREATE_VTABLE(NumberNodeStruct, selfTextLength,
        selfText,
        appendToLine, numberNodeTypeText);

    const node_vtable *VTables::NumberVTable = &_Number_VTable;



    NumberNodeStruct *Alloc::newNumberNode(ParseContext *context, NodeBase *parentNode) {
        auto *node = (NumberNodeStruct *)malloc(sizeof(NumberNodeStruct));
        INIT_NODE(node, context, parentNode, VTables::NumberVTable);
        node->text = nullptr;
        node->textLength = 0;

        return node;
    }

}