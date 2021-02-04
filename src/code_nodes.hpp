#pragma once

#include <stdlib.h>
#include <array>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <unordered_map>

#include <cstdint> // uint64_t, int_fast32_t
#include <ctime>

#include <string.h> // memcpy

#include "parse_util.hpp"
#include "common.hpp"

//using utf8byte = char;

namespace smart {

    struct ParseContext;
    struct CodeLine;


#define NODE_HEADER \
    const struct node_vtable *vtable; \
    _NodeBase *parentNode; \
    _NodeBase *nextNode; \
    _NodeBase *nextNodeInLine; \
    struct _SimpleTextNodeStruct *prevSpaceNode; \
    struct _LineBreakNodeStruct *prevLineBreakNode; \
    ParseContext *context; \
    char prev_char 


#define TEXT_MEMCPY(dst, src, len) \
        memcpy((dst), (src), (len))

#define INIT_NODE(node, context, parent, argvtable) \
        (node)->vtable = (argvtable); \
        (node)->prev_char = '\0'; \
        (node)->context = (context); \
        (node)->parentNode = (NodeBase*)(parent); \
        (node)->nextNode = nullptr; \
        (node)->nextNodeInLine = nullptr; \
        (node)->prevSpaceNode = nullptr; \
        (node)->prevLineBreakNode = nullptr; \
        32\

    //(node)->nextErrorNode = nullptr;

    using NodeBase = struct _NodeBase {
        NODE_HEADER;
    };


    using SimpleTextNodeStruct = struct _SimpleTextNodeStruct {
        NODE_HEADER;

        utf8byte *text;
        uint_fast32_t textLength;
    };

    using SpaceNodeStruct = SimpleTextNodeStruct;
    using NullNodeStruct = SimpleTextNodeStruct;

    using LineBreakNodeStruct = struct _LineBreakNodeStruct {
        NODE_HEADER;

        utf8byte text[3]; // "\r\n" or "\n" or "\r" plus "\0"
        _LineBreakNodeStruct *nextLineBreakNode = nullptr;
    };

    using EndOfFileNodeStruct = struct {
        NODE_HEADER;
    };


    using NameNodeStruct = struct {
        NODE_HEADER;

        char *name;
        size_t nameLength;
    };

    

    using StringLiteralNodeStruct = struct {
        NODE_HEADER;
        char *text;
        size_t textLength;

        char *strValue;
        size_t strValueLength;
    };

    using BoolNodeStruct = struct {
        NODE_HEADER;

        char *text;
        size_t textLength;
        bool boolValue;
    };


    using NumberNodeStruct = struct {
        NODE_HEADER;

        char *text;
        size_t textLength;
        int num;
        int unit;
    };

    using SymbolStruct = struct {
        NODE_HEADER;

        bool isEnabled;
        utf8byte symbol[2];
    };

    //    using ClassBodyStruct = struct {
    //        NODE_HEADER
    //
    //        bool isChecked;
    //        utf8byte body[2];
    //        // expressionNodes;
    //        SymbolStruct endBodyNode;
    //
    //        NodeBase *firstChildNode;
    //        NodeBase *lastChildNode;
    //    };



    using ClassNodeStruct = struct {
        NODE_HEADER;

        NameNodeStruct nameNode;

        struct Impl;
        struct Impl *sub;

        bool startFound;
        // utf8byte body[2];

        SymbolStruct bodyStartNode;
        SymbolStruct endBodyNode;

        NodeBase *firstChildNode;
        NodeBase *lastChildNode;
        int childCount;
    };

    using FuncBodyStruct = struct {
NODE_HEADER;

bool isChecked;
utf8byte body[2];
// expressionNodes;
SymbolStruct endBodyNode;
    };

    using FuncNodeStruct = struct {
        NODE_HEADER;

        NameNodeStruct nameNode;
        FuncBodyStruct bodyNode;
    };


    using JsonObjectKeyNodeStruct = struct {
        NODE_HEADER;

        char *text;
        size_t textLength;

        int namePos;
        size_t nameLength;
    };

    // --------- Json Key/Value --------- //
    using JsonKeyValueItemStruct = struct {
        NODE_HEADER;

        //utf8byte body[2];

        JsonObjectKeyNodeStruct *keyNode;

        NodeBase *valueNode;

        SymbolStruct delimeter;

        SymbolStruct follwingComma;
        bool hasComma;
    };


#define SIZE_TABLE 1024
#define hashnode_RANDOM 91231024
    struct HashNode {
        HashNode *next;
        char *key;
        int keyLength;
        NodeBase* nodeBase;
    };

    static int hash(char *key, int keyLength) {
        int sum = 0;
        for (int i = 0; i < keyLength; i++) {
            sum += key[i];
        }

        return sum % SIZE_TABLE;
    }

    struct HashMap {
        /*
         *
         */
        HashNode* entries[SIZE_TABLE];
        CharBuffer<char> charBuffer;


        template<std::size_t SIZE>
        void put2(const char(&f4)[SIZE], NodeBase* val) {
            return this->put((char*)f4, SIZE, val);
        }

        void put(char * keyA, int keyLength, NodeBase* val) {
            char *keyB = charBuffer.newChars(keyLength + 1);
            for (int i = 0; i < keyLength; i++) {
                keyB[i] = keyA[i];
            }
            keyB[keyLength] = '\0';

            auto hashInt = hash(keyA, keyLength);
            HashNode* hashNode = this->entries[hashInt];

            if (hashNode == nullptr) {// || hashNode->key == nullptr) {
                auto *newHashNode = simpleMalloc<HashNode>();
                newHashNode->next = nullptr;
                this->entries[hashInt] = newHashNode;

                newHashNode->key = keyB;
                newHashNode->keyLength = keyLength;
                newHashNode->nodeBase = val;
                return;
            }

            while (true) {
                // find same key
                bool sameKey = true;
                if (hashNode->keyLength == keyLength) {
                    for (int i = 0; i < keyLength; i++) {
                        if (hashNode->key[i] != keyA[i]) {
                            sameKey = false;
                            break;
                        }
                    }
                }
                else {
                    sameKey = false;
                }
                
                if (sameKey) {
                    hashNode->nodeBase = val;
                    return;
                }

                if (hashNode->next == nullptr) {
                    break;
                }
                hashNode = hashNode->next;
            }


            auto *newHashNode = simpleMalloc<HashNode>();
            newHashNode->key = keyB;
            newHashNode->keyLength = keyLength;
            newHashNode->nodeBase = val;
            newHashNode->next = nullptr;
            this->entries[hashInt] = newHashNode;


            hashNode->next = newHashNode;

        }

        void init() {
            charBuffer.init();

            for (int i = 0; i < SIZE_TABLE; i++) {
                this->entries[i] = nullptr;
            }
        }

        bool has(char * key, int keyLength) {
            return this->entries[hash(key, keyLength)]->key != nullptr;
        }

        void deleteKey(char * key, int keyLength) {
            if (this->entries[hash(key, keyLength)] != nullptr) {
                free(this->entries[hash(key, keyLength)]);
            }
        }

        template<std::size_t SIZE>
        NodeBase* get2(const char(&f4)[SIZE]) {
            return this->get((char*)f4, SIZE);
        }

        NodeBase* get(char * key, int keyLength) {
            auto keyInt = hash(key, keyLength);
            if (this->entries[keyInt] != nullptr) {
                auto * hashNode = this->entries[keyInt];
                while (hashNode) {
                    if (0 == strcmp(hashNode->key, key)) {
                        return hashNode->nodeBase;
                    }
                    hashNode = hashNode->next;
                }
            }
            return nullptr;
        }
    };


    // --------- Json Object --------- //
    using JsonObjectStruct = struct {
        NODE_HEADER;

        int parsePhase;

        utf8byte body[2]; // '{'
        SymbolStruct endBodyNode;
        JsonKeyValueItemStruct *firstKeyValueItem;
        JsonKeyValueItemStruct *lastKeyValueItem;
        HashMap *hashMap;
    };

    // --------- Json Array Item --------- //
    using JsonArrayItemStruct = struct {
        NODE_HEADER;

        NodeBase *valueNode;
        SymbolStruct follwingComma;
        bool hasComma;
    };


    // --------- Json Array --------- //
    using JsonArrayStruct = struct {
        NODE_HEADER;

        int parsePhase;

        utf8byte body[2]; // '{'
        SymbolStruct endBodyNode;
        JsonArrayItemStruct *firstItem;
        JsonArrayItemStruct *lastItem;
    };


    enum DocumentType {
        CodeDocument,
        JsonDocument
    };

    using DocumentStruct = struct _documentStruct {
        NODE_HEADER;

        DocumentType documentType;
        char *fileName;
        EndOfFileNodeStruct endOfFile;

        //SpaceNodeStruct *lastSpaceNode;
        //LineBreakNodeStruct *lastLineBreakNode;
        //NodeBase *lastErrorNode;

        int nodeCount{ 0 };

        CodeLine *firstCodeLine;
        int lineCount;

        NodeBase *firstRootNode;
        NodeBase *lastRootNode;
    };

    /*
    struct TextBuffer {
    private:
        // bufferMode == 1: has Next Buffer
        utf8byte *allBuffer;
        int allBufferLength;

    public:
        int bufferMode = 0;
        int bufferOffset;
        int currentBufferLength;
        utf8byte *currentBuffer;
        bool (*nextBufferGetter)(TextBuffer *textBuffer);

        bool waitNextTextBuffer() {
            if (this->bufferMode == 0) {
                return false;
            } else {
            }
            return true;
        };
    };
    */

    /**
     * Syntax error is allowed only once
     */
    using SyntaxErrorInfo = struct _errorInfo {
        bool hasError{ false };

        int errorCode = 100;
        char *reason;

        int charPosition;

        int startLine;
        int endLine;

        int startCharacter;
        int endCharacter;


    };

    struct ParseContext {
        int start;
        size_t length;
        bool scanEnd;
        NodeBase *codeNode;
        int former_start;
        utf8byte *chars;
        SyntaxErrorInfo syntaxErrorInfo;
        bool has_cancel_request{ false };

        void(*actionCreator)(void *node1, void *node2, int actionRequest);


        LineBreakNodeStruct *remainedLineBreakNode;
        SpaceNodeStruct *remainedSpaceNode;

        NodeBufferList<SimpleTextNodeStruct> spaceBufferList;
        NodeBufferList<CodeLine> codeLineBufferList;
        NodeBufferList<LineBreakNodeStruct> lineBreakBufferList;
        CharBuffer<char> charBuffer;


        LineBreakNodeStruct *mallocLineBreakNode() {
            return lineBreakBufferList.newNode();
        }

        CodeLine *mallocCodeLine() {
            return codeLineBufferList.newNode();
        }

        SpaceNodeStruct *mallocSpaceNode() {
            return spaceBufferList.newNode();
        }

        NullNodeStruct *mallocNullNode() {
            return spaceBufferList.newNode();
        }
    };


    static inline void deleteContext(ParseContext *context) {
        deleteNodeBufferList(context->lineBreakBufferList.firstBufferList);
        deleteNodeBufferList(context->codeLineBufferList.firstBufferList);
        deleteNodeBufferList(context->spaceBufferList.firstBufferList);
        deleteCharBuffer(context->charBuffer.firstBufferList);

        free(context);
    }


    struct Cast {
        template<typename T>
        static inline T downcast(NodeBase *node) {
            return (T)node;
        }

        template<typename T>
        static inline NodeBase *upcast(T *node) {
            return (NodeBase *)node;
        }
    };


#define VTABLE_DEF(T) \
        int (*selfTextLength)(T *self); \
        const utf8byte *(*selfText)(T *self); \
        CodeLine *(*appendToLine)(T *self, CodeLine *line); \
        const char *typeChars; \
        int typeCharsLength; \



    /**
     * code node vtable
     */
    using NodeVTable = struct node_vtable {
        VTABLE_DEF(NodeBase)
    };

    // this is for static type checking
    template<typename T>
    struct vtableT {
        VTABLE_DEF(T)
    };

    using selfTextLengthFunction = decltype(std::declval<NodeVTable>().selfTextLength);
    using selfTextFunction = decltype(std::declval<NodeVTable>().selfText);
    using appendToLineFunction = decltype(std::declval<NodeVTable>().appendToLine);

    template<typename T, std::size_t SIZE>
    static int vtable_type_check(
        decltype(std::declval<vtableT<T>>().selfTextLength) f1,
        decltype(std::declval<vtableT<T>>().selfText) f2,
        decltype(std::declval<vtableT<T>>().appendToLine) f3,
        const char(&f4)[SIZE]
    ) {
        return 0;
    }

#define CREATE_VTABLE(T, f1, f2, f3, f4) \
        node_vtable { \
            reinterpret_cast<selfTextLengthFunction> (f1) \
            , reinterpret_cast<selfTextFunction> (f2) \
            , reinterpret_cast<appendToLineFunction> (f3) \
            , (const char *)(f4) \
            , (sizeof(f4)-1) \
        } \
        ;static const int check_result_##T = vtable_type_check<T>(f1,f2,f3,f4)
    // static_assert(std::is_same<F2, decltype(std::declval<vtableT<T>>().selfText)>::value, "");

    struct VTables {
        static const node_vtable
            *DocumentVTable,
            *ClassVTable,

            *JsonObjectVTable,
            *JsonArrayVTable,
            *JsonKeyValueItemVTable,
            *JsonArrayItemVTable,
            *JsonObjectKeyVTable,

            *ClassBodyVTable,
            *NameVTable,
            *StringLiteralVTable,
            *NumberVTable,
            *BoolVTable,
            *SymbolVTable,
            *EndOfFileVTable,
            *SimpleTextVTable,
            *NullVTable,
            *SpaceVTable,
            *LineBreakVTable;
    };


    struct VTableCall {

        static int selfTextLength(NodeBase *node) {
            assert(node);
            assert(node->vtable);
            assert(node->vtable->selfTextLength);

            return node->vtable->selfTextLength(node);
        }


        static inline const utf8byte *selfText(void *node) {
            if (node == nullptr) {
                return "";
            }
            else {
                auto *nodeBase = Cast::upcast(node);
                return nodeBase->vtable->selfText(nodeBase);
            };
        }

        static int typeTextLength(NodeBase *node) {
            assert(node);
            assert(node->vtable);
            return node->vtable->typeCharsLength;
        }


        static inline const utf8byte *typeText(void *node) {
            if (node == nullptr) {
                return "";
            }
            else {
                auto *nodeBase = Cast::upcast(node);
                return nodeBase->vtable->typeChars;
            };
        }


        static CodeLine *appendToLine(void *node, CodeLine *currentCodeLine);

    };


    struct CodeLine {

        CodeLine *nextLine;
        CodeLine *prev;
        NodeBase *firstNode;
        NodeBase *lastNode;
        int indent;

        void init(ParseContext *context) {
            this->firstNode = nullptr;
            this->lastNode = nullptr;
            this->nextLine = nullptr;
            this->prev = nullptr;

            //            context->actionCreator(Cast::upcast(doc), 1);

        }

        CodeLine *appendNode(void *node) {
            if (firstNode == nullptr) {
                firstNode = (NodeBase *)node;
            }

            if (lastNode != nullptr) {
                lastNode->nextNodeInLine = (NodeBase *)node;
            }

            lastNode = (NodeBase *)node;

            return this;
        }

        CodeLine *addPrevLineBreakNode(void *node) {
            CodeLine *currentCodeLine = this;
            currentCodeLine = VTableCall::appendToLine(((NodeBase *)node)->prevLineBreakNode,
                currentCodeLine);
            currentCodeLine = VTableCall::appendToLine(((NodeBase *)node)->prevSpaceNode,
                currentCodeLine);
            return currentCodeLine;
        }

    };





    /**
     * Node Changed Event
     */
    enum EventType {
        CreateDocument,
        FirstLineChanged,
        CreateNode,
        CreateLine,
    };

    struct DocumentUtils {
        static void parseText(DocumentStruct *docStruct, const utf8byte *text, size_t length);

        static JsonObjectStruct* generateHashTables(DocumentStruct *doc);

        static utf8byte *getTextFromTree(DocumentStruct *docStruct);
        static utf8byte *getTypeTextFromTree(DocumentStruct *docStruct);

        static utf8byte *getTextFromLine(CodeLine *line);

        static utf8byte *getTextFromNode(NodeBase *line);
    };


    struct Init {
        static void initNameNode(NameNodeStruct *name, ParseContext *context, NodeBase *parentNode);
        static void initStringLiteralNode(StringLiteralNodeStruct *name, ParseContext *context, NodeBase *parentNode);

        static void initSymbolNode(SymbolStruct *self, ParseContext *context, void *parentNode,
            utf8byte letter);
    };

    struct Alloc {
        static NumberNodeStruct *newNumberNode(ParseContext *context, NodeBase *parentNode);
        static BoolNodeStruct *newBoolNode(ParseContext *context, NodeBase *parentNode);


        static LineBreakNodeStruct *newLineBreakNode(ParseContext *context, NodeBase *parentNode);
        //static ErrorNodeStruct *newErrorNode(ParseContext *context, NodeBase *parentNode);

        static SimpleTextNodeStruct *newSimpleTextNode(ParseContext *context, NodeBase *parentNode);

        static SpaceNodeStruct *newSpaceNode(ParseContext *context, NodeBase *parentNode);
        static NullNodeStruct *newNullNode(ParseContext *context, NodeBase *parentNode);

        static ClassNodeStruct *newClassNode(ParseContext *context, NodeBase *parentNode);

        static JsonKeyValueItemStruct *
            newJsonKeyValueItemNode(ParseContext *context, NodeBase *parentNode);

        static void deleteClassNode(NodeBase *node);

        static ClassNodeStruct *newFuncNode(ParseContext *context, NodeBase *parentNode);

        static void deleteFuncNode(NodeBase *node);

        static JsonObjectStruct *newJsonObject(ParseContext *context, NodeBase *parentNode);
        static JsonArrayStruct *newJsonArray(ParseContext *context, NodeBase *parentNode);
        static JsonArrayItemStruct *newJsonArrayItem(ParseContext *context, NodeBase *parentNode);
        static JsonObjectKeyNodeStruct *newJsonObjectKeyNode(ParseContext *context, NodeBase *parentNode);


        static void deleteJsonObject(NodeBase *node);
        static void deleteJsonArray(NodeBase *node);


        static DocumentStruct *newDocument(
            DocumentType docType,
            void(*actionCreator)(void *node1, void *node2, int actionRequest)
        );

        static void deleteDocument(DocumentStruct *doc);

    };


    /**
 * Function Types and vtable for node structures
 */
#define TokenizerParams_parent_ch_start_context \
        NodeBase *parent, utf8byte ch, int start, ParseContext *context

#define TokenizerParams_pass parent, ch, start, context

    using TokenizerFunction = int(*)(TokenizerParams_parent_ch_start_context);


    struct Tokenizers {
        static int nameTokenizer(TokenizerParams_parent_ch_start_context);
        static int numberTokenizer(TokenizerParams_parent_ch_start_context);
        static int nullTokenizer(TokenizerParams_parent_ch_start_context);
        static int stringLiteralTokenizer(TokenizerParams_parent_ch_start_context);

        static int boolTokenizer(TokenizerParams_parent_ch_start_context);

        static int classTokenizer(TokenizerParams_parent_ch_start_context);

        //static int classBodyTokenizer(TokenizerParams_parent_ch_start_context);

        static int jsonObjectTokenizer(TokenizerParams_parent_ch_start_context);
        static int jsonArrayTokenizer(TokenizerParams_parent_ch_start_context);

        static int jsonObjectNameTokenizer(TokenizerParams_parent_ch_start_context);

        static int jsonValueTokenizer(TokenizerParams_parent_ch_start_context);


        // SimpleTextNodeStruct
        template<typename TYPE, std::size_t SIZE>
        static inline int WordTokenizer(TokenizerParams_parent_ch_start_context, char capitalLetter, const TYPE(&word)[SIZE]) {
            if (capitalLetter == ch) {
                size_t length = sizeof(word) - 1;
                if (ParseUtil::matchWord(context->chars, context->length, word, length, start)) {

                    if (start + length == context->length // allowed to be the last char of the file
                        || ParseUtil::isNonIdentifierChar(context->chars[start + length])) { // otherwise,

                        //context->scanEnd = true;
                        auto *boolNode = Alloc::newSpaceNode(context, parent);

                        boolNode->text = context->charBuffer.newChars(length + 1);
                        boolNode->textLength = length;

                        TEXT_MEMCPY(boolNode->text, context->chars + start, length);
                        boolNode->text[length] = '\0';

                        context->codeNode = Cast::upcast(boolNode);
                        return start + length;
                    }
                }
            }

            return -1;
        }
    };



    /**
     * Implements common scanning and parsing method
     */
    struct Scanner {
        static int scan_for_root(
            void *parentNode,
            TokenizerFunction tokenizer,
            int start,
            ParseContext *context,
            bool root
        );

        static int scan(
            void *parentNode,
            TokenizerFunction tokenizer,
            int start,
            ParseContext *context
        );

        static int scanErrorNodeUntilSpace(
            void *parentNode,
            int start,
            ParseContext *context
        );
    };
}

/**
 *
 * Abstract Message
 *    A general message as defined by JSON-RPC.
 *    The language server protocol always uses “2.0” as the jsonrpc version.
 */
struct Message {
public:
    //std::string jsonrpc; // jsonrpc: string;
};


/**
 *  Request Message
 *     A request message to describe a request between the client and the server.
 *     Every processed request must send a response back to the sender of the request.
 */
struct RequestMessage : Message {

    /**
     * The request id.
     */
     //std::string id; // id: number | std::string;

     /**
      * The method to be invoked.
      */
      //std::string method; // method: string;

      /**
       * The method's params.
       */
       //std::vector<std::string> params; //params?: array | object;

};

