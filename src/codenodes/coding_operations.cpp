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
    // perform Indentation just to follow the indent rule
    /*
{
"jsonrpc":"2.0"
}
    ---------------------------
{
    "jsonrpc":"2.0"
}

     */

    // find an ancestor
    static NodeBase *findIndentChangingPointParent(NodeBase *node) {
        node = node->parentNode;
        while (node != nullptr) {
            if (node->vtable->is_indent_change_point_parent) {
                return node;
            }
            node = node->parentNode;
        }
        return nullptr;
    }


    static NodeBase *findFirstElementNode(CodeLine *line) {
        auto * node = line->firstNode;
        while (node) {
            if (node->vtable == VTables::SpaceVTable ||
            node->vtable == VTables::LineBreakVTable) {

                node = node->nextNodeInLine;
                continue;
            }

            return node;
        }

        return nullptr;
    }


    static void indentFormatLine(CodeLine *line) {
        auto *firstElement = findFirstElementNode(line);
        if (firstElement) {
            assert(firstElement->parentNode != nullptr);

            NodeBase *pointParent = findIndentChangingPointParent(firstElement);
            if (pointParent) {
                assert(pointParent->line);
                auto *parentLine = pointParent->line;

                if (parentLine && parentLine != firstElement->line) {
                    auto parentIndent = parentLine->indent;
                    auto *context = firstElement->context;

                    // modify Indent
                    SpaceNodeStruct *space;
                    if (firstElement->prevSpaceNode) {
                        space = firstElement->prevSpaceNode;
                    } else {
                        space = Alloc::newSpaceNode(context, firstElement);
                        firstElement->prev_char = '\0';
                        firstElement->prevSpaceNode = space;
                        firstElement->line->insertNode(Cast::upcast(space), nullptr);
                    }

                    auto &baseIndent = context->baseIndent;
                    line->indent = parentIndent + baseIndent;
                    space->textLength = parentIndent + baseIndent;
                    space->text = context->memBuffer.newMem<char>(parentIndent + baseIndent + 1);
                    for (int i = 0; i < parentIndent + baseIndent; i++) {
                        space->text[i] = ' ';
                    }
                }
            }
        }
    }

    // IndentSelection operation will not add a line
    static void performIndentSelectionOperation(
            DocumentStruct *doc, NodeBase *startNode, NodeBase *endNode
    ) {
        assert(startNode != nullptr);
        auto *line = startNode->line;
        while (line) {
            indentFormatLine(line);

            if (endNode == nullptr) {
                break;
            }

            if (line == endNode->line) {
                break;
            }
            line = line->nextLine;
        }
    }

    OperationResult * DocumentUtils::performCodingOperation(CodingOperations op, DocumentStruct * doc, NodeBase * startNode, NodeBase * endNode)
    {
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

        auto *line = doc->firstCodeLine;
        while (line) {
            auto *firstElement = findFirstElementNode(line);
            if (firstElement) {

            }
            line = line->nextLine;
        }
    }

}
