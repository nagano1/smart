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
        return Tokenizers::nameTokenizer_ignore(TokenizerParams_pass, start);
    }

    int Tokenizers::nameTokenizer_ignore(TokenizerParams_parent_ch_start_context, int ignorePos) {
        int found_count = ignorePos - start;
        for (int_fast32_t i = ignorePos; i < context->length; i++) {
            if (ParseUtil::isIdentifierLetter(context->chars[i])) {
                found_count++;
            }
            else {
                break;
            }
        }

        if (found_count > 0) {
            auto *nameNode = Cast::downcast<NameNodeStruct *>(parent);

            context->setCodeNode(nameNode);
            nameNode->name = context->memBuffer.newMem<char>(found_count + 1);
            nameNode->nameLength = found_count;
            nameNode->found = start;

            memcpy(nameNode->name, context->chars + start, found_count);
            nameNode->name[found_count] = '\0';

            return start + found_count;
        }

        return -1;
    }


    VariableNodeStruct *Alloc::newVariableNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *node = context->newMem<VariableNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::VariableVTable);

        node->stackOffset = 0;

        //Init::initNameNode(reinterpret_cast<NameNodeStruct *>(&node), context, parentNode);

        return node;
    }

    int Tokenizers::variableTokenizer(TokenizerParams_parent_ch_start_context)
    {
        auto *variableNode = Alloc::newVariableNode(context, parent);
        return Tokenizers::nameTokenizer(reinterpret_cast<NodeBase *>(variableNode), ch, start, context);
    }

    static int NameNodeStruct_applyFuncToDescendants(NameNodeStruct *node, ApplyFunc_params3)
    {
        if (targetVTable == nullptr || node->vtable == targetVTable) {
            func(Cast::upcast(node), ApplyFunc_pass);
        }

        return 0;
    }


    static constexpr const char nameTypeText[] = "<Name>";

    static node_vtable _nameVTable = CREATE_VTABLE(NameNodeStruct, selfTextLength,
                                                         self_text, appendToLine,
                                                   NameNodeStruct_applyFuncToDescendants,
                                                         nameTypeText, NodeTypeId::Name);
    const node_vtable *VTables::NameVTable = &_nameVTable;



    static constexpr const char variableTypeText[] = "<Variable>";

    static node_vtable _variableVTable = CREATE_VTABLE(VariableNodeStruct, selfTextLength,
                                                         self_text, appendToLine,
                                                       NameNodeStruct_applyFuncToDescendants,
                                                       variableTypeText,
                                                         NodeTypeId::Variable);
    const node_vtable *VTables::VariableVTable = &_variableVTable;


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