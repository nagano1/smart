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

    static st_textlen selfTextLength(ClassNodeStruct *classNode) {
        return 5;
    }

    static const utf8byte *selfText(ClassNodeStruct *node) {
        return "class";
    }

    static CodeLine *appendToLine(ClassNodeStruct *self, CodeLine *currentCodeLine) {
        auto *classNode = self;


        currentCodeLine = currentCodeLine->addPrevLineBreakNode(classNode);

        currentCodeLine->appendNode(classNode);


        auto formerParentDepth = self->context->parentDepth;
        self->context->parentDepth += 1;
        currentCodeLine = VTableCall::appendToLine(&classNode->nameNode, currentCodeLine);
        self->context->parentDepth = formerParentDepth;


        currentCodeLine = VTableCall::appendToLine(&classNode->bodyStartNode, currentCodeLine);
        

        formerParentDepth = self->context->parentDepth;
        self->context->parentDepth += 1;

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
        
        self->context->parentDepth = formerParentDepth;


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
                                                          classTypeText
                                                          ,NodeTypeId::Class);

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

        //console_log(std::string(""+ch).c_str());
        //console_log((std::string{"==,"} + std::string{ch} + std::to_string(ch)).c_str());


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
            auto idx = ParseUtil::matchFirstWithTrim(context->chars, class_chars, start);
            if (idx > -1) {
                if (idx + size_of_class < context->length
                    && ParseUtil::isSpaceOrLineBreak(context->chars[idx + size_of_class])
                        ) {

                    int currentPos = idx + size_of_class;
                    int resultPos = -1;

                    // console_log(std::string("wow5").c_str());

                    // "class " came here
                    auto *classNode = Alloc::newClassNode(context, parent);

                    {
                        resultPos = Scanner::scan(&classNode->nameNode,
                                                  Tokenizers::nameTokenizer,
                                                  currentPos,
                                                  context);

                        if (resultPos == -1) {
                            // the class should have a class name
                            //console_log(std::string(classNode->nameNode.name).c_str());

                            context->codeNode = Cast::upcast(classNode);
                            return currentPos;
                        }

                        //console_log("name=" + std::string(classNode->nameNode.name));
                    }


                    // Parse body
                    currentPos = resultPos;
                    if (-1 == (resultPos = Scanner::scan(classNode, classBodyTokenizer,
                                                         currentPos, context))) {
                        context->codeNode = Cast::upcast(classNode);



                        return currentPos;
                    }

                    context->codeNode = Cast::upcast(classNode);
                    return resultPos;
                }
            }
        }
        return -1;
    };
}



