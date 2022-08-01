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
#include "errors.hpp"


//using utf8byte = char;
/*
 *
 * aewffweoif
 *     <int>
 *     () {
 *
 * }
 *
 *
 */
namespace smart {

    struct ParseContext;
    struct CodeLine;

    #define NODE_HEADER \
        const struct node_vtable *vtable; \
        _NodeBase *parentNode; \
        _NodeBase *nextNode; \
        _NodeBase *nextNodeInLine; \
        CodeLine *line; \
        int indentType; \
        void *prevCommentNode; \
        struct _LineBreakNodeStruct *prevLineBreakNode; \
        ParseContext *context; \
        int found; \
        int prev_chars


    #define INIT_NODE(node, context, parent, argvtable) \
        (node)->vtable = (argvtable); \
        (node)->prev_chars = 0; \
        (node)->context = (context); \
        (node)->parentNode = (NodeBase*)(parent); \
        (node)->line = nullptr; \
        (node)->found = -1; \
        (node)->nextNode = nullptr; \
        (node)->nextNodeInLine = nullptr; \
        (node)->prevLineBreakNode = nullptr; \
        (node)->prevCommentNode = nullptr; \
        (0)

    #define TEXT_MEMCPY(dst, src, len) \
        memcpy((dst), (src), (len))

    using NodeBase = struct _NodeBase {
        NODE_HEADER;
    };


    using SimpleTextNodeStruct = struct _SimpleTextNodeStruct {
        NODE_HEADER;

        utf8byte *text;
        int_fast32_t textLength;
    };

    using SpaceNodeStruct = SimpleTextNodeStruct;
    using NullNodeStruct = SimpleTextNodeStruct;
    using LineCommentNodeStruct = SimpleTextNodeStruct;
    using BlockCommentFragmentStruct = SimpleTextNodeStruct;

    using LineBreakNodeStruct = struct _LineBreakNodeStruct {
        NODE_HEADER;

        utf8byte text[3]; // "\r\n" or "\n" or "\r" plus "\0"
        _LineBreakNodeStruct *nextLineBreakNode;
    };

    using BlockCommentNodeStruct = struct {
        NODE_HEADER;

        BlockCommentFragmentStruct *firstCommentFragment;
    };


    using EndOfFileNodeStruct = struct {
        NODE_HEADER;

        LineCommentNodeStruct *prevLineCommentNode;
    };


    using NameNodeStruct = struct {
        NODE_HEADER;

        char *name;
        int_fast32_t nameLength;
    };

    using VariableNodeStruct = NameNodeStruct;

    using TypeNodeStruct = struct _TypeNodeStruct {
        NODE_HEADER;

        NameNodeStruct nameNode;
        _TypeNodeStruct *typeNode; // generics
    };



    using StringLiteralNodeStruct = struct {
        NODE_HEADER;
        char *text; // unparsed, includes ""
        int_fast32_t textLength;

        char *str;
        int strLength;

        int literalType; // 0: "text", 1: `wjfeiofw`, 2: r"testfaojiwe"
    };

    using BoolNodeStruct = struct {
        NODE_HEADER;

        char *text;
        int textLength;
        bool boolValue;
    };


    using NumberNodeStruct = struct {
        NODE_HEADER;

        char *text;
        int textLength;
        int num;
        int unit;
    };

    using SymbolStruct = struct {
        NODE_HEADER;

        bool isEnabled;
        utf8byte symbol[2];
    };

    using AssignStatementNodeStruct = struct {
        NODE_HEADER;

        // $int a = 5
        // ?let *ptr = "jfwio"

        bool hasMutMark; // $
        bool hasNullableMark; // ?

        bool useLet; // or has type
        bool onlyAssign; // has not declare
        SimpleTextNodeStruct letOrType; // $let, int, ?string, etc..
        SymbolStruct pointerAsterisk; // *

        NameNodeStruct nameNode; // varName
        SymbolStruct equalSymbol; // =
        NodeBase *valueNode; // 32
    };

    using KeywordAndValueStruct = struct {
        NODE_HEADER;

        SimpleTextNodeStruct returnText;
        NodeBase *valueNode;
    };

    using ReturnStatementNodeStruct = KeywordAndValueStruct; // return 32

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

    using BodyNodeStruct = struct {
        NODE_HEADER;

        bool startFound;
        bool firstStatementFound;

        SymbolStruct bodyStartNode;
        SymbolStruct endBodyNode;

        NodeBase *firstChildNode;
        NodeBase *lastChildNode;
        int childCount;
    };


    using FuncNodeStruct = struct {
        NODE_HEADER;

        NameNodeStruct nameNode;

        SymbolStruct parameterStartNode;
        SymbolStruct parameterEndNode;

        bool parameterStartFound;
        bool parameterEndFound;

        BodyNodeStruct bodyNode;

        NodeBase *firstChildParameterNode;
        NodeBase *lastChildParameterNode;
        int parameterChildCount;
    };


    /* (mut point: Point) */
    using FuncParameterItemStruct = struct {
        NODE_HEADER;

        NameNodeStruct nameNode;

        SymbolStruct delimeter;

        TypeNodeStruct typeNode;

        SymbolStruct follwingComma;
        bool hasComma;
    };


    using JsonObjectKeyNodeStruct = struct {
        NODE_HEADER;

        char *text;
        int_fast32_t textLength;

        int namePos;
        int nameLength;
    };

    using JsonKeyValueItemStruct = struct {
        NODE_HEADER;

        JsonObjectKeyNodeStruct *keyNode;

        NodeBase *valueNode;

        SymbolStruct delimeter;

        SymbolStruct follwingComma;
        bool hasComma;
    };


    #define HashNode_TABLE_SIZE 104

    struct HashNode {
        HashNode *next;
        char *key;
        int keyLength;
        NodeBase *nodeBase;
    };

    struct HashMap {
        HashNode **entries;// [HashNode_TABLE_SIZE] = {};
        size_t entries_length;
        ParseContext *context;
        //MallocBuffer charBuffer;

        void init(ParseContext *context);

        template<std::size_t SIZE>
        static int calc_hash2(const char(&f4)[SIZE], size_t max) {
            return HashMap::calc_hash((const char *) f4, SIZE - 1, max);
        }
        int calc_hash0(const char *key, int keyLength) {
            return HashMap::calc_hash(key, keyLength, this->entries_length);
        }
        static int calc_hash(const char *key, int keyLength, size_t max);
        void put(const char *keyA, int keyLength, NodeBase *val) const;
        NodeBase *get(const char *key, int keyLength);
        bool has(const char *key, int keyLength);
        void deleteKey(const char *key, int keyLength);

        template<std::size_t SIZE>
        NodeBase *get2(const char(&f4)[SIZE]) {
            return this->get((const char *) f4, SIZE - 1);
        }
        template<std::size_t SIZE>
        void put2(const char(&f4)[SIZE], NodeBase *val) {
            return this->put((const char *) f4, SIZE - 1, val);
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

        int nodeCount{0};

        CodeLine *firstCodeLine;
        int lineCount;

        NodeBase *firstRootNode;
        NodeBase *lastRootNode;
    };

    struct ParseContext {
        st_uint start;
        int length;
        bool scanEnd;
        int prevFoundPos;


        bool afterLineBreak;
        NodeBase *codeNode;
        NodeBase *virtualCodeNode;
        // int former_start;
        int baseIndent;
        utf8byte *chars;
        SyntaxErrorInfo syntaxErrorInfo;
        bool has_cancel_request{false};
        bool has_depth_error{false};
        st_int parentDepth{ -1 };

        // node caches
        AssignStatementNodeStruct *unusedAssignment;
        ClassNodeStruct *unusedClassNode;


        void (*actionCreator)(void *node1, void *node2, int actionRequest);


        LineBreakNodeStruct *remainedLineBreakNode;
        void *remainedCommentNode;
        int remaindPrevChars{0};

        MemBuffer memBuffer;

        template<typename T>
        T *newMem() {
            return (T *) memBuffer.newMem<T>(1);
        }

        template<typename T>
        void tryDelete(T *m) {
            return (T *) memBuffer.tryDelete(m);
        }

        template<typename T>
        T *newMemArray(st_size len) {
            return (T *) memBuffer.newMem<T>(len);
        }


        LineBreakNodeStruct *newLineBreakNode() {
            return memBuffer.newMem<LineBreakNodeStruct>(1);
        }

        CodeLine *newCodeLine() {
            return memBuffer.newMem<CodeLine>(1);
        }

        template<typename T>
        SpaceNodeStruct *newMemForNode() {
            return memBuffer.newMem<T>(1);
            //return spaceBufferList.newNode();
        }

        void setError(ErrorCode errorCode, st_int startPos) {

            auto &errorInfo = this->syntaxErrorInfo;
            if (errorInfo.hasError) {
                return;
            }

            int a = 0, b = 0;
            if (this->length == startPos) {
                startPos--;
            }

            getLineAndPos(startPos, this->chars, this->length,
                          reinterpret_cast<int *>(&a),
                          reinterpret_cast<int *>(&b));

            errorInfo.charPosition = startPos;


            this->setError3(errorCode, a, b, -1, -1);
        }

        void setError2(ErrorCode errorCode, st_int startPos, st_int startPos2) {
            auto &errorInfo = this->syntaxErrorInfo;
            if (errorInfo.hasError) {
                return;
            }

            if (this->length == startPos) {
                startPos--;
            }
            if (this->length == startPos2) {
                startPos2--;
            }

            int a, b, c, d;
            getLineAndPos(startPos, this->chars, this->length,
                          reinterpret_cast<int *>(&a),
                          reinterpret_cast<int *>(&b));

            getLineAndPos(startPos2, this->chars, this->length,
                          reinterpret_cast<int *>(&c),
                          reinterpret_cast<int *>(&d));


            errorInfo.charPosition = startPos;
            errorInfo.charPosition2 = startPos2;

            this->setError3(errorCode, a, b, c, d);
        }


        void setIndentError(ErrorCode errorCode, st_int line1, st_int charPos1) {
            auto &errorInfo = this->syntaxErrorInfo;
            if (errorInfo.hasError) {
                return;
            }
            this->setError3(errorCode, line1, charPos1, -1, -1);
        }

        void setError3(ErrorCode errorCode, st_int line1, st_int charPos1, st_int line2, st_int charPos2) {
            auto &errorInfo = this->syntaxErrorInfo;
            if (errorInfo.hasError) {
                return;
            }

            errorInfo.linePos1 = line1;
            errorInfo.linePos2 = line2;
            errorInfo.charPos1 = charPos1;
            errorInfo.charPos2 = charPos2;


            errorInfo.hasError = true;
            errorInfo.errorCode = errorCode;

            errorInfo.errorId = getErrorId(errorCode);
            const char* reason = getErrorMessage(errorCode);
            if (reason == nullptr) {
                reason = "";
            }
            int len = (int)strlen(reason);
            errorInfo.reasonLength = len < MAX_REASON_LENGTH ? len : MAX_REASON_LENGTH;
            memcpy(errorInfo.reason, reason, errorInfo.reasonLength);
            errorInfo.reason[errorInfo.reasonLength] = '\0';
        }

        static bool getLineAndPos(int pos, const utf8byte *text, int textLength, int *line, int *charactor) {
            int currentLine = 0;
            int currentCharactor = 0;
            int lineFirstPos = 0;

            for (int32_t i = 0; i < textLength; i++) {

                if (i == pos) {
                    *line = currentLine;
                    *charactor = ParseUtil::utf16_length(text + lineFirstPos, currentCharactor);
                    return true;
                }

                currentCharactor++;

                utf8byte ch = text[i];
                //if (ParseUtil::isBreakLine(ch)) {
                if ('\n' == ch) {
                    currentCharactor = 0;
                    currentLine++;
                    lineFirstPos = i;
                }
            }


            return false;
        }

    };


    struct Cast {
        template<typename T>
        static inline T downcast(NodeBase *node) {
            return (T) node;
        }

        template<typename T>
        static inline NodeBase *upcast(T *node) {
            return (NodeBase *) node;
        }
    };

    enum class NodeTypeId {
        Document = 0,
        EndOfDoc = 1,
        StringLiteral = 2,
        Symbol = 3,
        Class = 4,
        Name = 5,
        SimpleText = 6,


        Number = 9,
        LineBreak = 10,
        Bool = 11,


        JsonObject = 12,
        JsonObjectKey = 13,
        JsonKeyValueItem = 14,

        JsonArrayItem = 7,
        JsonArrayStruct = 8,
        
        Space = 15,

        Func = 17,
        NULLId = 16,

        Type = 18,
        Body = 19,
        AssignStatement = 20,
        ReturnStatement = 24,

        LineComment = 21,
        BlockComment = 22,
        BlockCommentFragment = 23,

        Variable = 25,

    };

    #define VTABLE_DEF(T) \
        int (*selfTextLength)(T *self); \
        const utf8byte *(*selfText)(T *self); \
        CodeLine *(*appendToLine)(T *self, CodeLine *line); \
        const char *typeChars; \
        int typeCharsLength; \
        NodeTypeId nodeTypeId; \


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
            const char(&f4)[SIZE],
            NodeTypeId f5
    ) noexcept {
        return 0;
    }
    #define CREATE_VTABLE(T, f1, f2, f3, f4, f5) \
        node_vtable { \
            reinterpret_cast<selfTextLengthFunction> (f1) \
            , reinterpret_cast<selfTextFunction> (f2) \
            , reinterpret_cast<appendToLineFunction> (f3) \
            , (const char *)(f4) \
            , (sizeof(f4)-1) \
            , f5 \
        } \
        ;static const int check_result_##T = vtable_type_check<T>(f1,f2,f3,f4,f5)
    // static_assert(std::is_same<F2, decltype(std::declval<vtableT<T>>().selfText)>::value, "");

    struct VTables {
        static const node_vtable
                *const DocumentVTable,

                *const AssignStatementVTable,
                *const ReturnStatementVTable,

                *const ClassVTable,

                *const FnVTable,

                *const NameVTable,
                *const VariableVTable,
                *const BodyVTable,
                *const TypeVTable,
                *const StringLiteralVTable,
                *const NumberVTable,
                *const BoolVTable,
                *const SymbolVTable,
                *const SimpleTextVTable,
                *const NullVTable,
                *const SpaceVTable,
                *const LineBreakVTable,

                *const LineCommentVTable,
                *const BlockCommentVTable,
                *const BlockCommentFragmentVTable,


        *const JsonObjectVTable,
                *const JsonArrayVTable,
                *const JsonKeyValueItemVTable,
                *const JsonArrayItemVTable,
                *const JsonObjectKeyVTable,

                *const EndOfFileVTable;
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
            } else {
                auto *nodeBase = Cast::upcast(node);
                return nodeBase->vtable->selfText(nodeBase);
            }
        }

        static int typeTextLength(NodeBase *node) {
            assert(node);
            assert(node->vtable);
            return node->vtable->typeCharsLength;
        }


        static inline const utf8byte *typeText(void *node) {
            if (node == nullptr) {
                return "";
            } else {
                auto *nodeBase = Cast::upcast(node);
                return nodeBase->vtable->typeChars;
            }
        }


        static CodeLine *appendToLine(void *node, CodeLine *currentCodeLine);

    };


    struct CodeLine {
        CodeLine *nextLine;
        //CodeLine *prev;
        NodeBase *firstNode;
        NodeBase *lastNode;
        // int indent;
        int depth;

        void init(ParseContext *context) {
            this->firstNode = nullptr;
            this->lastNode = nullptr;
            this->nextLine = nullptr;
            ///this->prev = nullptr;
            this->depth = 0;

            // context->actionCreator(Cast::upcast(doc), 1);
        }

        CodeLine *insertNode(NodeBase *node, NodeBase *prev) {
            if (firstNode == nullptr) {
                assert(prev == nullptr);
                firstNode = node;
                lastNode = node;
            } else {
                if (prev != nullptr) { // insert it after prev
                    node->nextNodeInLine = prev->nextNodeInLine;
                    prev->nextNodeInLine = node;
                    if (prev == lastNode) {
                        lastNode = (NodeBase *) node;
                    }
                } else { // insert into top
                    (node)->nextNodeInLine = firstNode;
                    firstNode = node;
                }
            }

            ((NodeBase *) node)->line = this;

            return this;
        }

        CodeLine *appendNode(void *node) {
            if (firstNode == nullptr) {
                firstNode = (NodeBase *) node;
            }

            if (lastNode != nullptr) {
                lastNode->nextNodeInLine = (NodeBase *) node;
            }

            lastNode = (NodeBase *) node;
            ((NodeBase *) node)->line = this;

            return this;
        }

        CodeLine *addPrevLineBreakNode(void *node) {
            CodeLine *currentCodeLine = this;
            currentCodeLine = VTableCall::appendToLine(((NodeBase *) node)->prevLineBreakNode,
                                                       currentCodeLine);


            currentCodeLine = VTableCall::appendToLine(((NodeBase *) node)->prevCommentNode,
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

    /**
     * Defines Managed Coding Operations
     */
    enum class CodingOperations {
        AutoIndentSelection,
        AutoIndentForSpacingRule,
        BreakLine,
        Deletion,
        //AddMapItem
    };

    struct OperationResult {

    };

    struct JsonUtils {
        static void
        put(JsonObjectStruct *json, utf8byte *key, int keyLength, NodeBase *node);
    };


    struct DocumentUtils {

        static OperationResult* performCodingOperation(
            CodingOperations op,
            DocumentStruct* doc,
            NodeBase* startNode,
            NodeBase* endNode
        );

        static void parseText(DocumentStruct *docStruct, const utf8byte *text, int length);
        static JsonObjectStruct *generateHashTables(DocumentStruct *doc);

        static void checkIndentSyntaxErrors(DocumentStruct *doc);
        static void assignIndents(DocumentStruct *doc);
        static void formatIndent(DocumentStruct *doc);

        static utf8byte *getTextFromTree(DocumentStruct *doc);
        static utf8byte *getTypeTextFromTree(DocumentStruct *doc);
        static utf8byte *getTextFromLine(CodeLine *line);
        static utf8byte *getTextFromNode(NodeBase *line);

    };


    struct Init {
        static void initNameNode(NameNodeStruct *name, ParseContext *context, void *parentNode);
        static void initBodyNode(BodyNodeStruct *name, ParseContext *context, void *parentNode);
        static void initStringLiteralNode(StringLiteralNodeStruct *name, ParseContext *context,
                                          NodeBase *parentNode);

        static void
        initSymbolNode(SymbolStruct *self, ParseContext *context, void *parent, utf8byte letter);

        static void initSimpleTextNode(SimpleTextNodeStruct *name, ParseContext *context, void *parentNode, int charLen);
        static void assignText_SimpleTextNode(SimpleTextNodeStruct *name, ParseContext *context, int pos, int charLen);


        static void initAssignStatement(ParseContext *context, NodeBase *parentNode,
                                       AssignStatementNodeStruct *assignStatement
        );

        static void initReturnStatement(ParseContext *context, NodeBase *parentNode,
                                        ReturnStatementNodeStruct *returnStatement
        );

    };


    struct Alloc {

        static TypeNodeStruct *newTypeNode(ParseContext *context, NodeBase *parentNode);

        static NumberNodeStruct *newNumberNode(ParseContext *context, NodeBase *parentNode);
        static VariableNodeStruct *newVariableNode(ParseContext *context, NodeBase *parentNode);

        static BoolNodeStruct *newBoolNode(ParseContext *context, NodeBase *parentNode);
        static LineBreakNodeStruct *newLineBreakNode(ParseContext *context, NodeBase *parentNode);
        static SimpleTextNodeStruct *newSimpleTextNode(ParseContext *context, NodeBase *parentNode);
        static SpaceNodeStruct *newSpaceNode(ParseContext *context, NodeBase *parentNode);
        static NullNodeStruct *newNullNode(ParseContext *context, NodeBase *parentNode);

        // comment
        static LineCommentNodeStruct *newLineCommentNode(ParseContext *context, NodeBase *parentNode);
        static BlockCommentNodeStruct *newBlockCommentNode(ParseContext *context, NodeBase *parentNode);
        static BlockCommentFragmentStruct *newBlockCommentFragmentNode(ParseContext *context, NodeBase *parentNode);




        static ClassNodeStruct *newClassNode(ParseContext *context, NodeBase *parentNode);

        // statement
        static AssignStatementNodeStruct *newAssignStatement(ParseContext *context, NodeBase *parentNode);
        static ReturnStatementNodeStruct *newReturnStatement(ParseContext *context, NodeBase *parentNode);

        static FuncNodeStruct *newFuncNode(ParseContext *context, NodeBase *parentNode);

        static JsonObjectStruct *newJsonObject(ParseContext *context, NodeBase *parentNode);
        static JsonObjectKeyNodeStruct *newJsonObjectKeyNode(ParseContext *context, NodeBase *parentNode);
        static JsonKeyValueItemStruct *newJsonKeyValueItemNode(ParseContext *context, NodeBase *parentNode);
        static JsonArrayStruct *newJsonArray(ParseContext *context, NodeBase *parentNode);
        static JsonArrayItemStruct *newJsonArrayItem(ParseContext *context, NodeBase *parentNode);

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
        NodeBase *parent, utf8byte ch, st_int start, ParseContext *context

    #define TokenizerParams_pass parent, ch, start, context

    using TokenizerFunction = int (*)(TokenizerParams_parent_ch_start_context);

    struct Tokenizers {
        static int nameTokenizer(TokenizerParams_parent_ch_start_context);
        static int variableTokenizer(TokenizerParams_parent_ch_start_context);

        static int typeTokenizer(TokenizerParams_parent_ch_start_context);
        static int numberTokenizer(TokenizerParams_parent_ch_start_context);
        static int nullTokenizer(TokenizerParams_parent_ch_start_context);
        static int stringLiteralTokenizer(TokenizerParams_parent_ch_start_context);
        static int boolTokenizer(TokenizerParams_parent_ch_start_context);

        static int classTokenizer(TokenizerParams_parent_ch_start_context);
        static int bodyTokenizer(TokenizerParams_parent_ch_start_context);
        static int fnTokenizer(TokenizerParams_parent_ch_start_context);

        static int jsonObjectTokenizer(TokenizerParams_parent_ch_start_context);
        static int jsonArrayTokenizer(TokenizerParams_parent_ch_start_context);
        static int jsonObjectNameTokenizer(TokenizerParams_parent_ch_start_context);
        static int jsonValueTokenizer2(TokenizerParams_parent_ch_start_context);

        static int valueTokenizer(TokenizerParams_parent_ch_start_context);


        static int assignStatementTokenizer(TokenizerParams_parent_ch_start_context);
        static int assignStatementWithoutLetTokenizer(TokenizerParams_parent_ch_start_context);
        static int returnStatementTokenizer(TokenizerParams_parent_ch_start_context);

        // SimpleTextNodeStruct
        template<typename TYPE, std::size_t SIZE>
        static inline int WordTokenizer(TokenizerParams_parent_ch_start_context
                      , utf8byte capitalLetter
                      , const TYPE(&word)[SIZE])
        {
            if (capitalLetter == ch) {
                int length = st_size_of(word) - 1;
                if (ParseUtil::matchAt(context->chars, context->length, start, word)
                    ) {
                        auto *boolNode = Alloc::newSimpleTextNode(context, parent);
                        Init::initSimpleTextNode(boolNode, context, parent, 3);

                        boolNode->text = context->memBuffer.newMem<char>(length + 1);
                        boolNode->textLength = length;

                        TEXT_MEMCPY(boolNode->text, context->chars + start, length);
                        boolNode->text[length] = '\0';

                        context->codeNode = Cast::upcast(boolNode);
                        return start + length;
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
                bool root, bool multi
        );

        static int scanOnce(
                void *parentNode,
                TokenizerFunction tokenizer,
                int start,
                ParseContext *context
        );

        static int scanMulti(
                void *parentNode,
                TokenizerFunction tokenizer,
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

