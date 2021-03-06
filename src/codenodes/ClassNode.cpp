﻿#include <cstdio>
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

    static st_textlen selfTextLength(ClassNodeStruct *classNode) {
        return 5;
    }

    static const utf8byte *selfText(ClassNodeStruct *node) {
        return "class";
    }

    static CodeLine *appendToLine(ClassNodeStruct *self, CodeLine *currentCodeLine) {
        auto *classNode = self;//Cast::downcast<ClassNodeStruct *>(self);

        currentCodeLine = currentCodeLine->addPrevLineBreakNode(classNode);

        currentCodeLine->appendNode(classNode);

        currentCodeLine = VTableCall::appendToLine(&classNode->nameNode, currentCodeLine);
        currentCodeLine = VTableCall::appendToLine(&classNode->bodyStartNode, currentCodeLine);

        {
            auto *child = classNode->firstChildNode;
            while (child) {
                currentCodeLine = VTableCall::appendToLine(child, currentCodeLine);
                child = child->nextNode;
            }
        }

        currentCodeLine = VTableCall::appendToLine(&classNode->endBodyNode, currentCodeLine);


        return currentCodeLine;
    };


    static constexpr const char classTypeText[] = "<Class>";

    /*
     * class A {
     *
     *
     * }
     */
    static const node_vtable _ClassVTable = CREATE_VTABLE(ClassNodeStruct,
                                                          selfTextLength,
                                                          selfText,
                                                          appendToLine,
                                                          classTypeText,
                                                          true
    );
    const struct node_vtable *VTables::ClassVTable = &_ClassVTable;


    // -------------------- Implements ClassNode Allocator --------------------- //
    ClassNodeStruct *Alloc::newClassNode(ParseContext *context, NodeBase *parentNode) {
        auto *classNode = context->newMem<ClassNodeStruct>();
        //classNode->sub = simpleMalloc<ClassNodeStruct::Impl>();

        INIT_NODE(classNode, context, parentNode, &_ClassVTable);
        classNode->lastChildNode = nullptr;
        classNode->firstChildNode = nullptr;

        Init::initNameNode(&classNode->nameNode, context, parentNode);

        classNode->startFound = false;

        Init::initSymbolNode(&classNode->bodyStartNode, context, parentNode, '{');
        Init::initSymbolNode(&classNode->endBodyNode, context, parentNode, '}');

        return classNode;
    }

    void Alloc::deleteClassNode(NodeBase *node) {
//        auto *classNode = Cast::downcast<ClassNodeStruct *>(node);
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

    int classBodyTokenizer(TokenizerParams_parent_ch_start_context) {
        auto *classNode = Cast::downcast<ClassNodeStruct *>(parent);

        if (!classNode->startFound) {
            if (ch == '{') {
                classNode->startFound = true;
                context->codeNode = Cast::upcast(&classNode->bodyStartNode);
                return start + 1;
            }
        } else if (ch == '}') {
            context->scanEnd = true;
            context->codeNode = Cast::upcast(&classNode->endBodyNode);
            return start + 1;
        } else {
            int result;
            if (-1 < (result = Tokenizers::classTokenizer(parent, ch, start, context))) {
                auto *innerClassNode = Cast::downcast<ClassNodeStruct *>(parent);
                appendChildNode(innerClassNode, context->codeNode);
                return result;
            }
        }

        return -1;
    };


    int Tokenizers::classTokenizer(TokenizerParams_parent_ch_start_context) {
        static constexpr const char class_chars[] = "class";
        static constexpr unsigned int size_of_class = sizeof(class_chars) - 1;

        if ('c' == ch) {
            Tokenizers::WordTokenizer(TokenizerParams_pass, 'c', "class");

            auto idx = ParseUtil::matchFirstWithTrim(context->chars, class_chars, start);
            if (idx > -1) {
                if (idx + size_of_class < context->length
                    && ParseUtil::isSpace(context->chars[idx + size_of_class])
                        ) {

                    int returnPos = idx + size_of_class;

                    // here "class " came
                    auto *classNode = Alloc::newClassNode(context, parent);

                    {
                        returnPos = Scanner::scan(&classNode->nameNode,
                                                  Tokenizers::nameTokenizer,
                                                  returnPos,
                                                  context);

                        if (returnPos == -1) {
                            // a class should have a class name
                            context->codeNode = Cast::upcast(classNode);
                            return context->former_start;
                        }

                        //console_log("name=" + std::string(classNode->nameNode.name));
                    }

                    // Parse body
                    if (-1 == (returnPos = Scanner::scan(classNode, classBodyTokenizer,
                                                         returnPos, context))) {
                        context->codeNode = Cast::upcast(classNode);
                        return context->former_start;
                    }

                    context->codeNode = Cast::upcast(classNode);
                    return returnPos;
                }
            }
        }
        return -1;
    };
}



