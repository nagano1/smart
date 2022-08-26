#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <string>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <ctime>
#include <stdio.h>

#include "code_nodes.hpp"

namespace smart {

    // --------------------- Defines Document VTable ----------------------

    static int selfTextLength(DocumentStruct * self) {

        return 5;
    }

    static const char *selfText(DocumentStruct * self) {
        return "";
    }

    static CodeLine *appendToLine(DocumentStruct *self, CodeLine *currentCodeLine) {
        auto *child = self->firstRootNode;
        while (child) {
            currentCodeLine = VTableCall::appendToLine(child, currentCodeLine);
            child = child->nextNode;
        }
        currentCodeLine = VTableCall::appendToLine(&self->endOfFile, currentCodeLine);
        return currentCodeLine;
    }


    static constexpr const char DocumentTypeText[] = "<Document>";

    static const node_vtable DocumentVTable_ = CREATE_VTABLE(DocumentStruct, selfTextLength, selfText,
                                                       appendToLine, DocumentTypeText, NodeTypeId::Document);

    const node_vtable *const VTables::DocumentVTable = &DocumentVTable_;

    static void staticActionCreator(void *node1, void *node2, int actionRequest) {
        UNUSED(node1);
        UNUSED(node2);
        UNUSED(actionRequest);
    }

    // --------------------- Implements Document functions ----------------------
    DocumentStruct *Alloc::newDocument(
            DocumentType docType,
            void(*actionCreator)(void *node1, void *node2, int actionRequest)
    ) {
        UNUSED(actionCreator);

        auto *context = simpleMalloc2<ParseContext>();
        auto *doc = simpleMalloc2<DocumentStruct>();

        context->actionCreator = actionCreator != nullptr ? actionCreator : &staticActionCreator;

        INIT_NODE(doc, context, nullptr, VTables::DocumentVTable);
        INIT_NODE(&doc->endOfFile, context, Cast::upcast(doc), VTables::EndOfFileVTable);

        doc->documentType = docType;
        doc->firstRootNode = nullptr;
        doc->lastRootNode = nullptr;

        context->actionCreator(Cast::upcast(doc), nullptr,
                               EventType::CreateDocument); // create document

        doc->firstCodeLine = nullptr;
        doc->nodeCount = 0;

        context->memBuffer.init();
        /*
        context->spaceBufferList.init();
        context->lineBreakBufferList.init();
        context->charBuffer.init();
        context->codeLineBufferList.init();
         */

        return doc;
    }

    /*
        static inline void deleteLineNodes(CodeLine *line) {
            //assert(line != nullptr);
            if (line) {
                if (line->nextLine) {
                    deleteLineNodes(line->nextLine);
                }
                free(line);
            }
        }
     */

    void Alloc::deleteDocument(DocumentStruct *doc) {
        /*
        auto *node = doc->firstRootNode;
        while (node) {
            auto *nextNode = node->nextNode;
            if (node->vtable == VTables::ClassVTable) {
                Alloc::deleteClassNode(node);
            }
            node = nextNode;
        }
*/

        doc->context->memBuffer.freeAll();
        free(doc->context);
        free(doc);
    }


    utf8byte *DocumentUtils::getTextFromNode(NodeBase *node) {
        int len = VTableCall::selfTextLength(node);

        int prev_char = node->prev_chars; // = '\0' ? 1 : 0;
        auto *text = (char *) node->context->newMemArray<char>(len + 1 + prev_char);
        text[len + prev_char] = '\0';

        for (int i = 0; i < prev_char; i++) {
            text[i] = ' ';
        }

        if (len > 0) {
            auto *chs = VTableCall::selfText(node);
            memcpy(text + prev_char, chs, len);
        }

        return text;
    }

    utf8byte *DocumentUtils::getTextFromLine(CodeLine *line) {
        return nullptr;
        /*
        int totalCount = 0;
            auto *node = line->leftNode;
            while (node) {
                int len = VTableCall::selfTextLength(node);
                totalCount += len;
                node = node->nextNodeInLine;
            }

        auto *text = (char *)line->context->newMemArray<char>(totalCount+ 1);
        text[totalCount] = '\0';
        {
            auto *node = line->leftNode;
            size_t currentOffset = 0;
            while (node) {
                auto *chs = VTableCall::selfText(node);
                size_t len = VTableCall::selfTextLength(node);
                memcpy(text + currentOffset, chs, len);

                currentOffset += len;
                node = node->nextNodeInLine;
            }
        }
        return text;
        */

    }

    utf8byte *DocumentUtils::getTypeTextFromTree(DocumentStruct *doc) {
        // get size of chars
        int totalCount = 0;
        {
            auto *line = doc->firstCodeLine;
            while (line) {
                auto *node = line->firstNode;
                while (node) {
                    if (node->prev_chars > 0) {
                        totalCount += node->prev_chars;
                    }
                    int len = VTableCall::typeTextLength(node) + VTableCall::selfTextLength(node);
                    totalCount += len;
                    node = node->nextNodeInLine;
                }

                line = line->nextLine;
            }
        }

        if (totalCount == 0) {
            return nullptr;
        }

        // malloc and copy text
        auto *text = (char *) malloc(sizeof(char) * totalCount + 1);
        {
            auto *line = doc->firstCodeLine;
            size_t currentOffset = 0;
            while (line) {
                auto *node = line->firstNode;
                while (node) {
                    {
                        auto *chs = VTableCall::typeText(node);
                        size_t len = VTableCall::typeTextLength(node);
                        memcpy(text + currentOffset, chs, len);
                        currentOffset += len;
                    }

                    {
                        auto *chs = VTableCall::selfText(node);
                        if (node->prev_chars >0 ) {
                            for (int i = 0; i < node->prev_chars; i++) {
                                text[currentOffset] = ' ';
                                currentOffset++;
                            }
                        }

                        size_t len = VTableCall::selfTextLength(node);
                        if (len > 0) {
                            memcpy(text + currentOffset, chs, len);
                        }
                        currentOffset += len;
                    }

                    node = node->nextNodeInLine;
                }

                line = line->nextLine;
            }
        }

        text[totalCount] = '\0';

        return text;
    }



    JsonObjectStruct *DocumentUtils::generateHashTables(DocumentStruct *doc) {

        JsonObjectStruct *retJson = nullptr;
        auto *line = doc->firstCodeLine;
        while (line) {
            auto *node = line->firstNode;
            while (node) {
                if (node->vtable == VTables::JsonObjectVTable) {
                    auto *jsonObject = Cast::downcast<JsonObjectStruct *>(node);
                    retJson = jsonObject;

                    auto *keyItem = jsonObject->firstKeyValueItem;
                    while (keyItem) {
                        auto *keyNode = keyItem->keyNode;
                        jsonObject->hashMap->put(keyNode->text + keyNode->namePos,
                                                 keyNode->nameLength,
                                                 keyItem->valueNode);

                        keyItem = Cast::downcast<JsonKeyValueItemStruct *>(keyItem->nextNode);
                    }
                }

                node = node->nextNodeInLine;
            }

            line = line->nextLine;
        }

        return retJson;
    }



    static int getTokenTypeId(NodeBase *node, int i) { // NOLINT(readability-function-cognitive-complexity)

        auto *targetNode = node;
        if (targetNode->vtable == VTables::SimpleTextVTable) {
            targetNode = targetNode->parentNode;
        }
        
        if (targetNode->vtable == VTables::VariableVTable) {
            if (targetNode->parentNode->vtable == VTables::CallFuncVTable) {
                targetNode = targetNode->parentNode;
            }
        }

        if (targetNode->vtable == VTables::BlockCommentFragmentVTable
            || targetNode->vtable == VTables::LineCommentVTable) {
            return (int)TokenTypeIds::commentId;
        }
        else if (targetNode->vtable == VTables::BoolVTable) {
            return -1;
        }
        else if (targetNode->vtable == VTables::NullVTable) {
            return -1;
        }
        else if (targetNode->vtable == VTables::NumberVTable) {
            return (int)TokenTypeIds::numberId;
        }
        else if (targetNode->vtable == VTables::StringLiteralVTable) {
            return (int)TokenTypeIds::stringId;
        }
        else if (targetNode->vtable == VTables::CallFuncVTable) {
            return (int)TokenTypeIds::functionId;
        }
        else if (targetNode->vtable == VTables::VariableVTable) {
            return (int)TokenTypeIds::variableId;
        }
        else if (targetNode->vtable == VTables::SimpleTextVTable) {
            return -1;//(int)TokenTypeIds::keywordId;
        }
        else if (targetNode->vtable == VTables::ClassVTable) {
            return -1;//(int)TokenTypeIds::myclass;

        }
        else if (targetNode->vtable == VTables::FnVTable) {
            return -1;
        }
        else if (targetNode->vtable == VTables::SymbolVTable) {
            return (int)TokenTypeIds::keywordId;
        }
        else if (targetNode->vtable == VTables::AssignStatementVTable) {
            auto *assign = Cast::downcast<AssignStatementNodeStruct*>(targetNode);
            if (assign->typeOrLet.hasMutMark || assign->typeOrLet.hasNullableMark) {
                if (i == 0) {
                    return (int) TokenTypeIds::numberId;
                }
            }
            return -1;//(int) TokenTypeIds::keywordId;
        }
        else if (targetNode->vtable == VTables::NameVTable) {
            if (targetNode->parentNode->vtable == VTables::FnVTable) {
                return (int)TokenTypeIds::functionId;
            }
            else if (targetNode->parentNode->vtable == VTables::ClassVTable) {
                return (int)TokenTypeIds::classId;
            }
            else {
                return (int)TokenTypeIds::variableId;
            }
        }
        return -1;
    }

    static void splitCharsIfYouWant(NodeBase *node, int *len0, int *utf16Len0, int *len1, int *utf16Len1)
    {
        auto *chs = VTableCall::selfText(node);
        int len = VTableCall::selfTextLength(node);
        *utf16Len0 = ParseUtil::utf16_length(chs, len);

        auto *targetNode = node;
        if (targetNode->vtable == VTables::SimpleTextVTable) {
            targetNode = targetNode->parentNode;
        }

        if (targetNode->vtable == VTables::AssignStatementVTable) {
            auto *assign = Cast::downcast<AssignStatementNodeStruct*>(targetNode);
            if (assign->typeOrLet.hasMutMark || assign->typeOrLet.hasNullableMark) {
                *utf16Len1 = *utf16Len0 - 1;
                *len0 = 1;
                *utf16Len0 = 1;
                *len1 = len -1;
                return;
            }
        }


        if (ParseUtil::hasCharBeforeLineBreak(chs, len, 0)) {
            *len0 = len;
        }
    }

    static inline int addSemanticTokens(NodeBase *node, char *text, int currentLineNo, bool *first,
                                        int *prevSetLine, int *prevSetStart, int *charPos)
    {
        int len0 = 0, len1 = 0, utf16Len0 = 0, utf16Len1 = 0;
        splitCharsIfYouWant(node, &len0, &utf16Len0, &len1, &utf16Len1);

        int writeBytes = 0;

        for (int i = 0; i <= 1; i++) { // NOLINT(altera-unroll-loops)
            int len = i == 0 ? len0 : len1;
            int utf16len = i == 0 ? utf16Len0 : utf16Len1;
            if (len > 0) {
                int tokenTypeId = getTokenTypeId(node, i);
                if (tokenTypeId > -1) {
                    // { line: 2, startChar:  5, length: 3, tokenType: 0, tokenModifiers: 3 },
                    const char *tokenModifiersFlag = "0";
                    int wlen = sprintf(text + writeBytes,
                                       "%s%d,%d,%d,%d,%s",
                                       *first ? "" : ",",
                                       currentLineNo - *prevSetLine,
                                       *charPos - *prevSetStart,
                                       utf16len,
                                       tokenTypeId,
                                       tokenModifiersFlag
                    );

                    if (wlen > 0) {
                        *prevSetLine = currentLineNo;
                        *prevSetStart = *charPos;

                        if (*first) {
                            *first = false;
                        }
                        writeBytes += wlen;
                    }
                }
                *charPos += utf16len;
            }
        }
        return writeBytes;
    }


// { line: 2, startChar:  5, length: 3, tokenType: 0, tokenModifiers: 3 },
    static int getSemanticTokensLength(DocumentStruct *doc, char* text, int line0, int line1) {
        int totalByteCount = 0;
        {
            static char buff[255];

            auto *line = doc->firstCodeLine;
            int currentLineNo = 0;
            bool first = true;
            int prevLine = 0;
            while (line) { // NOLINT(altera-id-dependent-backward-branch)

                bool insideRange = true;
                if (line0 != -1) {
                    if (currentLineNo < line0 || line1 < currentLineNo) {
                        insideRange = false;
                    }
                }

                if (insideRange) {
                    auto *node = line->firstNode;

                    int charPos = 0;
                    int prevStart = 0;
                    while (node) { // NOLINT(altera-id-dependent-backward-branch,altera-unroll-loops)
                        charPos += node->prev_chars;

                        char *dst = text != nullptr ? text + totalByteCount : buff;
                        int writeBytes = addSemanticTokens(node, dst, currentLineNo, &first, &prevLine,
                                          &prevStart, &charPos);
                        totalByteCount += writeBytes;

                        node = node->nextNodeInLine;
                    }
                }
                line = line->nextLine;
                currentLineNo++;
            }
        }

        return totalByteCount;
    }


    utf8byte *DocumentUtils::getSemanticTokensTextFromTree(DocumentStruct *doc, int *len, int line0, int line1)
    {
        int totalCount = getSemanticTokensLength(doc, nullptr, line0, line1);

        *len = totalCount;
        auto *text = (char *) malloc(sizeof(char) * totalCount + 1);
        getSemanticTokensLength(doc, text, line0, line1);
        text[totalCount] = '\0';
        return text;
    }

    utf8byte *DocumentUtils::getTextFromTree(DocumentStruct *doc) {
        // get size of chars
        int totalCount = 0;
        {
            auto *line = doc->firstCodeLine;
            while (line) {
                auto *node = line->firstNode;
                while (node) {
                    if (node->prev_chars > 0) {
                        totalCount += node->prev_chars;
                    }
                    int len = VTableCall::selfTextLength(node);
                    totalCount += len;
                    node = node->nextNodeInLine;
                }

                line = line->nextLine;
            }
        }

        // malloc and copy text
        auto *text = (char *) malloc(sizeof(char) * totalCount + 1);
        text[totalCount] = '\0';
        if (totalCount > 0) {
            CodeLine *line = doc->firstCodeLine;
            size_t currentOffset = 0;
            while (line) {
                auto *node = line->firstNode;
                while (node) {
                    auto *chs = VTableCall::selfText(node);
                    if (node->prev_chars > 0) {
                        for (int i = 0; i < node->prev_chars; i++ ) {
                            text[currentOffset] = ' ';//node->prev_char;
                            currentOffset++;
                        }
                    }

                    size_t len = VTableCall::selfTextLength(node);
                    memcpy(text + currentOffset, chs, len);

                    currentOffset += len;
                    node = node->nextNodeInLine;
                }

                line = line->nextLine;
            }
        }

        return text;
    }


    static void appendRootNode(DocumentStruct *doc, NodeBase *node) {
        if (doc->firstRootNode == nullptr) {
            doc->firstRootNode = node;
        }
        if (doc->lastRootNode != nullptr) {
            doc->lastRootNode->nextNode = node;
        }
        doc->lastRootNode = node;
        doc->nodeCount++;
    }


    static int tryTokenize(TokenizerParams_parent_ch_start_context) {
        int result;

        if (-1 < (result = Tokenizers::classTokenizer(parent, ch, start, context))) {
            auto *doc = Cast::downcast<DocumentStruct *>(parent);
            appendRootNode(doc, context->virtualCodeNode);
            return result;
        } else if (-1 < (result = Tokenizers::fnTokenizer(parent, ch, start, context))) {
            auto* doc = Cast::downcast<DocumentStruct*>(parent);
            appendRootNode(doc, context->virtualCodeNode);
            return result;
        }

        if (ch != '\0') {
            context->setError(ErrorCode::syntax_error, start);
        }

        if (context->syntaxErrorInfo.hasError) {
            //throw 3;
        }

        return -1;
    }


    static int tryTokenizeJson(TokenizerParams_parent_ch_start_context) {
        int result;
        if (-1 < (result = Tokenizers::jsonObjectTokenizer(parent, ch, start, context))) {
            auto *doc = Cast::downcast<DocumentStruct *>(parent);
            appendRootNode(doc, context->virtualCodeNode);
            return result;
        }
        if (-1 < (result = Tokenizers::jsonArrayTokenizer(parent, ch, start, context))) {
            auto *doc = Cast::downcast<DocumentStruct *>(parent);
            appendRootNode(doc, context->virtualCodeNode);
            return result;
        }

        return -1;
    };

    static void callAllLineEvent(DocumentStruct *docStruct, CodeLine *line, ParseContext *context) {
        CodeLine *prev = nullptr;
        int lineCount = 0;
        while (line) {
            context->actionCreator(prev, line, EventType::CreateLine);
            lineCount++;
            prev = line;
            line = line->nextLine;
        }

        docStruct->lineCount = lineCount;
        // change first line of document
        context->actionCreator(docStruct, nullptr, EventType::FirstLineChanged);

    }



    void DocumentUtils::parseText(DocumentStruct *docStruct, const utf8byte *text, int length) {
        assert(docStruct->context != nullptr);

        auto *context = docStruct->context;
        context->syntaxErrorInfo.hasError = false;
        context->syntaxErrorInfo.errorCode = ErrorCode::no_syntax_error;
        context->syntaxErrorInfo.errorId = 10000;
        context->syntaxErrorInfo.charPosition = -1;
        context->syntaxErrorInfo.charPosition2 = -1;
        context->chars = const_cast<utf8byte *>(text);
        context->start = 0;
        context->scanEnd = false;
        context->length = length;
        //context->codeNode = nullptr;
        context->leftNode = nullptr;
        context->virtualCodeNode = nullptr;

        context->remainedLineBreakNode = nullptr;
        context->remainedCommentNode = nullptr;

        context->remaindPrevChars = 0;
        context->baseIndent = 4;
        context->parentDepth = -1;
        context->arithmeticBaseDepth = -1;
        context->afterLineBreak = false;

        context->unusedAssignment = nullptr;
        context->unusedClassNode = nullptr;



        if (docStruct->documentType == DocumentType::CodeDocument) {
            Scanner::scan_for_root(docStruct, tryTokenize, 0, context, /*root*/true, true);
        } else {
            Scanner::scan_for_root(docStruct, tryTokenizeJson, 0, context, /*root*/true, true);
        }

        
        if (!context->syntaxErrorInfo.hasError) {
            if (docStruct->lastRootNode) {
                //docStruct->lastRootNode->nextNode = Cast::upcast(&docStruct->endOfFile);
            }

            docStruct->lastRootNode = Cast::upcast(&docStruct->endOfFile);
            docStruct->lastRootNode->prev_chars = context->remaindPrevChars;
            docStruct->lastRootNode->prevLineBreakNode = context->remainedLineBreakNode;
            docStruct->lastRootNode->prevCommentNode = context->remainedCommentNode;

            docStruct->firstCodeLine = context->newCodeLine();// simpleMalloc<CodeLine>();
            docStruct->firstCodeLine->init(context);

            VTableCall::appendToLine(docStruct, docStruct->firstCodeLine);

            DocumentUtils::assignIndents(docStruct);
            DocumentUtils::checkIndentSyntaxErrors(docStruct);

            DocumentUtils::calcStackSize(docStruct);

            callAllLineEvent(docStruct, docStruct->firstCodeLine, context);
        }
    }
}
