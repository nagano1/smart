//#include <stdio.h>
//#include <sstream>
//#include <iostream>
//#include <algorithm>
//#include <iterator>
//#include <condition_variable>
//#include <unordered_map>
//#include <chrono>
//#include <thread>
//#include <atomic>
//#include <pthread.h>
//#include <time.h>
//#include <random>
//#include <vector>
//#include <windows.h>
//#include <winnt.h>

//using namespace std;

// sample code of 
/*
DWORD WINAPI ThreadFunc(LPVOID arg)
{
	int i;
	for (i = 0; i < 100; i++) {
		//printf("ThreadFunc %d\n", i);
		//Sleep(50);
	}
    //InterlockedCompareExchange64;
	return 0;
}
*/


int func(int k) {
    int i = 99;
    i += k;
    //std::atomic<int> g{0};
    //g = g+1;
    //printf("Hello, world!%d \n", i);
    
    //std::thread([](int *index) {
        /*
        *index = *index+59;
        if (*index>55) {
            *index = *index+60;
        }
        */
    //}, &i).join();

    //add_thread.detach();
    /*
    */

    //pthread_cond_timedwait;

//#include <pthread.h>
//#include <time.h>

    /*
    pthread_cond_t cv;
    pthread_mutex_t mp;
    struct timespec abstime;
    timespec_get(&abstime, TIME_UTC);
    abstime.tv_sec += 1;
    //abstime.tv_nsec = 1;
    //time.tv_sec += 5;
    int ret = pthread_cond_timedwait(&cv, &mp, &abstime); 
    */


    return 9*i-20;
}


int fib(int n) {
    int i, t, a = 0, b = 1;
    for (i = 0; i < n; i++) {
        t = a + b; a = b; b = t;
    }
    return b;
}
