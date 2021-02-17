﻿#include <stdio.h>
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
    // perform Indentation just to follow the indent rule
    /*
{
"jsonrpc":"2.0"
}
    |
{
    "jsonrpc":"2.0"
}

     */
    static void performIndentSelectionOperation(
            DocumentStruct *doc, NodeBase *startNode, NodeBase *endNode
    ) {

        assert(startNode != nullptr);
        assert(startNode->parentNode != nullptr);



        if (startNode->parentNode != nullptr) {
            auto *parentNode = startNode->parentNode;
            parentNode->line;
        }

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


    OperationResult *DocumentUtils::performCodingOperation(
            CodingOperations op,
            DocumentStruct *doc,
            NodeBase *startNode, NodeBase *endNode
    ) {
        if (startNode == nullptr) {
            return nullptr;
        }

        when(op) {
            wfor(CodingOperations::IndentSelection,
                 performIndentSelectionOperation(doc, startNode, endNode))
            wfor(CodingOperations::Deletion,
                 performIndentSelectionOperation(doc, startNode, endNode));
            wfor(CodingOperations::BreakLine,
                 performIndentSelectionOperation(doc, startNode, endNode));
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
