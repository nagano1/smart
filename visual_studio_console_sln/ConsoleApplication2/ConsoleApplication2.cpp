#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <array>
#include <algorithm>

#include <windows.h>
//#include <winnt.h>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <vector>

#include <cstdint> 
#include <ctime>
#include <emmintrin.h>


#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include "thread_pool.hpp"
#include "LSP_Server/LSPMain.hpp"

#pragma execution_character_set( "utf-8" )

constexpr uint64_t NANOS_IN_SECOND = 1000 * 1000 * 1000;


//int wakeup_count = 0;
std::atomic<unsigned long long > head3{ 2845 };

bool stopped = false;

void lsp_server();
int test_central_job_queues();

extern "C" NTSTATUS NtWriteVirtualMemory(HANDLE processHandle, PVOID baseAddress, PVOID buffer, ULONG numberOfBytesToWrite, PDWORD numberOfBytesWritten);

int main() {

    /*
    auto x = 100;
    auto y = 999;

    std::cout << "x:" << x << std::endl;
    DWORD numBytesWritten = 0;
    NTSTATUS success = NtWriteVirtualMemory(GetCurrentProcess(), &x, &y, sizeof(y), &numBytesWritten);

    std::cout << "x:" << x << std::endl;
    */

    //test_central_job_queues();
    auto _ = _setmode(_fileno(stdout), 0x8000); // binmode
    LSPManager::LSP_main();

    return 0;
}

void lsp_server() {
    while (true) {
        std::string text;
        std::cin >> text;
        std::cout << text;
    }
}

int test_central_job_queues() {

    SetConsoleOutputCP(65001);


    ThreadPool tp;
    tp.init();

    auto start = std::chrono::high_resolution_clock::now();

    auto ajwoief = {

        324
    };

    tp.start();





    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    stopped = true;

    auto elapsed = std::chrono::high_resolution_clock::now() - start;

    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    long long diff = microseconds * 1000;


    for (int i = 0; i < threads_size; i++) {
        tp.threads[i]->doStop();
    }

    for (int i = 0; i < threads_size; i++) {
        tp.threads[i]->join();
    }


    std::cout << diff << "diff!\n" << "\n";

    long long count = 0;
    for (int i = 0; i < threads_size; i++) {
        count += tp.threads[i]->set_count;
        std::cout << i << ": " << tp.threads[i]->set_count << ": \n";
    }

    long long count_b = 0;
    for (int i = 0; i < thread_queue_size; i++) {
        std::cout << " \n";

        //count_b += tp.thread_queue[i]->lastRunNode.load()->idx;

        auto *firstNode = POINTER_LOAD(tp.thread_queue[i]->doingRunNode);
        auto *lastNode = POINTER_LOAD(tp.thread_queue[i]->lastRunNode);

        int count_temp = -1;
        while (firstNode) {
            count_temp++;
            //printf("%d,", firstNode->nodeIndex.load());
            if (firstNode == lastNode) {
                printf("\nlastNode->nodeIndex = %llu\n", POINTER_LOAD(lastNode->nodeIndex));
                break;
            }
            firstNode = firstNode->next;
        }

        count_b += count_temp;
        //		std::cout << i << ":: " << tp.thread_queue[i]->lastRunNode.load()->idx << "\n";// << aobj->gf;
    }

    std::cout << count << ": count by each thread: " << " " << "\n";// << aobj->gf;
    std::cout << count_b << ": last node counter: " << "   !\n" << "\n";// << aobj->gf;
    std::cout << diff << "diff         !\n" << "\n";// << aobj->gf;

    return count == count_b ? 0 : 1;

    //TestUtil::testUtf8Text();


    //std::cout << "\set_count = " << set_count_b << "\n";
    //std::cout << "\nconflict = " << conflict << "\n";
    //std::cout << "\nconflict2 = " << conflict2 << "\n";

    //_getchar_nolock();
}