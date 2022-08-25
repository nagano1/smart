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

void call() {
    char b = 3;
    uint64_t addr = (uint64_t)&b;
    printf("%llu", addr);

}

int main()
{
    int a = 3;
    printf("%llu\n", &a);
    call();
    printf("Smart Lang v0.0.1\n");

    constexpr char text[] = R"(
fn main() {
    let a = (
        3 + 5 + 6
    )
    int b = 0
    
    print("test日本語", 4321 + 5)
}
)";

    ScriptEnv::startScript((char*)text, sizeof(text)-1);



    /*
    auto* item = document->hashMap->get2("method");

//    item->vtable, VTables::StringLiteralVTable

    auto* strNode = Cast::downcast<StringLiteralNodeStruct*>(item);
    strNode->strLength;
    strNode->textLength;
    */

    printf("\n\n...end\n");
}
