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
    /**
     *  EndOfFile Struct
     */
    static CodeLine *appendToLine(EndOfFileNodeStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    };

    static const utf8byte *selfText(EndOfFileNodeStruct *self) {
        return "";
    };

    static int selfTextLength(EndOfFileNodeStruct *self) {
        return 0;
    }

    static const char endOfFileTypeText[] = "<EndOfFile>";

    static const node_vtable _endOfDocVTable = CREATE_VTABLE(EndOfFileNodeStruct,
                                                             selfTextLength,
                                                             selfText,
                                                             appendToLine, endOfFileTypeText, false);

    const node_vtable *VTables::EndOfFileVTable = &_endOfDocVTable;
}