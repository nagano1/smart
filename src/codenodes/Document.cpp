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

    // --------------------- Defines Document VTable ----------------------

    static int selfTextLength(DocumentStruct *self) {
        return 5;
    }

    static const char *selfText(DocumentStruct *self) {
        return "";
    };

    static CodeLine *appendToLine(DocumentStruct *self, CodeLine *currentCodeLine) {
        auto *doc = self;//Cast::downcast<DocumentStruct *>(self);
        //currentCodeLine = addPrevLineBreakNode(node, currentCodeLine);
        auto *child = doc->firstRootNode;
        while (child) {
            currentCodeLine = VTableCall::appendToLine(child, currentCodeLine);
            child = child->nextNode;
        }

        currentCodeLine = VTableCall::appendToLine(&doc->endOfFile, currentCodeLine);

        return currentCodeLine;
    };



    static constexpr const char DocumentTypeText[] = "<Document>";

    static const node_vtable _Document = CREATE_VTABLE(DocumentStruct,selfTextLength, selfText,
                                                       appendToLine, DocumentTypeText);

    const node_vtable *VTables::DocumentVTable = &_Document;

    static void staticActionCreator(void *node1, void *node2, int actionRequest) {

    }

    // --------------------- Implements Document functions ----------------------
    DocumentStruct *Alloc::newDocument(
            DocumentType docType,
            void (*actionCreator)(void *node1, void *node2, int actionRequest)
    ) {
        auto *doc = simpleMalloc<DocumentStruct>();
        auto *context = simpleMalloc<ParseContext>();

        context->actionCreator = actionCreator != nullptr ? actionCreator : &staticActionCreator;

        INIT_NODE(doc, context, nullptr, VTables::DocumentVTable);
        INIT_NODE(&doc->endOfFile, context, Cast::upcast(doc), VTables::EndOfFileVTable);

        doc->documentType = docType;
        doc->firstRootNode = nullptr;
        doc->lastRootNode = nullptr;

        context->actionCreator(Cast::upcast(doc), nullptr,
                               EventType::CreateDocument); // create document

        //doc->lastLineBreakNode = nullptr;
        //doc->lastSpaceNode = nullptr;

        doc->firstCodeLine = nullptr;
        doc->nodeCount = 0;

        context->spaceBufferList.init();
        context->lineBreakBufferList.init();
        context->charBuffer.init();
        context->codeLineBufferList.init();

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
        auto *node = doc->firstRootNode;
        while (node) {
            auto *nextNode = node->nextNode;
            if (node->vtable == VTables::ClassVTable) {
                Alloc::deleteClassNode(node);
            }
            node = nextNode;
        }

        deleteContext(doc->context);
        free(doc);
    }


    utf8byte *DocumentUtils::getTextFromNode(NodeBase *node) {
        int len = VTableCall::selfTextLength(node);
        int prev_char = node->prev_char != '\0' ? 1 : 0;

        auto *text = (char *) malloc(sizeof(char) * len + 1 + prev_char);
        text[len + prev_char] = '\0';

        int offset = 0;
        if (prev_char == 1) {
            text[0] = node->prev_char;
            offset++;
        }

        if (len > 0) {
            auto *chs = VTableCall::selfText(node);
            memcpy(text + offset, chs, len);
        }

        return text;
    }

    utf8byte *DocumentUtils::getTextFromLine(CodeLine *line) {
        int totalCount = 0;
        {
            auto *node = line->firstNode;
            while (node) {
                int len = VTableCall::selfTextLength(node);
                totalCount += len;
                node = node->nextNodeInLine;
            }
        }
        auto *text = (char *) malloc(sizeof(char) * totalCount + 1);
        text[totalCount] = '\0';
        {
            auto *node = line->firstNode;
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
    }

    utf8byte *DocumentUtils::getTypeTextFromTree(DocumentStruct *docStruct) {
        // get size of chars
        int totalCount = 0;
        {
            auto *line = docStruct->firstCodeLine;
            while (line) {
                auto *node = line->firstNode;
                while (node) {
                    if (node->prev_char != '\0') {
                        totalCount++;
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
        auto *text = (char *)malloc(sizeof(char) * totalCount + 1);
        text[totalCount] = '\0';
        {
            auto *line = docStruct->firstCodeLine;
            size_t currentOffset = 0;
            while (line) {
                auto *node = line->firstNode;
                while (node) {
                    auto *chs = VTableCall::typeText(node);
                    if (node->prev_char != '\0') {
                        text[currentOffset] = node->prev_char;
                        currentOffset++;
                    }

                    size_t len = VTableCall::typeTextLength(node);
                    memcpy(text + currentOffset, chs, len);

                    currentOffset += len;

                    {
                        auto *chs = VTableCall::selfText(node);
                        if (node->prev_char != '\0') {
                            text[currentOffset] = node->prev_char;
                            currentOffset++;
                        }

                        size_t len = VTableCall::selfTextLength(node);
                        memcpy(text + currentOffset, chs, len);

                        currentOffset += len;

                    }




                    node = node->nextNodeInLine;
                }

                line = line->nextLine;
            }
        }

        return text;
    }


    void DocumentUtils::generateHashTables(DocumentStruct *doc) {
        auto *line = doc->firstCodeLine;
        while (line) {
            auto *node = line->firstNode;
            while (node) {
                if (node->vtable == VTables::JsonObjectVTable) {
                    auto *jsonObject = Cast::downcast<JsonObjectStruct*>(node);
                    
                    auto *keyItem = jsonObject->firstKeyValueItem;

                }
                   
                node = node->nextNodeInLine;
            }

            line = line->nextLine;
        }
    }



    utf8byte *DocumentUtils::getTextFromTree(DocumentStruct *docStruct) {
        // get size of chars
        int totalCount = 0;
        {
            auto *line = docStruct->firstCodeLine;
            while (line) {
                auto *node = line->firstNode;
                while (node) {
                    if (node->prev_char != '\0') {
                        totalCount++;
                    }
                    int len = VTableCall::selfTextLength(node);
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
        text[totalCount] = '\0';
        {
            auto *line = docStruct->firstCodeLine;
            size_t currentOffset = 0;
            while (line) {
                auto *node = line->firstNode;
                while (node) {
                    auto *chs = VTableCall::selfText(node);
                    if (node->prev_char != '\0') {
                        text[currentOffset] = node->prev_char;
                        currentOffset++;
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
            appendRootNode(doc, context->codeNode);
            return result;
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
            appendRootNode(doc, context->codeNode);
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

    void DocumentUtils::parseText(DocumentStruct *docStruct, const utf8byte *text, size_t length) {
        assert(docStruct->context != nullptr);

        auto *context = docStruct->context;
        context->syntaxErrorInfo.hasError = false;
        context->chars = const_cast<utf8byte *>(text);
        context->start = 0;
        context->scanEnd = false;
        context->length = length;
        context->codeNode = nullptr;
        context->remainedLineBreakNode = nullptr;
        context->remainedSpaceNode = nullptr;


        if (docStruct->documentType == DocumentType::CodeDocument) {
            Scanner::scan_for_root(docStruct, tryTokenize, 0, context, /*root*/true);
        } else {
            Scanner::scan_for_root(docStruct, tryTokenizeJson, 0, context, /*root*/true);
        }

        if (docStruct->lastRootNode) {
            //docStruct->lastRootNode->nextNode = Cast::upcast(&docStruct->endOfFile);
        }

        docStruct->lastRootNode = Cast::upcast(&docStruct->endOfFile);
        docStruct->lastRootNode->prevSpaceNode = context->remainedSpaceNode;
        docStruct->lastRootNode->prevLineBreakNode = context->remainedLineBreakNode;

        docStruct->firstCodeLine = context->mallocCodeLine();// simpleMalloc<CodeLine>();
        docStruct->firstCodeLine->init(context);

        VTableCall::appendToLine(docStruct, docStruct->firstCodeLine);

        callAllLineEvent(docStruct, docStruct->firstCodeLine, context);

    }
}
