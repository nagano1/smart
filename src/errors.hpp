#pragma once
//
//#include <stdlib.h>
//#include <array>
//
//#include <cstdlib>
//#include <cassert>
//#include <cstdio>
//#include <chrono>
//#include <unordered_map>
//
//#include <cstdint> // uint64_t, int_fast32_t
//#include <ctime>
//
//#include <string.h> // memcpy
//
//
////using utf8byte = char;
///*
// *
// * aewffweoif
// *     <int>
// *     () {
// *
// * }
// *
// *
// */
namespace smart {



    enum class ErrorCode {
        first_keeper,

        missing_closing_quote,
        missing_closing_quote2,
        missing_object_delemeter,

        last_keeper
    };

    static constexpr int errorListSize = 1 + static_cast<int>(ErrorCode::last_keeper);

    struct ErrorInfo {
        ErrorCode errorIndex;
        int errorCode;
        const char* msg;
    };

    extern ErrorInfo ErrorInfoList[errorListSize];
    extern bool errorInfoInitialized;
    static ErrorInfo sortErrorInfoList[errorListSize];




    static int acompare(void const * alhs, void const * arhs) {
        ErrorInfo* lhs = (ErrorInfo*)alhs;
        ErrorInfo* rhs = (ErrorInfo*)arhs;

        if (lhs->errorCode == rhs->errorCode) {
#if defined(_MSVC_LANG) //_MSC_VER  _MSVC_LANG _MSC_BUILD
            printf("duplicate error id(%d)\n ", lhs->errorCode);
#endif
            //exit(9990);
            return 0;
        }
        else if (lhs->errorCode > rhs->errorCode) {
            return 1;
        }
        else {
            return -1;
        }
        //return lhs->errorCode - rhs->errorCode;
        return 0;
    }


    // C++-14
    /*
    static constexpr bool is_sorted(const ErrorInfo tempList[])
    {
        for (std::size_t i = 0; i < errorListSize - 1; ++i) {
            if (tempList[i].errorCode >= tempList[i + 1].errorCode) {
                return false;
            }
        }
        return true;
    }
    */


    static int initErrorInfoList()
    {
        errorInfoInitialized = true;

        ErrorInfo tempList[] = {
            ErrorInfo{ ErrorCode::first_keeper, 9912, "start"},

            ErrorInfo{ ErrorCode::missing_closing_quote, 989800, "missing closing quote" },
            ErrorInfo{ ErrorCode::missing_closing_quote2, 989900, "missing closing quote" },

            ErrorInfo{ ErrorCode::missing_object_delemeter, 7777812, "missing object delimeter"},

            ErrorInfo{ ErrorCode::last_keeper, 9999999, "end" },
        };


        static_assert(errorListSize == (sizeof tempList) / sizeof(ErrorInfo), "error list should have the same length");
        //static_assert(is_sorted(tempList), "error List should be sorted"); // C++14


        constexpr int len = (sizeof tempList) / (sizeof tempList[0]);
        for (int i = 0; i < len; i++) {
            auto &&errorInfo = tempList[i];
            if (static_cast<int>(errorInfo.errorIndex) != i) {
                printf("error info index\n");
            }

            ErrorInfoList[static_cast<int>(tempList[i].errorIndex)] = errorInfo;
            sortErrorInfoList[static_cast<int>(tempList[i].errorIndex)] = errorInfo;
        }

        // check duplicate of error code
        //std::sort(sortErrorInfoList, sortErrorInfoList + errorListSize, acompare);
        qsort(sortErrorInfoList, sizeof(sortErrorInfoList) / sizeof(sortErrorInfoList[0]), sizeof(ErrorInfo), acompare);

        return 0;
    }






    enum class Language {
        en = 8591000,
        jp = 8591001,
    };


    static const char *translateErrorMessage(ErrorCode errorCode, Language lang) {
        return nullptr;
    }


    static const char *getErrorMessage(ErrorCode errorCode) {
        if (errorInfoInitialized == false) {
            initErrorInfoList();
        }

        const char *mes = nullptr;
        auto&& errorInfo = ErrorInfoList[static_cast<int>(errorCode)];
        mes = errorInfo.msg;

        auto *transMess = translateErrorMessage(errorCode, Language::jp);
        if (transMess != nullptr) {
            mes = transMess;
        }

        return mes;
    }



    #define MAX_REASON_LENGTH 1024
    /**
     * Syntax error is allowed only once
     */
    using SyntaxErrorInfo = struct _errorInfo {
        bool hasError{false};

        ErrorCode errorCode;
        char reason[MAX_REASON_LENGTH + 1];
        st_textlen reasonLength = 0;

        st_uint charPosition;
        int charEndPosition;

        // 0: "between start and  end"
        // 1: "from start to end of line,"
        int errorDisplayType = 0;

        static const int SYNTAX_ERROR_RETURN = -1;

        static void setError(_errorInfo *error, ErrorCode errorCode, st_uint start) {
            error->hasError = true;
            error->errorCode = errorCode;
            error->charPosition = start;

            const char* reason = getErrorMessage(errorCode);
            if (reason == nullptr) {
                reason = "";
            }
            st_textlen len = (st_textlen) strlen(reason);
            error->reasonLength = len < MAX_REASON_LENGTH ? len : MAX_REASON_LENGTH;
            memcpy(error->reason, reason, error->reasonLength);
            error->reason[error->reasonLength] = '\0';
        }
    };

}

