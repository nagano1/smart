#include <stdio.h>
#include <string>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <ctime>

#include "code_nodes.hpp"

namespace smart {

    
    /**
     *
     */
    static void performFormatSelectionOperation(
            DocumentStruct *doc, NodeBase *startNode, NodeBase *endNode
    ) {
        auto *line = doc->firstCodeLine;
        while (line) {
            auto *node = line->firstNode;
            if (node->vtable == VTables::SpaceVTable) {
                auto *space = Cast::downcast<SpaceNodeStruct *>(node);
                line->indent = space->textLength;
            }
            line = line->nextLine;
        }
    }


#define when(op) switch(op) 
#define wfor(val, handler) case val: {\
    (handler); break; \
    } \

    OperationResult *DocumentUtils::performCodingOperation(
            CodingOperations op,
            DocumentStruct *doc,
            NodeBase *startNode, NodeBase *endNode
    ) {

        if (startNode == nullptr) {
            return nullptr;
        }

        when(op) {
            wfor(CodingOperations::IndentSelection, performFormatSelectionOperation(doc, startNode, endNode));
            wfor(CodingOperations::deletion, performFormatSelectionOperation(doc, startNode, endNode));
        }

        return nullptr;
    }

     

    void DocumentUtils::formatIndent(DocumentStruct *doc) {

        DocumentUtils::performCodingOperation(
                CodingOperations::IndentSelection,
                doc,
                doc->firstRootNode,
                Cast::upcast(&doc->endOfFile)
        );

        /*

        auto *line = doc->firstCodeLine;
        while (line) {
            auto *node = line->firstNode;
            if (node->vtable == VTables::SpaceVTable) {
                auto *space = Cast::downcast<SpaceNodeStruct *>(node);
                line->indent = space->textLength;
            }
            line = line->nextLine;
        }
        */
    }
     
}
