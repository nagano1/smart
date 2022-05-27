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

    static CodeLine *appendToLine(TypeNodeStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    }

    static const char *self_text(TypeNodeStruct *self) {
        return self->name;
    }

    static st_textlen selfTextLength(TypeNodeStruct *self) {
        return self->nameLength;
    }


    int Tokenizers::typeTokenizer(TokenizerParams_parent_ch_start_context) {
        return Tokenizers::nameTokenizer(TokenizerParams_pass);
    }

    static constexpr const char typeTypeText[] = "<Type>";

    static const node_vtable _typeVTable = CREATE_VTABLE(TypeNodeStruct, selfTextLength,
                                                         self_text,
                                                         appendToLine, typeTypeText, NodeTypeId::Type);
    const node_vtable *const VTables::TypeVTable = &_typeVTable;


    void Init::initTypeNode(TypeNodeStruct *name, ParseContext *context, void *parentNode) {
        INIT_NODE(name, context, parentNode, VTables::TypeVTable);
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