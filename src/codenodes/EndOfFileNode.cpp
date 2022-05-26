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

    static st_textlen selfTextLength(EndOfFileNodeStruct *) {
        return 0;
    }

    static const char endOfFileTypeText[] = "<EndOfFile>";

    static const node_vtable _endOfDocVTable = CREATE_VTABLE(EndOfFileNodeStruct,
                                                             selfTextLength,
                                                             selfText,
                                                             appendToLine, endOfFileTypeText, NodeTypeId::EndOfDoc);

    const node_vtable *const VTables::EndOfFileVTable = &_endOfDocVTable;
}