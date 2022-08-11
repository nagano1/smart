
#include <cstdio>
#include <iostream>
#include <string>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>
#include <cstdint>

#include "script_runtime.hpp"

namespace smart {

    FuncNodeStruct* findMainFunc(DocumentStruct *document)
    {
        auto *rootNode = document->firstRootNode;
        while (rootNode != nullptr) {
            if (rootNode->vtable == VTables::FnVTable) {
                // fn
                auto *fnNode = Cast::downcast<FuncNodeStruct*>(rootNode);
                auto *nameNode = &fnNode->nameNode;
                if (ParseUtil::equal(nameNode->name, nameNode->nameLength, "main", 4))
                {
                    return fnNode;
                }
            }
            rootNode = rootNode->nextNode;
        }
        return nullptr;
    }


    void evaluateNode(NodeBase *valueNode)
    {

    }

    void executeMain(FuncNodeStruct* mainFunc) {
        auto* childNode = mainFunc->bodyNode.firstChildNode;
        while (childNode) {
            if (childNode->vtable == VTables::CallFuncVTable) {
                auto* funcCall = Cast::downcast<CallFuncNodeStruct*>(childNode);

                evaluateNode(funcCall->valueNode);


                auto* arg = funcCall->firstArgumentItem;
                if (arg != nullptr) {
                    while (true) {
                        printf("arg = <%s>\n", arg->valueNode->vtable->typeChars);
                        auto* valueNode = arg->valueNode;
                        if (valueNode->vtable == VTables::StringLiteralVTable) {
                            auto* stringArg = Cast::downcast<StringLiteralNodeStruct*>(valueNode);
                            printf("arg = <%s>\n", stringArg->str);

                        }

                        if (arg->nextNode == nullptr) {
                            break;
                        }
                        else {
                            arg = Cast::downcast<FuncArgumentItemStruct*>(arg->nextNode);
                        }
                    }
                }

            }
            printf("<%s>", childNode->vtable->typeChars);

            childNode = childNode->nextNode;
        }

    }

    void startScript(DocumentStruct* document)
    {
        auto *mainFunc = findMainFunc(document);
        if (mainFunc) {
            printf("main Found");
            printf("<%s()>\n", mainFunc->nameNode.name);
            executeMain(mainFunc);

        }

        auto* rootNode = document->firstRootNode;
        while (rootNode != nullptr) {

            // printf("%s\n", rootNode->vtable->typeChars);

            if (rootNode->vtable == VTables::ClassVTable) {
                // class
            }
            else if (rootNode->vtable == VTables::FnVTable) {
                // fn
                
            }

            rootNode = rootNode->nextNode;
        }
    }
}