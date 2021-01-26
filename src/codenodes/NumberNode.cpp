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

    static CodeLine *appendToLine(NumberNodeStruct *self, CodeLine *currentCodeLine) {
        if (self->text == nullptr) {
            return currentCodeLine;
        }
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    };

    static const char *self_text(NumberNodeStruct *self) {
        return self->text;
    };

    static int selfTextLength(NumberNodeStruct *self) {
        return self->textLength;
    }


    int Tokenizers::numberTokenizer(TokenizerParams_parent_ch_start_context) {
        unsigned int found_count = 0;
        for (uint_fast32_t i = start; i < context->length; i++) {
            if (Tokenizer::isNumberLetter(context->chars[i])) {
                found_count++;
            } else {
                break;
            }
        }

        if (found_count > 0) {
            context->scanEnd = true;
            auto *numberNode = Allocator::newNumberNode(context, parent);

            context->codeNode = Cast::upcast(numberNode);
            numberNode->text = context->charBuffer.newChars(found_count + 1);
            numberNode->textLength = found_count;

            TEXT_MEMCPY(numberNode->text, context->chars + start, found_count);
            numberNode->text[found_count] = '\0';
            //console_log(numberNode->text);
//            printf("number : %s\n", numberNode->text);
            return start + found_count;
        }

        return -1;
    };


    static const node_vtable _Number_VTable = CREATE_VTABLE(NumberNodeStruct,
            selfTextLength,
                                                          self_text,
                                                          appendToLine);
    const node_vtable *VTables::NumberVTable = &_Number_VTable;


    void Init::initNumberNode(NumberNodeStruct *name, ParseContext *context, NodeBase *parentNode) {
        INIT_NODE(name, context, parentNode, VTables::NumberVTable);
        //name->name = nullptr;
        //name->nameLength = 0;
    }

    NumberNodeStruct *Allocator::newNumberNode(ParseContext *context, NodeBase *parentNode) {
        auto *node = (NumberNodeStruct *) malloc(sizeof(NumberNodeStruct));
        INIT_NODE(node, context, parentNode, VTables::NumberVTable);
        node->parentNode = parentNode;
        node->text = nullptr;
        node->textLength = 0;
        return node;
    }

}