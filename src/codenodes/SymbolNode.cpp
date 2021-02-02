﻿#include <stdio.h>
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

    static CodeLine *appendToLine(SymbolStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);
        return currentCodeLine;
    };

    static const utf8byte *self_text(SymbolStruct *self) {
        return self->symbol;
    };

    static int selfTextLength(SymbolStruct *self) {
        return 1;
    }

    static constexpr const char SymbolTypeText[] = "<Symbol>";

    static const node_vtable _nameVTable = CREATE_VTABLE(SymbolStruct ,selfTextLength,
                                                       self_text,
                                                       appendToLine, SymbolTypeText);
    const node_vtable *VTables::SymbolVTable = &_nameVTable;


    void Init::initSymbolNode(SymbolStruct *node, ParseContext *context, void *parentNode,
                              utf8byte letter) {
        INIT_NODE(node, context, parentNode, VTables::SymbolVTable);
        node->isEnabled = false;
        node->symbol[0] = letter;
        node->symbol[1] = '\0';
    }

}