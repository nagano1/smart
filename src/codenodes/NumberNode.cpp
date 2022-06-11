#include <cstdio>
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
        |      nullTokenizer       |
        |                          |
        |                          |
        +--------------------------+
    */

    int Tokenizers::nullTokenizer(TokenizerParams_parent_ch_start_context) {
        static constexpr const char null_chars[] = "null";
        return Tokenizers::WordTokenizer(TokenizerParams_pass, 'n', null_chars);
    }




    /*
        +----------------------------------------------------+
        |                          
        |      BoolNode            
        |                          
        +----------------------------------------------------+
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


    int Tokenizers::boolTokenizer(TokenizerParams_parent_ch_start_context)
    {
        int result = Tokenizers::WordTokenizer(TokenizerParams_pass, 't', "true");
        bool isTrue = result > -1;
        if (!isTrue) {
            result = Tokenizers::WordTokenizer(TokenizerParams_pass, 'f', "false");
        }

        if (result > -1) {
            auto *boolNode = Cast::downcast<BoolNodeStruct*>(context->codeNode);
            boolNode->boolValue = isTrue;
            return result;
        }

        return -1;
    }


    static constexpr const char boolNodeTypeText[] = "<bool>";
    static const node_vtable _boolVTable = CREATE_VTABLE(BoolNodeStruct, selfTextLength2,
                                                         selfText2, appendToLine2, boolNodeTypeText, NodeTypeId::Bool);

    const node_vtable *const VTables::BoolVTable = &_boolVTable;

    BoolNodeStruct* Alloc::newBoolNode(ParseContext *context, NodeBase *parentNode) {
        auto *node = context->newMem<BoolNodeStruct>();
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
        int found_count = 0;
        for (int_fast32_t i = start; i < context->length; i++) {
            if (!ParseUtil::isNumberLetter(context->chars[i])) {
                break;
            }

            found_count++;
        }

        if (found_count > 0) {
            //context->scanEnd = true;
            auto *numberNode = Alloc::newNumberNode(context, parent);

            context->codeNode = Cast::upcast(numberNode);
            numberNode->text = context->memBuffer.newMem<char>(found_count + 1);
            numberNode->textLength = found_count;

            TEXT_MEMCPY(numberNode->text, context->chars + start, found_count);
            numberNode->text[found_count] = '\0';

            return start + found_count;
        }

        return -1;
    }


    static const node_vtable _numberVTable_ = CREATE_VTABLE(NumberNodeStruct, selfTextLength,
                                                            selfText,
                                                            appendToLine, numberNodeTypeText,
                                                            NodeTypeId::Number);

    const node_vtable *const VTables::NumberVTable = &_numberVTable_;



    NumberNodeStruct *Alloc::newNumberNode(ParseContext *context, NodeBase *parentNode) {
        auto *node = context->newMem<NumberNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::NumberVTable);
        node->text = nullptr;
        node->textLength = 0;

        return node;
    }

}