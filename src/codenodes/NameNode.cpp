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

    static CodeLine *appendToLine(NameNodeStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    }

    static const char *self_text(NameNodeStruct *self) {
        return self->name;
    }

    static int selfTextLength(NameNodeStruct *self) {
        return self->nameLength;
    }

    int Tokenizers::nameTokenizer(TokenizerParams_parent_ch_start_context) {
        int found_count = 0;
        for (int_fast32_t i = start; i < context->length; i++) {
            if (ParseUtil::isIdentifierLetter(context->chars[i])) {
                found_count++;
            } else {
                break;
            }
        }

        if (found_count > 0) {
            auto *nameNode = Cast::downcast<NameNodeStruct *>(parent);

            context->codeNode = Cast::upcast(nameNode);
            nameNode->name = context->memBuffer.newMem<char>(found_count + 1);
            nameNode->nameLength = found_count;
            nameNode->found = start;

            memcpy(nameNode->name, context->chars + start, found_count);
            nameNode->name[found_count] = '\0';

            return start + found_count;
        }

        return -1;
    }

    static constexpr const char nameTypeText[] = "<Name>";

    static const node_vtable _nameVTable = CREATE_VTABLE(NameNodeStruct, selfTextLength,
                                                         self_text,
                                                         appendToLine, nameTypeText,
                                                         NodeTypeId::Name);
    const node_vtable *const VTables::NameVTable = &_nameVTable;



    static constexpr const char variableTypeText[] = "<Variable>";

    static const node_vtable _variableVTable = CREATE_VTABLE(VariableNodeStruct, selfTextLength,
                                                         self_text,
                                                         appendToLine, variableTypeText,
                                                         NodeTypeId::Variable);
    const node_vtable *const VTables::VariableVTable = &_variableVTable;


    void Init::initNameNode(NameNodeStruct *name, ParseContext *context, void *parentNode) {
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