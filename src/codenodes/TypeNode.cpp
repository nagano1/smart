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

namespace smart
{

    static CodeLine *appendToLine(TypeNodeStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    }

    static const char *self_text(TypeNodeStruct *self) {
        return VTableCall::selfText(&self->nameNode);
    }

    static int selfTextLength(TypeNodeStruct *self) {
        return VTableCall::selfTextLength(Cast::upcast(&self->nameNode));
    }


    int Tokenizers::typeTokenizer(TokenizerParams_parent_ch_start_context) {
        auto *typeNode  = Alloc::newTypeNode(context, parent);

        auto returnPos = Tokenizers::nameTokenizer(Cast::upcast(&typeNode->nameNode), ch, start, context);
        if (returnPos > -1) {
            //if (context->codeNode) {
                context->codeNode = Cast::upcast(typeNode);
            //}
        }
        return returnPos;
    }

    static constexpr const char typeTypeText[] = "<Type>";

    static const node_vtable _typeVTable = CREATE_VTABLE(TypeNodeStruct, selfTextLength,
                                                         self_text,
                                                         appendToLine, typeTypeText, NodeTypeId::Type);
    const node_vtable *const VTables::TypeVTable = &_typeVTable;

    TypeNodeStruct *Alloc::newTypeNode(ParseContext *context, NodeBase *parentNode) {
        auto *node = context->newMem<TypeNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::TypeVTable);

        node->typeNode = nullptr;

        Init::initNameNode(&node->nameNode, context, node);

        return node;
    }
}