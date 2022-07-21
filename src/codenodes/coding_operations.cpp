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
//	static NodeBase* findIndentChangingPointParent(NodeBase* node) {
//		node = node->parentNode;
//		while (node != nullptr) {
//			if (node->vtable->is_indent_change_point_parent) {
//				return node;
//			}
//			node = node->parentNode;
//		}
//		return nullptr;
//	}
//

	static NodeBase* findFirstElementNode(CodeLine* line) {
		auto* node = line->firstNode;
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


	/*
	* Before editing this file, it is required to modify indentation to follow ZM's indent rule
	*/
	void DocumentUtils::assignIndents(DocumentStruct* doc)
	{
		doc->context->has_depth_error = false;

		// assign indent
		auto* line = doc->firstCodeLine;
		while (line) {
			auto* node = line->firstNode;
			line->indent = node->prev_chars;

			line = line->nextLine;
		}
	}


	/*
	* if justKeepRule is specified,  only rightward indent fix is gonna be performed.
	*/
	static void indentFormatLine(CodeLine* line, bool justKeepRule) {
		auto* firstElement = findFirstElementNode(line);
		if (firstElement) {
			assert(firstElement->parentNode != nullptr);
			//assert(pointParent->line);
			auto* context = firstElement->context;
			auto& baseIndent = context->baseIndent;

			// modify indent
			if (justKeepRule) {
				if (firstElement->prev_chars >= line->depth * baseIndent) {
					return;
				}
			}
			int textLen = static_cast<int>(line->depth * baseIndent);
			firstElement->prev_chars = textLen;
		}
	}

	// IndentSelection operation will not add a line
	static void performIndentSelectionOperation(
		DocumentStruct* doc, NodeBase* startNode, NodeBase* endNode, bool keepRuleMode
	) {
		assert(startNode != nullptr);
		auto* line = startNode->line;
		while (line) {
			indentFormatLine(line, keepRuleMode);

			if (endNode == nullptr) {
				break;
			}

			if (line == endNode->line) {
				break;
			}
			line = line->nextLine;
		}

	}

	OperationResult* DocumentUtils::performCodingOperation(
		CodingOperations op,
		DocumentStruct* doc,
		NodeBase* startNode,
		NodeBase* endNode
	) {
		if (startNode == nullptr) {
			return nullptr;
		}

		when(op) {
			wfor(CodingOperations::AutoIndentForSpacingRule,
				performIndentSelectionOperation(doc, startNode, endNode, true))

			wfor(CodingOperations::AutoIndentSelection,
				performIndentSelectionOperation(doc, startNode, endNode, false))

			wfor(CodingOperations::Deletion,
				performIndentSelectionOperation(doc, startNode, endNode, false))

			wfor(CodingOperations::BreakLine,
				performIndentSelectionOperation(doc, startNode, endNode, false))
		}

		return nullptr;
	}


	void DocumentUtils::formatIndent(DocumentStruct* doc) {

		DocumentUtils::performCodingOperation(
			CodingOperations::AutoIndentSelection,
			doc,
			doc->firstRootNode,
			Cast::upcast(&doc->endOfFile)
		);

		auto* line = doc->firstCodeLine;
		while (line) {
			auto* firstElement = findFirstElementNode(line);
			if (firstElement) {


			}
			line = line->nextLine;
		}
	}

}
