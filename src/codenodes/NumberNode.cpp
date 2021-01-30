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


    int Tokenizers::nullTokenizer(TokenizerParams_parent_ch_start_context) {
        static constexpr const char null_chars[] = "null";
        return Tokenizers::WordTokenizer(TokenizerParams_pass, 'n', null_chars);
    }




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
        return currentCodeLine->addPrevLineBreakNode(self)->appendNode(self);
    }

    static const char *selfText2(BoolNodeStruct*self) {
        return self->text;
    }

    static int selfTextLength2(BoolNodeStruct*self) {
        return self->textLength;
    }


    int Tokenizers::boolTokenizer(TokenizerParams_parent_ch_start_context) {
        int truePos = Tokenizers::WordTokenizer(TokenizerParams_pass, 't', "true");
        int result = truePos < 0 ? Tokenizers::WordTokenizer(TokenizerParams_pass, 'f', "false") : truePos;

        if (result > -1) {
            auto *boolNode = Cast::downcast<BoolNodeStruct*>(context->codeNode);
            boolNode->boolValue = truePos > -1;
            return result;
        }

        return -1;
    };


    static constexpr const char boolNodeTypeText[] = "<bool>";
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
            if (!ParseUtil::isNumberLetter(context->chars[i])) {
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