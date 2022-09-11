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

    enum class PrimitiveCalcRegisterEnum {
        eax, ebx, ecx, edx
    };


    #define __RX(reg) (reg)->rax
    #define __EX(reg) (reg)->eax.eax
    #define __X(reg) (reg)->eax.ax.ax
    #define __H(reg) (reg)->eax.ax.ahal.ah
    #define __L(reg) (reg)->eax.ax.ahal.al

    #define __RAX(reg) (reg).rax
    #define __EAX(reg) (reg).eax.eax
    #define __AX(reg) (reg).eax.ax.ax
    #define __AH(reg) (reg).eax.ax.ahal.ah
    #define __AL(reg) (reg).eax.ax.ahal.al

    #define RAX(reg) __RAX((reg)->rax)
    #define EAX(reg) __EAX((reg)->rax)
    #define AX(reg) __AX((reg)->rax)
    #define AH(reg) __AH((reg)->rax)
    #define AL(reg) __AL((reg)->rax)

    #define RBX(reg) __RAX((reg)->rbx)
    #define EBX(reg) __EAX((reg)->rbx)
    #define BX(reg) __AX((reg)->rbx)
    #define BH(reg) __AH((reg)->rbx)
    #define BL(reg) __AL((reg)->rbx)

    #define RCX(reg) __RAX((reg)->rcx)
    #define ECX(reg) __EAX((reg)->rcx)
    #define CX(reg) __AX((reg)->rcx)
    #define CH(reg) __AH((reg)->rcx)
    #define CL(reg) __AL((reg)->rcx)

    #define RDX(reg) __RAX((reg)->rdx)
    #define EDX(reg) __EAX((reg)->rdx)
    #define DX(reg) __AX((reg)->rdx)
    #define DH(reg) __AH((reg)->rdx)
    #define DL(reg) __AL((reg)->rdx)


    union CalcRegister {
        uint64_t rax;
        union {
            uint32_t eax;
            union {
                uint16_t ax;
                struct {
                    uint8_t al;
                    uint8_t ah;
                } ahal;
            } ax;
        } eax;
    };

    /*
          64bit RAX, RBX, RCX, RDX, RSI, RDI, RSP, RBP, R8~R15
          32bit EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP, R8D~R15D
          16bit AX, BX, CX, DX, SI, DI, SP, BP, R8W~R15W
    upper 8bit 	AH, BH, CH, DH,
    lower 8bit 	AL, BL, CL, DL, SIL, DIL, SPL, BPL, R8L~R15L
    */


    using CPURegister = struct _CPURegister {
        CalcRegister rax;
        CalcRegister rbx;
        CalcRegister rcx;
        CalcRegister rdx;

        void add(int num) {
            auto *reg = this;
            RAX(reg) = RAX(reg) + num;
        }
    };

    /*
    static void setActualCaclRegister() {

    }
*/
    #define NODE_HEADER \
        const struct node_vtable *vtable; \
        _NodeBase *parentNode; \
        _NodeBase *nextNode; \
        _NodeBase *nextNodeInLine; \
        CodeLine *codeLine; \
        int indentType; \
        void *prevCommentNode; \
        struct _LineBreakNodeStruct *prevLineBreakNode; \
        ParseContext *context; \
        PrimitiveCalcRegisterEnum calcRegEnum; \
        st_byte *calcReg;                 \
        int typeIndex2;                \
        int found; \
        int prev_chars


    #define INIT_NODE(node, context, parent, argvtable) \
        (node)->vtable = (argvtable); \
        (node)->prev_chars = 0; \
        (node)->context = (context); \
        (node)->parentNode = (NodeBase*)(parent); \
        (node)->codeLine = nullptr; \
        (node)->found = -1; \
        (node)->typeIndex2 = -1; \
        (node)->nextNode = nullptr; \
        (node)->nextNodeInLine = nullptr; \
        (node)->prevLineBreakNode = nullptr; \
        (node)->calcRegEnum = PrimitiveCalcRegisterEnum::eax; \
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

        int stackOffset;
//        int typeIndex;

        char *name;
        int_fast32_t nameLength;
    };

    using VariableNodeStruct = NameNodeStruct;

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


    // $let
    // ?string a
    // alt
    using TypeNodeStruct = struct _TypeNodeStruct {
        NODE_HEADER;

        bool hasMutMark; // $
        bool hasNullableMark; // ?
        int stackSize; // $
        bool isLet; // or has type

        NameNodeStruct nameNode;
        _TypeNodeStruct *typeNode; // generics
    };

    using AssignStatementNodeStruct = struct {
        NODE_HEADER;

        // $int a = 5
        // ?let *ptr = "jfwio"

        bool onlyAssign; // has not declare
        TypeNodeStruct typeOrLet; // $let, int, ?string, etc..
        bool hasTypeDecl; // $let, int, ?string, etc..
        SymbolStruct pointerAsterisk; // *

        int stackOffset;
        NameNodeStruct nameNode; // varName
        SymbolStruct equalSymbol; // =
        NodeBase *valueNode; // 32
        //int typeIndex;
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



    /* ($int point) */
    using FuncParameterItemStruct = struct {
        NODE_HEADER;
        //TypeNodeStruct typeNode;
        //NameNodeStruct nameNode;
        AssignStatementNodeStruct *assignStatementNodeStruct;
        SymbolStruct follwingComma;
        bool hasComma;
    };

    using FuncNodeStruct = struct {
        NODE_HEADER;

        NameNodeStruct nameNode;
        int stackSize;

        SymbolStruct parameterStartNode;
        SymbolStruct parameterEndNode;

        BodyNodeStruct bodyNode;

        int parameterParsePhase;
        FuncParameterItemStruct *firstChildParameterNode;
        FuncParameterItemStruct *lastChildParameterNode;
        int parameterChildCount;
    };


    using ParenthesesNodeStruct = struct {
        NODE_HEADER;

        SymbolStruct openNode;
        SymbolStruct closeNode;

        NodeBase *valueNode;
    };


    using BinaryOperationNodeStruct = struct {
        NODE_HEADER;

        NodeBase *leftExprNode;
        SymbolStruct opNode;
        NodeBase *rightExprNode;
    };




    using FuncArgumentItemStruct = struct {
        NODE_HEADER;

        NodeBase *exprNode; // expression Node
        SymbolStruct follwingComma;
        bool hasComma;
    };


    // value(param, param)
    using CallFuncNodeStruct = struct {
        NODE_HEADER;

        NodeBase *exprNode;
        int parsePhase;
        SymbolStruct openNode;
        SymbolStruct closeNode2;

        FuncArgumentItemStruct *firstArgumentItem;
        FuncArgumentItemStruct *lastArgumentItem;
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

    // --------- Json Object --------- //
    using JsonObjectStruct = struct {
        NODE_HEADER;

        int parsePhase;

        utf8byte body[2]; // '{'
        SymbolStruct endBodyNode;
        JsonKeyValueItemStruct *firstKeyValueItem;
        JsonKeyValueItemStruct *lastKeyValueItem;
        VoidHashMap *hashMap;
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


    enum class TokenTypeIds {
        namespaceId = 0,
        typeId = 1,
        classId = 2,
        enumId = 3,
        interfaceId = 4,
        structId = 5,
        typeParameterId = 6,
        parameterId = 7,
        variableId = 8,
        propertyId = 9,
        enumMemberId = 10,
        eventId = 11,
        functionId = 12,
        methodId = 13,
        macroId = 14,
        keywordId = 15,
        modifierId = 16,
        commentId = 17,
        stringId = 18,
        numberId = 19,
        regexp = 20,
        operatorId = 21,
        decoratorId = 22,
        myclass = 23,
    };

    static const char* tokenTypes[] = {
            "namespace",
            "type",
            "class",
            "enum",
            "interface",
            "struct",
            "typeParameter",
            "parameter",
            "variable",
            "property",
            "enumMember",
            "event",
            "function",
            "method",
            "macro",
            "keyword",
            "modifier",
            "comment",
            "string",
            "number",
            "regexp",
            "operator",
            "decorator",
            "myclass",
            nullptr
    };
    static const char* tokenModifiers[] = {"declaration", "documentation", nullptr};

    enum class AppendLineMode {
        Normal,
        DetectErrorSpanNodes
    };
    struct ParseContext {
        st_uint start;
        int length;
        bool scanEnd;
        AppendLineMode appendLineMode; // 0:
        int prevFoundPos;


        bool afterLineBreak;
        //NodeBase *codeNode;
        NodeBase *leftNode;
        NodeBase *valueNode;
        NodeBase *virtualCodeNode;
        int baseIndent;
        utf8byte *chars;
        SyntaxErrorInfo syntaxErrorInfo;
        bool has_cancel_request{false};
        bool has_depth_error{false};
        int parentDepth { -1 };
        int arithmeticBaseDepth{ -1 };

        // node caches
        AssignStatementNodeStruct *unusedAssignment;
        ClassNodeStruct *unusedClassNode;

        void (*actionCreator)(void *node1, void *node2, int actionRequest);


        LineBreakNodeStruct *remainedLineBreakNode;
        void *remainedCommentNode;
        int remaindPrevChars{0};

        MemBuffer memBuffer;
        MemBuffer memBufferForCodeLines;

        void setCodeNode(void* node) {
            this->leftNode = static_cast<NodeBase *>(node);
            this->virtualCodeNode = static_cast<NodeBase *>(node);
        }

        void init() {
            memBuffer.init();
            memBufferForCodeLines.init();
            appendLineMode = AppendLineMode::Normal;
        }

        void dispose() {
            memBuffer.freeAll();
            memBufferForCodeLines.freeAll();
        }

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
            return memBufferForCodeLines.newMem<CodeLine>(1);
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

            errorInfo.errorItem.charPosition = startPos;


            this->setError3(errorCode, a, b, a+1, 0);
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


            errorInfo.errorItem.charPosition = startPos;
            errorInfo.errorItem.charPosition2 = startPos2;

            this->setError3(errorCode, a, b, c, d);
        }


        void setIndentError(ErrorCode errorCode, st_int line1, st_int charPos1) {
            auto &errorInfo = this->syntaxErrorInfo;
            if (errorInfo.hasError) {
                return;
            }
            this->setError3(errorCode, line1, 0, line1, charPos1);
        }

        void setError3(ErrorCode errorCode, st_int line1, st_int charPos1, st_int line2, st_int charPos2) {
            auto &errorInfo = this->syntaxErrorInfo;
            if (errorInfo.hasError) {
                return;
            }

            errorInfo.errorItem.linePos1 = line1;
            errorInfo.errorItem.linePos2 = line2;
            errorInfo.errorItem.charPos1 = charPos1;
            errorInfo.errorItem.charPos2 = charPos2;


            errorInfo.hasError = true;
            errorInfo.errorItem.errorCode = errorCode;

            errorInfo.errorItem.errorId = getErrorId(errorCode);
            const char* reason = getErrorMessage(errorCode);
            if (reason == nullptr) {
                reason = "";
            }
            int len = (int)strlen(reason);
            errorInfo.errorItem.reasonLength = len < MAX_REASON_LENGTH ? len : MAX_REASON_LENGTH;
            memcpy(errorInfo.errorItem.reason, reason, errorInfo.errorItem.reasonLength);
            errorInfo.errorItem.reason[errorInfo.errorItem.reasonLength] = '\0';
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
        static inline T downcast(void *node) {
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
        Parentheses = 26,
        CallFunc = 27,
        FuncArgument = 28,
        FuncParameter = 29,

        BinaryOperation = 30
    };



    #define ApplyFunc_params3 \
        void *scriptEngineContext, \
        void *targetVTable, \
        int (*func)(NodeBase *, void *scriptEngineContext, void *targetVTable,void *func, bool parentIsFirst, void *arg, void *arg2), \
        bool parentIsFirst,           \
        void *arg,           \
        void *arg2

    #define ApplyFunc_params2 \
        void *scriptEngineContext, \
        void *targetVTable, \
        void *func, \
        bool parentIsFirst,           \
        void *arg,           \
        void *arg2


    #define ApplyFunc_pass \
                scriptEngineContext, \
                targetVTable, (void *)func, parentIsFirst, arg, arg2

    #define ApplyFunc_pass2 \
                scriptEngineContext, \
                targetVTable, func, parentIsFirst, arg, arg2


    #define VTABLE_DEF(T) \
        int (*selfTextLength)(T *self); \
        const utf8byte *(*selfText)(T *self); \
        CodeLine *(*appendToLine)(T *self, CodeLine *line); \
        int (*applyFuncToDescendants)(T *Node, ApplyFunc_params3); \
        const char *typeChars; \
        int typeCharsLength; \
        int (*typeSelector)(void *env, NodeBase *self);     \
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
    using applyFuncToDescendantsFunction = decltype(std::declval<NodeVTable>().applyFuncToDescendants);

    template<typename T, std::size_t SIZE>
    static int vtable_type_check(
            decltype(std::declval<vtableT<T>>().selfTextLength) f1,
            decltype(std::declval<vtableT<T>>().selfText) f2,
            decltype(std::declval<vtableT<T>>().appendToLine) f3,
            decltype(std::declval<vtableT<T>>().applyFuncToDescendants) f4,
            const char(&f5)[SIZE],
            NodeTypeId f6
    ) noexcept {
        return 0;
    }
    #define CREATE_VTABLE(T, f1, f2, f3, f4, f5, f6) \
        node_vtable { \
            reinterpret_cast<selfTextLengthFunction> (f1) \
            , reinterpret_cast<selfTextFunction> (f2) \
            , reinterpret_cast<appendToLineFunction> (f3) \
            , reinterpret_cast<applyFuncToDescendantsFunction> (f4) \
            , (const char *)(f5) \
            , (sizeof(f5)-1) \
            , nullptr \
            , f6 \
        } \
        ;static const int check_result_##T = vtable_type_check<T>(f1,f2,f3,f4,f5,f6)
    // static_assert(std::is_same<F2, decltype(std::declval<vtableT<T>>().selfText)>::value, "");

    struct VTables {
        static const node_vtable
                *DocumentVTable,

                *AssignStatementVTable,
                *ReturnStatementVTable,

                *ClassVTable,

                *FnVTable,

                *NameVTable,
                *VariableVTable,
                *BodyVTable,
                *TypeVTable,
                *StringLiteralVTable,
                *NumberVTable,
                *BoolVTable,
                *SymbolVTable,
                *SimpleTextVTable,
                *NullVTable,
                *SpaceVTable,
                *LineBreakVTable,
                *ParenthesesVTable,
                *CallFuncVTable,
                *FuncArgumentVTable,
                *FuncParameterVTable,

                // operation
                *BinaryOperationVTable,

                *LineCommentVTable,
                *BlockCommentVTable,
                *BlockCommentFragmentVTable,


                *JsonObjectVTable,
                *JsonArrayVTable,
                *JsonKeyValueItemVTable,
                *JsonArrayItemVTable,
                *JsonObjectKeyVTable,

                *EndOfFileVTable;
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


        static CodeLine *callAppendToLine(void *node, CodeLine *currentCodeLine);

    };


    struct CodeLine {
        CodeLine *nextLine;
        int lineNumber;
        //CodeLine *prev;
        NodeBase *firstNode;
        NodeBase *lastNode;
        // int indent;
        ParseContext *context;

        int depth;

        void init(ParseContext *argContext) {
            this->context = argContext;
            this->lineNumber = 0;

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

            ((NodeBase *) node)->codeLine = this;

            return this;
        }

        CodeLine *appendNode(void *node) {
            if (firstNode == nullptr) {
                firstNode = (NodeBase *) node;
            }

            if (lastNode != nullptr) {
                if (this->context->appendLineMode == AppendLineMode::Normal) {
                    lastNode->nextNodeInLine = (NodeBase *) node;
                }
            }

            lastNode = (NodeBase *) node;

            if (this->context->appendLineMode == AppendLineMode::Normal) {
                ((NodeBase *) node)->codeLine = this;
            }

            return this;
        }

        CodeLine *addPrevLineBreakNode(void *node) {

            CodeLine *currentCodeLine = this;

            if (this->context->appendLineMode == AppendLineMode::Normal) {
                currentCodeLine = VTableCall::callAppendToLine(
                        ((NodeBase *) node)->prevLineBreakNode,
                        currentCodeLine);
            }

            currentCodeLine = VTableCall::callAppendToLine(((NodeBase *) node)->prevCommentNode,
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

        static void regenerateCodeLines(DocumentStruct *docStruct);
        static void checkIndentSyntaxErrors(DocumentStruct *doc);
        static void assignIndents(DocumentStruct *doc);
        static void calcStackSize(DocumentStruct *doc);
        static void formatIndent(DocumentStruct *doc);

        static utf8byte *getTextFromTree(DocumentStruct *doc);
        static utf8byte *getSemanticTokensTextFromTree(DocumentStruct *doc, int *len, int line0, int line1);
        static utf8byte *getTypeTextFromTree(DocumentStruct *doc);
        static utf8byte *getTextFromLine(CodeLine *line);
        static utf8byte *getTextFromNode(NodeBase *line);

    };


    struct Init {
        static void initNameNode(NameNodeStruct *name, ParseContext *context, void *parentNode);
        static void initBodyNode(BodyNodeStruct *name, ParseContext *context, void *parentNode);
        static void initStringLiteralNode(StringLiteralNodeStruct *name, ParseContext *context,
                                          NodeBase *parentNode);

        static void initSymbolNode(SymbolStruct *self, ParseContext *context, void *parent, utf8byte letter);
        static void initTypeNode(TypeNodeStruct *self, ParseContext *context, void *parent);

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
        static ParenthesesNodeStruct *newParenthesesNode(ParseContext *context, NodeBase *parentNode);
        static CallFuncNodeStruct *newFuncCallNode(ParseContext *context, NodeBase *parentNode);

        static BoolNodeStruct *newBoolNode(ParseContext *context, NodeBase *parentNode);
        static LineBreakNodeStruct *newLineBreakNode(ParseContext *context, NodeBase *parentNode);
        static SimpleTextNodeStruct *newSimpleTextNode(ParseContext *context, NodeBase *parentNode);
        static NullNodeStruct *newNullNode(ParseContext *context, NodeBase *parentNode);

        // comment
        static LineCommentNodeStruct *newLineCommentNode(ParseContext *context, NodeBase *parentNode);
        static BlockCommentNodeStruct *newBlockCommentNode(ParseContext *context, NodeBase *parentNode);
        static BlockCommentFragmentStruct *newBlockCommentFragmentNode(ParseContext *context, NodeBase *parentNode);

        // operations
        static BinaryOperationNodeStruct *newBinaryOperationNode(ParseContext *context, NodeBase *parentNode, char op);




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
        static FuncParameterItemStruct *newFuncParameterItem(ParseContext *context, NodeBase *parentNode);
        static FuncArgumentItemStruct *newFuncArgumentItem(ParseContext *context, NodeBase *parentNode);

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
        static int nameTokenizer_ignore(TokenizerParams_parent_ch_start_context, int ignorePos);
        static int variableTokenizer(TokenizerParams_parent_ch_start_context);
        static int expressionTokenizer(TokenizerParams_parent_ch_start_context);
        static int parenthesesTokenizer(TokenizerParams_parent_ch_start_context);
        static int funcCallTokenizer(TokenizerParams_parent_ch_start_context);
        static int binaryOperationTokenizer(TokenizerParams_parent_ch_start_context);


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



        static int assignStatementTokenizer(TokenizerParams_parent_ch_start_context);
        static int assignStatementWithoutLetTokenizer(TokenizerParams_parent_ch_start_context);
        static int returnStatementTokenizer(TokenizerParams_parent_ch_start_context);

        // SimpleTextNodeStruct
        template<typename TYPE, std::size_t SIZE, typename GENTYPE>
        static inline int WordTokenizer2(TokenizerParams_parent_ch_start_context
                      , GENTYPE* (*genereater)(ParseContext *, NodeBase*)
                      , utf8byte capitalLetter
                      , const TYPE(&word)[SIZE])
        {
            if (capitalLetter == ch) {
                int length = st_size_of(word) - 1;
                if (ParseUtil::matchAt(context->chars, context->length, start, word) > -1 ) {
                    auto *boolNode = (SimpleTextNodeStruct*)(genereater(context, parent));
                    //Init::initSimpleTextNode(boolNode, context, parent, 3);

                    boolNode->text = context->memBuffer.newMem<char>(length + 1);
                    boolNode->textLength = length;

                    TEXT_MEMCPY(boolNode->text, context->chars + start, length);
                    boolNode->text[length] = '\0';

                    context->leftNode = Cast::upcast(boolNode);
                    context->virtualCodeNode = Cast::upcast(boolNode);
                    return start + length;
                }
            }

            return -1;
        }

        // SimpleTextNodeStruct
        template<typename TYPE, std::size_t SIZE>
        static inline int WordTokenizer(TokenizerParams_parent_ch_start_context
                , utf8byte capitalLetter
                , const TYPE(&word)[SIZE])
        {
            return WordTokenizer2(TokenizerParams_pass, Alloc::newSimpleTextNode, capitalLetter, word);
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
        static void *
        generateBlockCommentFragments(void *parentNode, ParseContext *context, const int32_t &i,
                                      int commendEndIndex);
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

