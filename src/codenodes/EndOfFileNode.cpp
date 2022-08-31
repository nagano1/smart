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
    /**
     *  EndOfFile Struct
     */
    static CodeLine *appendToLine(EndOfFileNodeStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    }

    static const utf8byte *selfText(EndOfFileNodeStruct *) {
        return "";
    }

    static int selfTextLength(EndOfFileNodeStruct *) {
        return 0;
    }

    static const char endOfFileTypeText[] = "<EndOfFile>";


    static int applyFuncToDescendants(EndOfFileNodeStruct *Node, void *targetVTable, int (*applyFuncToDescendants)(NodeBase *Node, void *targetVTable, void *func, void *arg, int argLen), void *arg, int argLen) {


        return 0;
    }



    static node_vtable _endOfDocVTable = CREATE_VTABLE(EndOfFileNodeStruct,
                                                             selfTextLength,
                                                             selfText,
                                                             appendToLine,
                                                             applyFuncToDescendants,
                                                             endOfFileTypeText,
                                                             NodeTypeId::EndOfDoc);

    const node_vtable *VTables::EndOfFileVTable = &_endOfDocVTable;
}