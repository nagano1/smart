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

    static CodeLine *appendToLine(NameNodeStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    };

    static const char *self_text(NameNodeStruct *self) {
        auto *node = self;//Cast::downcast<NameNodeStruct *>((NodeBase *) self);
        return node->name;
    };

    static st_textlen selfTextLength(NameNodeStruct *self) {
        auto *node = self;//Cast::downcast<NameNodeStruct *>((NodeBase *) self);

        return node->nameLength;
    }


    int Tokenizers::nameTokenizer(TokenizerParams_parent_ch_start_context) {
        unsigned int found_count = 0;
        for (uint_fast32_t i = start; i < context->length; i++) {
            if (ParseUtil::isIdentifierLetter(context->chars[i])) {
                found_count++;
            } else {
                break;
            }
        }

        if (found_count > 0) {
            //context->scanEnd = true;

            //auto *nameNode = Cast::downcast<NameNodeStruct *>(context->codeNode);
            auto *nameNode = Cast::downcast<NameNodeStruct *>(parent);

            //auto *nameNode = Allocator::newNameNode(context, parent);
            context->codeNode = Cast::upcast(nameNode);
            nameNode->name = context->memBuffer.newMem<char>(found_count + 1);
            nameNode->nameLength = found_count;

            memcpy(nameNode->name, context->chars + start, found_count);
            //nameNode->name[found_count] = '\0';
            return start + found_count;
        }
        return -1;
    };

    static constexpr const char nameTypeText[] = "<Name>";

    static const node_vtable _Name_VTable = CREATE_VTABLE(NameNodeStruct, selfTextLength,
                                                          self_text,
                                                          appendToLine, nameTypeText, false);
    const node_vtable *VTables::NameVTable = &_Name_VTable;


    void Init::initNameNode(NameNodeStruct *name, ParseContext *context, NodeBase *parentNode) {
        INIT_NODE(name, context, parentNode, VTables::NameVTable);
        name->name = nullptr;
        name->nameLength = 0;
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