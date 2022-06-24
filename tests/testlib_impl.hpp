#pragma  once

#include <array>
#include <vector>
#include <atomic>
#include <iostream>
#include <algorithm>
#include <string>
#include <mutex>
#include <thread>

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <sstream>


#include <unordered_map>
#include <cstdint>

#ifdef __ANDROID__
#include <android/log.h>
#endif



namespace smartlang {

    class TestInfo {
    public:
        // Destructs a TestInfo object.  This function is not virtual, so
        // don't inherit from TestInfo.
        ~TestInfo();

        // Returns the test case name.
        const char *test_case_name() const { return test_case_name_.c_str(); }

        // Returns the test name.
        const char *name() const { return name_.c_str(); }

        // Returns the name of the parameter type, or NULL if this is not a typed
        // or a type-parameterized test.
        const char *type_param() const {
            if (type_param_.get() != nullptr)
                return type_param_->c_str();
            return nullptr;
        }

        // Returns the text representation of the value parameter, or NULL if this
        // is not a value-parameterized test.
        const char *value_param() const {
            if (value_param_.get() != nullptr)
                return value_param_->c_str();
            return nullptr;
        }

    private:
        const std::unique_ptr<const ::std::string> type_param_;
        // Text representation of the value parameter, or NULL if this is not a
        // value-parameterized test.
        const std::unique_ptr<const ::std::string> value_param_;

        const std::string test_case_name_; // TestImpl case name
        const std::string name_;           // TestImpl name
        // bool should_run_;                  // True if this test should run
        // bool is_disabled_;                 // True if this test is disabled
        // bool matches_filter_;              // True if this test matches the
    };


    class LogStream {
        int mo, da, yr;
    public:
        std::mutex mtx;

        LogStream(int m, int d, int y) {
            mo = m;
            da = d;
            yr = y;
        }
    };



    class PrintThread: public std::ostringstream
    {
    public:
        PrintThread() = default;

        ~PrintThread()
        {
            std::lock_guard<std::mutex> guard(_mutexPrint);
            std::cout << this->str();
        }

    private:
        static std::mutex _mutexPrint;
    };





/*
    ostream &operator<<(ostream &os, const LogStream &dt) {
        //std::lock_guard<std::mutex> guard(dt.mtx);
        os << dt.mo << '/' << dt.da << '/' << dt.yr;
        return os;
    }
 */



    class Test {
    public:
        friend class smartlang::TestInfo;
        static Test* this_current_test;
        static std::stringstream logstream;
        static std::mutex log_mtx;

        bool failed = false;


        // Defines types for pointers to functions that set up and tear down
        // a test case.
        //typedef internal::SetUpTestCaseFunc SetUpTestCaseFunc;
        //typedef internal::TearDownTestCaseFunc TearDownTestCaseFunc;

        // The d'tor is virtual as we intend to inherit from TestImpl.
        virtual ~Test();

        virtual void TestBody() = 0;

        void runTestBody() {
            this_current_test = this;
            TestBody();
            this_current_test = nullptr;
        }
    };

    class Log: public std::ostringstream {
    public:
        Log() = default;

        ~Log()
        {
            std::lock_guard<std::mutex> guard(Test::log_mtx);
            Test::logstream << this->str();
        }
    };


    class TestFactoryBase {

    public:
        virtual ~TestFactoryBase() {}

        // Creates a test instance to run. The instance is both created and destroyed
        // within TestInfoImpl::Run()
        virtual smartlang::Test *CreateTest() = 0;

    protected:
        TestFactoryBase() {}
    };

    // This class provides implementation of TeastFactoryBase interface.
    // It is used in TEST and TEST_F macros.
    template<class TestClass>
    class TestFactoryImpl : public TestFactoryBase {
    public:
        virtual smartlang::Test *CreateTest() { return new TestClass; }
    };

    class CPPTestUtil {
    public:
        static int *MakeAndRegisterTestInfo(const char *testcasename, const char *testname,
                                            const char *filename, int lineno,
                                            TestFactoryBase *factory);

        static void *doAllTests();

        static unsigned int runTest(const char *testcasename, const char *testname);

        static std::string runTestNext(int testIndex);

    };


    // Expands to the name of the class that implements the given test.
#define GTEST_TEST_CLASS_NAME_(test_case_name, test_name) \
    test_case_name##_##test_name##_Test

// Helper macro for defining tests.
#define GTEST_TEST_(test_case_name, test_name, parent_class)                      \
    class GTEST_TEST_CLASS_NAME_(test_case_name, test_name) : public parent_class \
    {                                                                             \
    public:                                                                       \
        GTEST_TEST_CLASS_NAME_(test_case_name, test_name)                         \
        () {}                                                                     \
        ~GTEST_TEST_CLASS_NAME_(test_case_name, test_name)                         \
        (){}                                                                       \
    private:                                                                      \
        static int *test_info_;                                                   \
        virtual void TestBody(); \
};                                                                            \
                                                                                \
    int *GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::test_info_ =          \
        smartlang::CPPTestUtil::MakeAndRegisterTestInfo(#test_case_name, #test_name,__FILE__,__LINE__ \
        ,new smartlang::TestFactoryImpl<GTEST_TEST_CLASS_NAME_(test_case_name, test_name)> ); \
                                                                                  \
void GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::TestBody()

#define GTEST_TEST(test_case_name, test_name) \
    GTEST_TEST_(test_case_name, test_name, smartlang::Test)

#define TEST(test_case_name, test_name) GTEST_TEST(test_case_name, test_name)

    static void test_is_true(bool true_test, bool t1, const char *t1_chars) {
        std::stringstream s;
        s << (t1 ? "true" : "false");

        bool ok = true_test == t1;
        std::lock_guard<std::mutex> guard(Test::log_mtx);

        Test::logstream <<  (ok? "\n    ✅ OK: ": "\n    ❌ NG: ")
                        << s.str().c_str() << " "
                        << std::endl << "　　　　    ( " << t1_chars << " "<<  ")"
                        << std::endl << "    ";

        if (!ok) {
            if (Test::this_current_test != nullptr) {
                Test::this_current_test->failed = true;
            }
        }
    }

    template<typename T1, typename T2>
    static void test_binary_op(T1 t1, T2 t2, const char *t1_chars, const char *t2_chars, bool ok, const std::string &op) {
        std::stringstream s;

        if (std::is_same<T1, bool>::value) {
            s << (*(bool*)&t1 ? "true" : "false");
        }  // optimizable...

        else if (sizeof(T1) == 1) { // for avoiding crash on android
            char moji[8];
            sprintf(moji, "%d", *(int*)(&t1));
            s << moji;
        } else {
            s << t1;
        }
        std::stringstream s2;

        if (std::is_same<T2, bool>::value) {
            s2 << (*(bool*)&t2 ? "true" : "false");

        } else if (sizeof(T2) == 1) {
            char moji[8];
            sprintf(moji, "%d", *(int*)(&t2));
            s2 << moji;
        } else {
            s2 << t2;
        }

        std::lock_guard<std::mutex> guard(Test::log_mtx);

        Test::logstream <<  (ok? "\n    ✅ OK: ": "\n    ❌ NG: ")
                        << s.str().c_str() << " " << op << " " << s2.str().c_str()
                        << std::endl << "　　　　   (" << t1_chars << " "<< op << " " << t2_chars <<  ")"
                        << std::endl << "    ";

        if (!ok) {
            if (Test::this_current_test != nullptr) {
                Test::this_current_test->failed = true;
            }
        }
    }

    template<typename T1, typename T2>
    static void ecompare_static(T1 t1, T2 t2, const char *t1_chars, const char *t2_chars) {
        test_binary_op(t1, t2, t1_chars, t2_chars, t1 == t2, "==");
    }

    static void etrue_static(bool true_test, bool is_true, const char *t1_chars) {
        test_is_true(true_test, is_true, t1_chars);
    }


    template<typename T1, typename T2>
    static void ecompare_not_equal(T1 t1, T2 t2, const char *t1_chars, const char *t2_chars) {
        test_binary_op(t1, t2, t1_chars, t2_chars, t1 != t2, "!=");
    }


    /*
    template<>
    void ecompare_static(float t1, float t2, const char *t1_chars, const char *t2_chars) {

    }
     */


    template<typename T1, typename T2>
    static void lt_static(T1 t1, T2 t2, const char *t1_chars, const char *t2_chars) {
        test_binary_op(t1, t2, t1_chars, t2_chars, t1 < t2, "<");

    }

    template<typename T1, typename T2>
    static void gt_static(T1 t1, T2 t2, const char *t1_chars, const char *t2_chars) {
        test_binary_op(t1, t2, t1_chars, t2_chars, t1 > t2, ">");
    }

#define FAIL() \
smartlang::etrue_static(false, true, "FAIL()");

#define EXPECT_EQ(val1, val2) \
smartlang::ecompare_static(val1, val2, #val1, #val2)

#define EXPECT_TRUE(val1) \
smartlang::etrue_static(true, val1, #val1)

#define EXPECT_FALSE(val1) \
smartlang::etrue_static(false, val1, #val1)


#define EXPECT_NE(val1, val2) \
smartlang::ecompare_not_equal(val1, val2, #val1, #val2)



#define EXPECT_LT(val1, val2) \
smartlang::lt_static(val1, val2, #val1, #val2)

#define EXPECT_GT(val1, val2) \
smartlang::gt_static(val1, val2, #val1, #val2)


}


long funca();

class Test2 {
public:
    static long g;
};

class JsCallFromBackground {
    void sta_func() {
        EXPECT_TRUE(true);
        EXPECT_FALSE(false);
    }
};


