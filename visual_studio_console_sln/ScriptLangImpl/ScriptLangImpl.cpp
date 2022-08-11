#include <cstdio>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <atomic>
#include <condition_variable>
#include <unordered_map>


#include "code_nodes.hpp"
#include "parse_util.hpp"
#include "script_runtime/script_runtime.hpp"

using namespace smart;


int main()
{
    printf("Smart Lang v0.0.1\n");

    constexpr char text[] = R"(
fn main() {
    let a = 0
    int b = 0
    print("test日本語")
}
)";

    const char* chars = text;// .c_str();
    auto* document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
    DocumentUtils::parseText(document, chars, sizeof(text)-1);
    DocumentUtils::generateHashTables(document);

    char* treeText = DocumentUtils::getTextFromTree(document);
    //printf("%s\n", treeText);

    startScript(document);


    auto* rootNode = document->firstRootNode;
    while (rootNode != nullptr) {

        // printf("%s\n", rootNode->vtable->typeChars);
        
        if (rootNode->vtable == VTables::ClassVTable) {
            // class
        }
        else if (rootNode->vtable == VTables::FnVTable) {
            // fn
            auto* fnNode = Cast::downcast<FuncNodeStruct*>(rootNode);
            printf("<%s()>\n", fnNode->nameNode.name);
            auto* childNode = fnNode->bodyNode.firstChildNode;
            while (childNode) {
                if (childNode->vtable == VTables::CallFuncVTable) {
                    printf("Func Call!!!!!!!!!!!!!!!!\n");
                    auto* funcCall = Cast::downcast<CallFuncNodeStruct*>(childNode);
                    funcCall->valueNode;


                    auto* arg = funcCall->firstArgumentItem;
                    if (arg != nullptr) {
                        while (true) {
                            printf("arg ");
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

        rootNode = rootNode->nextNode;
    }

    Alloc::deleteDocument(document);

    /*
    auto* item = document->hashMap->get2("method");

//    item->vtable, VTables::StringLiteralVTable

    auto* strNode = Cast::downcast<StringLiteralNodeStruct*>(item);
    strNode->strLength;
    strNode->textLength;


    */

    printf("\n\n...end\n");
}
