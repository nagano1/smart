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
        auto *typeNode  = Cast::downcast<TypeNodeStruct*>(parent);// Alloc::newTypeNode(context, parent);

        int currentPos = start;

        static constexpr const char let_chars[] = "let";
        static constexpr int size_of_let = sizeof(let_chars) - 1;

        bool hasMutMark = false;
        bool hasNullableMark = false;

        if ('$' == ch) { // $ is mutable mark
            hasMutMark = true;
            currentPos += 1;
        }
        else if ('?' == ch) { // ? is nullable mark
            hasNullableMark = true;
            currentPos += 1;
        }

        typeNode->hasNullableMark = hasNullableMark;
        typeNode->hasMutMark = hasMutMark;
        int result = Tokenizers::nameTokenizer_ignore(Cast::upcast(&typeNode->nameNode),
                                     context->chars[currentPos], start, context, currentPos);

        if (result > -1) {
            typeNode->useLet = ParseUtil::equal(
                                   typeNode->nameNode.name,
                                   typeNode->nameNode.nameLength,let_chars, size_of_let
                               );

            context->setCodeNode(typeNode);
            return result;
        }

        return -1;
    }

    static constexpr const char typeTypeText[] = "<Type>";

    static const node_vtable _typeVTable = CREATE_VTABLE(TypeNodeStruct, selfTextLength,
                                                         self_text,
                                                         appendToLine, typeTypeText, NodeTypeId::Type);
    const node_vtable *const VTables::TypeVTable = &_typeVTable;

    TypeNodeStruct *Alloc::newTypeNode(ParseContext *context, NodeBase *parentNode) {
        auto *node = context->newMem<TypeNodeStruct>();
        Init::initTypeNode(node, context, parentNode);
        return node;
    }

    void Init::initTypeNode(TypeNodeStruct *node, ParseContext *context, void *parentNode) {
        INIT_NODE(node, context, parentNode, VTables::TypeVTable);

        node->typeNode = nullptr;

        node->hasMutMark = false;
        node->hasNullableMark = false;
        node->stackSize = 0;
        node->useLet = false;

        Init::initNameNode(&node->nameNode, context, node);
    }

}