#include <jni.h>
#include <string>
#include <mutex>
#include <thread>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <sstream>
#include <vector>
#include <array>

#include <locale>
#include <codecvt>

#include <cstdint>
#include <ctime>

#include "../../../../../tests/testlib_impl.hpp"

#include <android/log.h>
#include "utf8.h"


std::mutex mtx;

#ifndef ENDTEST
#define ENDTEST //[ENDTEST]
#endif

namespace doorlangtest {
    TEST(test_casewow, test_name239) {
        EXPECT_EQ(54, 54.0f);
    } ENDTEST

    TEST(test_casewow, test_name2399) {
        // __android_log_print(ANDROID_LOG_DEBUG, "Tag", "TestImpl Message : aow");
        EXPECT_EQ(54.8f, 54.9f);
    } ENDTEST


    typedef int(*handle_character_proc)(int next_character);

    int handle1(int a) {
        int g = a * a * a * a;
        return g;
    }

    /*
     * 次の文字に対する処理を実行する関数を
     * 関数ポインタで保持。
     */
    static handle_character_proc state_proc;

    /*
    auto handle1 = [](int a){
        return 5;
    };
     */
    static void SetStateProc(handle_character_proc next_proc) {
        state_proc = next_proc;
    }


    extern "C" JNIEXPORT jdouble JNICALL
    Java_org_rokist_canlangtest_MainActivity_donothing( /* do nothing*/
            JNIEnv *env,
            jobject /* this */) {


        unsigned long long along = 124;
        switch (along) {
            case 124:

                break;
        }


        auto startTime = std::chrono::high_resolution_clock::now();


        //std::thread thread1([]() {
        //static long num_steps = 1000000000;
        static long num_steps = 1000 * 1000 * 100;
        double step;

        double pi = 0.0;
        double sum = 0.0;
        std::mutex mtx;

        step = 1.0 / (double) num_steps;




        //#pragma omp parallel
        {
            //int i;
            long steps100 = num_steps / 8;

            std::vector<std::thread *> args;
            auto end = 0;

            SetStateProc(handle1);

            while (true) {
                auto start = end + 1;
                end = start + steps100;
                bool finished = false;
                if (end > num_steps) {
                    finished = true;
                }

                std::atomic<int> *atomic_add = nullptr;
                atomic_add = new std::atomic<int>{24};


                auto tha = new std::thread(
                        [&sum/*, &mtx*/](int start, int end, double step, std::atomic<int> *atomic_add
                        ) {
                            double x = 0.0;
                            double sum_here = 0.0;
                            double this_step = step;
                            int this_start = start;
                            int this_end = end;

                            int twf = 24;


                            //handle_character_proc state_proc2 = state_proc;

                            for (int j = this_start; j <= this_end; j++) {
                                atomic_add->compare_exchange_weak(twf, 24);
                                //for (i = 1; i <= steps100; i++) {

                                x = (j - 0.5) * this_step;
                                auto p = 4.0 / (1.0 + x * x);
                                //mtx.lock();
                                //state_proc2(3);
                                sum_here += p;
                                //mtx.unlock();
                                //sum = sum +、 4.0/(1.0+x*x);
                                //}
                            }

                            sum += sum_here;


                        }, start, end, step, atomic_add
                );

                args.push_back(tha);
                if (finished) {
                    break;
                }
            }

            for (int th = 0; th < args.size(); th++) {
                args[th]->join();
                delete args[th];
            }

            //int a = 5;
            //#pragma omp for reduction(+:sum)

        }

        std::this_thread::sleep_for(std::chrono::microseconds(1 * 1000 * 1000));

        pi = step * sum;


        auto elapsed = std::chrono::high_resolution_clock::now() - startTime;

        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
                elapsed).count();

        __android_log_print(ANDROID_LOG_DEBUG, "Tag", "TestImpl  : text = %llu", microseconds);

        //printf("The computed value of Pi is: %1.10lf\n", pi);
        //});

        //thread1.join();

        //env->NewIntArray(1);

        //std::lock_guard<std::mutex> mtx_guard{mtx};
        /*
        jclass arrayClass = env->FindClass("java/lang/Object");
        jmethodID initMethod = env->GetMethodID(arrayClass, "<init>", "()V");
        //jmethodID addMethod = env->GetMethodID(arrayClass, "add", "(Ljava/lang/Object;)Z");
        jobject myArray = env->NewObject(arrayClass, initMethod);
        */
        return pi;
    }



    extern "C" JNIEXPORT jstring JNICALL
    Java_org_rokist_canlangtest_MainActivity_stringFromJNI(
            JNIEnv *env,
            jobject /* this */) {


        std::string hello = "Hello from C++ 1";
        std::unique_ptr<int> a;
        return env->NewStringUTF(hello.c_str());
    }

    long prev_time = 0;

    extern "C" JNIEXPORT jlong JNICALL
    Java_org_rokist_canlangtest_MainActivity_stringFromJNI2(
            JNIEnv *env,
            jobject /* this */,
            jlong addrGray
    ) {
        //jintArray result;
        //result = env->NewIntArray(25);

        std::string hello = "Hello from C++ 4";
        unsigned int G_NumOfCores = std::thread::hardware_concurrency();

        Test2::g = G_NumOfCores;

        return Test2::g;// env->NewStringUTF(hello.c_str());
    }


    std::wstring JavaToWSZ(JNIEnv *env, jstring string) {
        std::wstring value;
        if (string == NULL) {
            return value; // empty string
        }
        const jchar *raw = env->GetStringChars(string, NULL);
        if (raw != NULL) {
            jsize len = env->GetStringLength(string);
            //value.assign(raw, len);
            value.assign(raw, raw + len);
            env->ReleaseStringChars(string, raw);
        }
        return value;
    }


    extern "C" JNIEXPORT jlong JNICALL
    Java_org_rokist_canlangtest_MainActivity_runTest(
            JNIEnv *env,
            jobject /* this */,
            jstring testcasename,
            jstring testname
    ) {
        jboolean isCopy = JNI_FALSE;

        auto *cpp_testcasename = env->GetStringUTFChars(testcasename, &isCopy);
        auto *cpp_testname = env->GetStringUTFChars(testname, &isCopy);

        return smartlang::CPPTestUtil::runTest(cpp_testcasename, cpp_testname);


        //return 53;
    }

    extern "C" JNIEXPORT jstring JNICALL
    Java_org_rokist_canlangtest_MainActivity_runTestNext(
            JNIEnv *env,
            jobject /* this */,
            jlong testIndex
    ) {
        return env->NewStringUTF(smartlang::CPPTestUtil::runTestNext((int)testIndex).c_str());
    }


// convert wstring to UTF-8 string
    std::string wstring_to_utf8(const std::wstring &str) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
        return myconv.to_bytes(str);
    }

    extern "C" JNIEXPORT jlong JNICALL
    Java_org_rokist_canlangtest_MainActivity_doWithText(
            JNIEnv *env,
            jobject /* this */,
            jstring str
    ) {
        //jintArray result;
        //result = env->NewIntArray(25);

        //smartlang::CPPTestUtil::doAllTests();

        jboolean isCopy = JNI_FALSE;
        auto *text = env->GetStringUTFChars(str, &isCopy);
        //jsize len = env->GetStringLength(str);
        jsize len = env->GetStringUTFLength(str);

        std::string strin{text};
        // auto *g = L"iofw";
        std::wstring st = JavaToWSZ(env, str);


        char *twochars = (char *) text;
        char *w = twochars;
        utf8::advance(w, 0, w + len + 1);
        // auto valid = utf8::is_valid(w, w + len);


        // int alen = len;

        std::vector<char> chs;

//    char* w2 = twochars+4;
//    int cp = utf8::next(w2, twochars + len+38);
        //assert (cp == 0x65e5);
//    0x65e5/assert (w == twochars + 3);



        //assert (w == twochars + 5);

//    __android_log_print(ANDROID_LOG_DEBUG, "Tag", "TestImpl  : text = %s %d", w, cp);

        std::string hello = "Hello from C++ 4";

        // unsigned int G_NumOfCores = std::thread::hardware_concurrency();

        return 55;
    }


}