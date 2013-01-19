/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_STR_COMPARE_H
#define CXX_STR_COMPARE_H
#include <strings.h>
#include <locale>
#include <algorithm>
#include "common/config.h"

namespace cxx {
    namespace str {

#if     defined(OS_LINUX)
        int strcmp_ic(const char* arg1, const char* arg2)
        {
            return strcasecmp(arg1, arg2);
        }
#elif   defined(OS_WINDOWS)
        int strcmp_ic(const char* arg1, const char* arg2)
        {
            return stricmp(arg1, arg2);
        }
#endif

        template<typename T >
        struct is_equal {
            bool operator()(const T& arg1, const T& arg2) const {
                return arg1 == arg2;
            }
        };

        template<>
        struct is_equal<const char * > {
            bool operator()(const char*& arg1, const char*& arg2) const {
                return strcmp(arg1, arg2) == 0;
            }
        };

        template<typename T >
        struct is_case_equal {
            bool operator()(const T& arg1, const T& arg2) const {
                T lhs;
                T rhs;
                std::transform(arg1.begin(), arg1.end(), std::back_inserter(lhs), ::toupper);
                std::transform(arg2.begin(), arg2.end(), std::back_inserter(rhs), ::toupper);
                return lhs == rhs;
            }
        };

        template<>
        struct is_case_equal<const char* > {
            bool operator()(const char*& arg1, const char*& arg2) const {
                return strcmp_ic(arg1, arg2) == 0;
            }
        };

        template<typename T >
        struct is_less {
            bool operator()(const T& arg1, const T& arg2) const {
                return arg1 < arg2;
            }
        };

        template<>
        struct is_less<const char* > {
            bool operator()(const char*& arg1, const char*& arg2) const {
                return strcmp(arg1, arg2) < 0;
            }
        };

        template<typename T >
        struct is_case_less {
            bool operator()(const T& arg1, const T& arg2) const {
                T lhs;
                T rhs;
                std::transform(arg1.begin(), arg1.end(), std::back_inserter(lhs), ::toupper);
                std::transform(arg2.begin(), arg2.end(), std::back_inserter(rhs), ::toupper);
                return lhs < rhs;
            }
        };

        template<>
        struct is_case_less<const char* > {
            bool operator()(const char*& arg1, const char*& arg2) const {
                return strcmp_ic(arg1, arg2) < 0;
            }
        };

        template<typename T >
        struct is_not_greater {
            bool operator()(const T& arg1, const T& arg2) const {
                return arg1 <= arg2;
            }
        };

        template<>
        struct is_not_greater<const char* > {
            bool operator()(const char*& arg1, const char*& arg2) const {
                return strcmp(arg1, arg2) <= 0;
            }
        };

        template<typename T >
        struct is_not_case_greater {
            bool operator()(const T& arg1, const T& arg2) const {
                T lhs;
                T rhs;
                std::transform(arg1.begin(), arg1.end(), std::back_inserter(lhs), ::toupper);
                std::transform(arg2.begin(), arg2.end(), std::back_inserter(rhs), ::toupper);
                return lhs <= rhs;
            }
        };

        template<>
        struct is_not_case_greater<const char* > {
            bool operator()(const char*& arg1, const char*& arg2) const {
                return strcmp_ic(arg1, arg2) <= 0;
            }
        };

    }
}

#endif
