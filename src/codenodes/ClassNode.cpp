#include <cstdio>
#include <iostream>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <vector>

#include <cstdint>
#include <ctime>
#include <cstring>

#include "code_nodes.hpp"


namespace smart {

    using InnerNodeStruct = struct {
        NODE_HEADER;
    };

    struct ClassNodeStruct::Impl {
        int a;
    };

    // --------------------- Defines Class VTable ---------------------- /

    static int selfTextLength(ClassNodeStruct *) {
        return 5;
    }

    static const utf8byte *selfText(ClassNodeStruct *) {
        return "class";
    }


    static CodeLine *appendToLine(ClassNodeStruct *classNode, CodeLine *currentCodeLine) {

        currentCodeLine = currentCodeLine->addPrevLineBreakNode(classNode);
        currentCodeLine->appendNode(classNode);


        auto formerParentDepth = classNode->context->parentDepth;
        classNode->context->parentDepth += 1;
        currentCodeLine = VTableCall::appendToLine(&classNode->nameNode, currentCodeLine);
        classNode->context->parentDepth = formerParentDepth;

        currentCodeLine = VTableCall::appendToLine(&classNode->bodyStartNode, currentCodeLine);

        formerParentDepth = classNode->context->parentDepth;
        classNode->context->parentDepth += 1;

        {
            auto *child = classNode->firstChildNode;
            while (child) {
                currentCodeLine = VTableCall::appendToLine(child, currentCodeLine);
                child = child->nextNode;
            }
        }



        auto* prevCodeLine = currentCodeLine;
        currentCodeLine = VTableCall::appendToLine(&classNode->endBodyNode, currentCodeLine);

        if (prevCodeLine != currentCodeLine) {
            currentCodeLine->depth = formerParentDepth+1;
        }

        classNode->context->parentDepth = formerParentDepth;


        return currentCodeLine;
    };


    static constexpr const char classTypeText[] = "<Class>";

    static int applyFuncToDescendants(ClassNodeStruct *Node, void *targetVTable, int (*applyFuncToDescendants)(NodeBase *Node, void *targetVTable, void *func, void *arg, void *arg2), void *arg, void *arg2) {


        return 0;
    }

    /*
     * class A {
     *
     *
     * }
     */
    static node_vtable _classVTable = CREATE_VTABLE(ClassNodeStruct,
                                                          selfTextLength,
                                                          selfText,
                                                          appendToLine,
                                                          applyFuncToDescendants,
                                                          classTypeText
                                                          , NodeTypeId::Class);

    const struct node_vtable *VTables::ClassVTable = &_classVTable;


    // -------------------- Implements ClassNode Allocator --------------------- //
    ClassNodeStruct *Alloc::newClassNode(ParseContext *context, NodeBase *parentNode) {
        auto *classNode = context->newMem<ClassNodeStruct>();
        //classNode->sub = simpleMalloc<ClassNodeStruct::Impl>();

        INIT_NODE(classNode, context, parentNode, &_classVTable);
        classNode->lastChildNode = nullptr;
        classNode->firstChildNode = nullptr;

        Init::initNameNode(&classNode->nameNode, context, classNode);

        classNode->startFound = false;

        Init::initSymbolNode(&classNode->bodyStartNode, context, classNode, '{');
        Init::initSymbolNode(&classNode->endBodyNode, context, classNode, '}');

        return classNode;
    }


    // --------------------- Implements ClassNode Parser ----------------------

    static void appendChildNode(ClassNodeStruct *classNode, NodeBase *node) {
        if (classNode->firstChildNode == nullptr) {
            classNode->firstChildNode = node;
        }
        if (classNode->lastChildNode != nullptr) {
            classNode->lastChildNode->nextNode = node;
        }
        classNode->lastChildNode = node;
        classNode->childCount++;
    }

    static int inner_classBodyTokenizer(TokenizerParams_parent_ch_start_context) {
        auto *classNode = Cast::downcast<ClassNodeStruct *>(parent);

        //console_log(std::string(""+ch).c_str());
        //console_log((std::string{"==,"} + std::string{ch} + std::to_string(ch)).c_str());

        if (!classNode->startFound) {
            if (ch == '{') {
                classNode->startFound = true;
                context->setCodeNode(&classNode->bodyStartNode);
                return start + 1;
            }
            else {
                context->setError(ErrorCode::no_brace_for_class, classNode->found);
            }
        }
        else if (ch == '}') {
            context->scanEnd = true;
            context->setCodeNode(&classNode->endBodyNode);
            return start + 1;
        }
        else {
            int result;
            if (-1 < (result = Tokenizers::classTokenizer(parent, ch, start, context))) {
                auto *innerClassNode = Cast::downcast<ClassNodeStruct *>(parent);
                appendChildNode(innerClassNode, context->virtualCodeNode);
                return result;
            }

            if (-1 < (result = Tokenizers::fnTokenizer(parent, ch, start, context))) {
                auto* innerClassNode = Cast::downcast<ClassNodeStruct*>(parent);
                appendChildNode(innerClassNode, context->virtualCodeNode);
                return result;
            }

            context->scanEnd = true;
            context->setError2(ErrorCode::no_brace_of_end_for_class, classNode->found, start);
        }

        return -1;
    }




    int Tokenizers::classTokenizer(TokenizerParams_parent_ch_start_context) {
        static constexpr const char class_chars[] = "class";
        static constexpr int size_of_class = sizeof(class_chars) - 1;

        if ('c' == ch) {
            auto idx = ParseUtil::matchAt(context->chars, context->length, start, class_chars);
            if (idx > -1) {
                int currentPos = idx + size_of_class;
                int resultPos;

                // "class " came here
                auto *classNode = Alloc::newClassNode(context, parent);
                classNode->found = start;

                {
                    resultPos = Scanner::scanOnce(&classNode->nameNode,
                                              Tokenizers::nameTokenizer,
                                              currentPos,
                                              context);

                    if (resultPos == -1) {
                        // the class should have a class name
                        //console_log(std::string(classNode->nameNode.name).c_str());
                        context->setError(ErrorCode::invalid_class_name, start);
                        return -1;
                    }
                }


                // Parse body
                currentPos = resultPos;
                if (-1 == (resultPos = Scanner::scanMulti(classNode, inner_classBodyTokenizer,
                                                     currentPos, context))) {
                    //context->codeNode = Cast::upcast(classNode);
                    //return currentPos;
                    return -1;
                }

                context->setCodeNode(classNode);
                return resultPos;
            }
        }
        return -1;
    }
}



