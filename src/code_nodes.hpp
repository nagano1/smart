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
        struct _SimpleTextNodeStruct *prevSpaceNode; \
        struct _LineBreakNodeStruct *prevLineBreakNode; \
        ParseContext *context; \
        char prev_char


    #define INIT_NODE(node, context, parent, argvtable) \
        (node)->vtable = (argvtable); \
        (node)->prev_char = '\0'; \
        (node)->context = (context); \
        (node)->parentNode = (NodeBase*)(parent); \
        (node)->line = nullptr; \
        (node)->nextNode = nullptr; \
        (node)->nextNodeInLine = nullptr; \
        (node)->prevSpaceNode = nullptr; \
        (node)->prevLineBreakNode = nullptr; \
        32\

    #define TEXT_MEMCPY(dst, src, len) \
        memcpy((dst), (src), (len))

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
        st_textlen nameLength;
    };



    using StringLiteralNodeStruct = struct {
        NODE_HEADER;
        char *text;
        st_textlen textLength;

        char *str;
        st_textlen strLength;

        int literalType; // 0: "text", 1: `wjfeiofw`, 2: r"testfaojiwe"
    };

    using BoolNodeStruct = struct {
        NODE_HEADER;

        char *text;
        st_textlen textLength;
        bool boolValue;
    };


    using NumberNodeStruct = struct {
        NODE_HEADER;

        char *text;
        st_textlen textLength;
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
        st_textlen textLength;

        int namePos;
        st_textlen nameLength;
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
        st_textlen keyLength;
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
        void put(const char *keyA, st_textlen keyLength, NodeBase *val);
        NodeBase *get(const char *key, st_textlen keyLength);
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

    
    enum class ErrorCode {
        first_keeper,

        missing_closing_quote,
        missing_closing_quote2,
        missing_object_delemeter,

        last_keeper
    };

    static constexpr int errorListSize = 1 + static_cast<int>(ErrorCode::last_keeper);
    
    struct ErrorInfo {
        ErrorCode errorIndex;
        int errorCode;
        const char* msg;
    };

    extern ErrorInfo ErrorInfoList[errorListSize];
    extern bool errorInfoInitialized;
    static ErrorInfo sortErrorInfoList[errorListSize];




    static int acompare(void const * alhs, void const * arhs) {
        ErrorInfo* lhs = (ErrorInfo*)alhs;
        ErrorInfo* rhs = (ErrorInfo*)arhs;

        if (lhs->errorCode == rhs->errorCode) {
            // printf("duplicate error id(%d)\n ", lhs->errorCode);
            return 0;
            // throw 3;
        }
        else if (lhs->errorCode > rhs->errorCode) {
            return 1;
        }
        else {
            return -1;
        }

        //return lhs->errorCode - rhs->errorCode;
        return 0;
    }



    static int checkSum() {
        errorInfoInitialized = true;


        static constexpr ErrorInfo tempList[] = {
    ErrorInfo{ ErrorCode::first_keeper, 9912, "start"},

    ErrorInfo{ ErrorCode::missing_closing_quote, 989800, "missing closing quote" },
    ErrorInfo{ ErrorCode::missing_closing_quote2, 989900, "missing closing quote" },

    ErrorInfo{ ErrorCode::missing_object_delemeter, 77812, "missing object delimeter"},


    ErrorInfo{ ErrorCode::last_keeper, 9999999, "end" },
        };


        constexpr int len = (sizeof tempList) / (sizeof tempList[0]);
        for (int i = 0; i < len; i++) {
            auto &&errorInfo = tempList[i];
            if (static_cast<int>(errorInfo.errorIndex) != i) {
                printf("error info index\n");
            }

            ErrorInfoList[static_cast<int>(tempList[i].errorIndex)] = errorInfo;
            //sortErrorInfoList[static_cast<int>(tempList[i].errorIndex)] = errorInfo;
        }

        // check duplicate of error code
        //std::sort(sortErrorInfoList, sortErrorInfoList + errorListSize, acompare);
        qsort(sortErrorInfoList, sizeof(sortErrorInfoList) / sizeof(sortErrorInfoList[0]), sizeof(ErrorInfo), acompare);

        return 0;
    }

    /*
    // C++-14
    bool is_sorted() {
        for (std::size_t i = 0; i < (sizeof tempList) / (sizeof tempList[0]) - 1; ++i) {
            if (tempList[i].errorCode >= tempList[i + 1].errorCode) {
                return false;
            }
        }
        return true;
    }
    static_assert(is_sorted(), "error list should have the same length");
    */

//    static_assert(errorListSize == (sizeof tempList) / (sizeof(ErrorInfo)), "error list should have the same length");



    enum class Language {
        en = 8591000,
        jp = 8591001,
    };


    static const char *translateErrorMessage(ErrorCode errorCode, Language lang) {
        return nullptr;
    }


    static const char *getErrorMessage(ErrorCode errorCode) {
        if (errorInfoInitialized == false) {
            checkSum();
            errorInfoInitialized = true;
        }
        const char *mes = nullptr;
        auto&& errorInfo = ErrorInfoList[static_cast<int>(errorCode)];
        mes = errorInfo.msg;

        auto *transMess = translateErrorMessage(errorCode, Language::jp);
        if (transMess != nullptr) {
            mes = transMess;
        }

        return mes;
    }



    #define MAX_REASON_LENGTH 1024
    /**
     * Syntax error is allowed only once
     */
    using SyntaxErrorInfo = struct _errorInfo {
        bool hasError{false};

        ErrorCode errorCode;
        char reason[MAX_REASON_LENGTH + 1];
        st_textlen reasonLength = 0;

        st_uint charPosition;
        int charEndPosition;

        // 0: "between start and  end"
        // 1: "from start to end of line,"
        int errorDisplayType = 0;

        static const int SYNTAX_ERROR_RETURN = -1;

        static void setError(_errorInfo *error, ErrorCode errorCode, st_uint start) {
            error->hasError = true;
            error->errorCode = errorCode;
            error->charPosition = start;

            const char* reason = getErrorMessage(errorCode);
            if (reason == nullptr) {
                reason = "";
            }
            st_textlen len = (st_textlen) strlen(reason);
            error->reasonLength = len < MAX_REASON_LENGTH ? len : MAX_REASON_LENGTH;
            TEXT_MEMCPY(error->reason, reason, error->reasonLength);
            error->reason[error->reasonLength] = '\0';
        }
    };


    struct ParseContext {
        st_uint start;
        st_textlen length;
        bool scanEnd;
        bool afterLineBreak;
        NodeBase *codeNode;
        // int former_start;
        st_uint baseIndent;
        utf8byte *chars;
        SyntaxErrorInfo syntaxErrorInfo;
        bool has_cancel_request{false};
        bool has_depth_error{false};
        st_int parentDepth{ -1 };


        void (*actionCreator)(void *node1, void *node2, int actionRequest);


        LineBreakNodeStruct *remainedLineBreakNode;
        SpaceNodeStruct *remainedSpaceNode;

        MemBuffer memBuffer;
        /*
        NodeBufferList<SimpleTextNodeStruct> spaceBufferList;
        NodeBufferList<CodeLine> codeLineBufferList;
        NodeBufferList<LineBreakNodeStruct> lineBreakBufferList;
        CharBuffer<char> charBuffer;
         */

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
            /*
            return lineBreakBufferList.newNode();
             */
        }

        CodeLine *newCodeLine() {
            return memBuffer.newMem<CodeLine>(1);

            //return codeLineBufferList.newNode();
        }

        SpaceNodeStruct *newSpaceNode() {
            return memBuffer.newMem<SpaceNodeStruct>(1);
            //return spaceBufferList.newNode();
        }

        NullNodeStruct *newNullNode() {
            return memBuffer.newMem<SimpleTextNodeStruct>(1);
            //return spaceBufferList.newNode();
        }
    };


    static inline void deleteContext(ParseContext *context) {
        context->memBuffer.freeAll();
        /*
        deleteNodeBufferList(context->lineBreakBufferList.firstBufferList);
        deleteNodeBufferList(context->codeLineBufferList.firstBufferList);
        deleteNodeBufferList(context->spaceBufferList.firstBufferList);
        deleteCharBuffer(context->charBuffer.firstBufferList);
*/
        free(context);
    }


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

        //Func(3),
        NULLId = 16
    };

    #define VTABLE_DEF(T) \
        st_textlen (*selfTextLength)(T *self); \
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
        CREATE_VTABLE2(T, f1, f2, f3, f4, f5, 0)

    #define CREATE_VTABLE2(T, f1, f2, f3, f4, f5, f6) \
        node_vtable { \
            reinterpret_cast<selfTextLengthFunction> (f1) \
            , reinterpret_cast<selfTextFunction> (f2) \
            , reinterpret_cast<appendToLineFunction> (f3) \
            , (const char *)(f4) \
            , (sizeof(f4)-1) \
            , f5 \
        } \
        ;static const int check_result_##T##f6 = vtable_type_check<T>(f1,f2,f3,f4, f5)
    // static_assert(std::is_same<F2, decltype(std::declval<vtableT<T>>().selfText)>::value, "");

    struct VTables {
        static const node_vtable
                *DocumentVTable,

                *ClassVTable,
                *ClassBodyVTable,

                *NameVTable,
                *StringLiteralVTable,
                *NumberVTable,
                *BoolVTable,
                *SymbolVTable,
                *SimpleTextVTable,
                *NullVTable,
                *SpaceVTable,
                *LineBreakVTable,

                *JsonObjectVTable,
                *JsonArrayVTable,
                *JsonKeyValueItemVTable,
                *JsonArrayItemVTable,
                *JsonObjectKeyVTable,

                *EndOfFileVTable;
    };




    struct VTableCall {

        static st_textlen selfTextLength(NodeBase *node) {
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
            };
        }

        static st_textlen typeTextLength(NodeBase *node) {
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
            };
        }


        static CodeLine *appendToLine(void *node, CodeLine *currentCodeLine);

    };


    struct CodeLine {
        CodeLine *nextLine;
        CodeLine *prev;
        NodeBase *firstNode;
        NodeBase *lastNode;
        st_uint indent;
        st_uint depth;

        void init(ParseContext *context) {
            this->firstNode = nullptr;
            this->lastNode = nullptr;
            this->nextLine = nullptr;
            this->prev = nullptr;
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
            currentCodeLine = VTableCall::appendToLine(((NodeBase *) node)->prevSpaceNode,
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
        put(JsonObjectStruct *json, utf8byte *key, st_textlen keyLength, NodeBase *node);
    };


    struct DocumentUtils {

        static OperationResult* performCodingOperation(
            CodingOperations op,
            DocumentStruct* doc,
            NodeBase* startNode,
            NodeBase* endNode
        );

        static void parseText(DocumentStruct *docStruct, const utf8byte *text, st_textlen length);
        static JsonObjectStruct *generateHashTables(DocumentStruct *doc);


        static void assignIndentsAndDepth(DocumentStruct *doc);
        //static void assignIndentsToBreakLines(DocumentStruct *doc);
        static void formatIndent(DocumentStruct *doc);

        static utf8byte *getTextFromTree(DocumentStruct *doc);
        static utf8byte *getTypeTextFromTree(DocumentStruct *doc);
        static utf8byte *getTextFromLine(CodeLine *line);
        static utf8byte *getTextFromNode(NodeBase *line);


    };


    struct Init {
        static void initNameNode(NameNodeStruct *name, ParseContext *context, NodeBase *parentNode);
        static void initStringLiteralNode(StringLiteralNodeStruct *name, ParseContext *context,
                                          NodeBase *parentNode);

        static void
        initSymbolNode(SymbolStruct *self, ParseContext *context, void *parent, utf8byte letter);
    };

    struct Alloc {
        static NumberNodeStruct *newNumberNode(ParseContext *context, NodeBase *parentNode);
        static BoolNodeStruct *newBoolNode(ParseContext *context, NodeBase *parentNode);
        static LineBreakNodeStruct *newLineBreakNode(ParseContext *context, NodeBase *parentNode);
        static SimpleTextNodeStruct *newSimpleTextNode(ParseContext *context, NodeBase *parentNode);
        static SpaceNodeStruct *newSpaceNode(ParseContext *context, NodeBase *parentNode);
        static NullNodeStruct *newNullNode(ParseContext *context, NodeBase *parentNode);

        static ClassNodeStruct *newClassNode(ParseContext *context, NodeBase *parentNode);

        static void deleteClassNode(NodeBase *node);

        static ClassNodeStruct *newFuncNode(ParseContext *context, NodeBase *parentNode);
        static void deleteFuncNode(NodeBase *node);

        static JsonObjectStruct *newJsonObject(ParseContext *context, NodeBase *parentNode);
        static JsonObjectKeyNodeStruct *
        newJsonObjectKeyNode(ParseContext *context, NodeBase *parentNode);
        static JsonKeyValueItemStruct *
        newJsonKeyValueItemNode(ParseContext *context, NodeBase *parentNode);
        static JsonArrayStruct *newJsonArray(ParseContext *context, NodeBase *parentNode);
        static JsonArrayItemStruct *newJsonArrayItem(ParseContext *context, NodeBase *parentNode);

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
        NodeBase *parent, utf8byte ch, st_uint start, ParseContext *context

    #define TokenizerParams_pass parent, ch, start, context

    using TokenizerFunction = int (*)(TokenizerParams_parent_ch_start_context);

    struct Tokenizers {
        static int nameTokenizer(TokenizerParams_parent_ch_start_context);
        static int numberTokenizer(TokenizerParams_parent_ch_start_context);
        static int nullTokenizer(TokenizerParams_parent_ch_start_context);
        static int stringLiteralTokenizer(TokenizerParams_parent_ch_start_context);
        static int boolTokenizer(TokenizerParams_parent_ch_start_context);

        static int classTokenizer(TokenizerParams_parent_ch_start_context);

        static int jsonObjectTokenizer(TokenizerParams_parent_ch_start_context);
        static int jsonArrayTokenizer(TokenizerParams_parent_ch_start_context);
        static int jsonObjectNameTokenizer(TokenizerParams_parent_ch_start_context);
        static int jsonValueTokenizer(TokenizerParams_parent_ch_start_context);


        // SimpleTextNodeStruct
        template<typename TYPE, std::size_t SIZE>
        static inline int
        WordTokenizer(TokenizerParams_parent_ch_start_context, utf8byte capitalLetter,
                      const TYPE(&word)[SIZE]) {
            if (capitalLetter == ch) {
                st_size length = st_size_of(word) - 1;
                if (ParseUtil::matchWord(context->chars, context->length, word, length, start)) {

                    if (start + length == context->length // allowed to be the last char of the file
                        || ParseUtil::isNonIdentifierChar(
                            context->chars[start + length])) { // otherwise,

                        //context->scanEnd = true;
                        auto *boolNode = Alloc::newSpaceNode(context, parent);

                        boolNode->text = context->memBuffer.newMem<char>(
                                length + 1);// context->charBuffer.newChars(length + 1);
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

