/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_SYS_SEH_H
#define CXX_SYS_SEH_H
#include "common/config.h"
#ifdef  OS_WINDOWS
#define _WIN32_WINNT	0x0400
#if	(_WIN32_WINNT < 0x0400)
    #error	Require Windows NT Server 3.5 and later
#endif
#include <crtdbg.h>
#include <windows.h>
#include <imagehlp.h>
#include <string>

// use SEH, imagehlp.lib is needed
#pragma comment(lib, "imagehlp.lib")

namespace cxx {
    namespace sys {
        namespace seh {

            // predefined exception handlers
            // 1. predefined exception handler, shutdown the process
            struct shutdown {
                static LONG handler(const char*, PEXCEPTION_POINTERS);
            };

            // 2. predefined exception handler, report to file
            struct report {
                static LONG handler(const char*, PEXCEPTION_POINTERS);
            };


            // windows structure exception filter
            template<typename HANDLER>
            class filter {
            public:
                static void install();
                static void set_log_file(const char* file);
                static void uninstall();
                ~filter();
            private:
                static LONG WINAPI callback(EXCEPTION_POINTERS* );
                static char* get_file_name();
                static LPTOP_LEVEL_EXCEPTION_FILTER& get_previous_filter();
            };

            template<typename HANDLER>
            inline char* filter<HANDLER>::get_file_name()
            {
                static char log_file_name[MAX_PATH] = {0};
                return log_file_name;
            }

            template<typename HANDLER>
            inline LPTOP_LEVEL_EXCEPTION_FILTER& filter<HANDLER>::get_previous_filter()
            {
                static LPTOP_LEVEL_EXCEPTION_FILTER	previous_filter = 0;
                return previous_filter;
            }

            template<typename HANDLER>
            inline void filter<HANDLER>::install()
            {
                get_previous_filter() = ::SetUnhandledExceptionFilter(callback);
                ::GetModuleFileName(NULL, get_file_name(), MAX_PATH);
                char* dot = ::strrchr(get_file_name(), '.');

                // replace the extension with "rpt"
                if(dot) {
                    ++dot;
                    if(strlen(dot) >= 3) {
                        ::strcpy(dot, "rpt");
                    }
                }
            }

            template<typename HANDLER>
            inline void filter<HANDLER>::uninstall()
            {
                ::SetUnhandledExceptionFilter(get_previous_filter());
            }

            template<typename HANDLER>
            inline void filter<HANDLER>::set_log_file(const char* file)
            {
                if(!file)
                    return;
                ::strncpy(get_file_name(), file, std::min(strlen(file), MAX_PATH));
            }

            template<typename HANDLER>
            inline LONG WINAPI filter<HANDLER>::callback(EXCEPTION_POINTERS* info)
            {
                HANDLER::handler(get_file_name(), info);
                if(get_previous_filter())
                    return (get_previous_filter())(info);

                return EXCEPTION_CONTINUE_SEARCH;
            }

        }

    }
}

#else   /** OS_WINDOWS */

namespace cxx {
    namespace sys {
        namespace seh {

            // 对于其他操作系统，没有结构化异常处理，因此创建一些空的结构，便于编译通过
            struct EXCEPTION_POINTERS {
            };

            struct LPTOP_LEVEL_EXCEPTION_FILTER {
            };

            typedef EXCEPTION_POINTERS* PEXCEPTION_POINTERS;
            typedef long                LONG;
            #define WINAPI

            struct shutdown {
                static LONG handler(const char*, PEXCEPTION_POINTERS);
            };

            // 2. predefined exception handler, report to file
            struct report {
                static LONG handler(const char*, PEXCEPTION_POINTERS);
            };

            // windows structure exception filter
            template<typename HANDLER>
            class filter {
            public:
                static void install() {}
                static void set_log_file(const char* file) {}
                static void uninstall() {}
                ~filter();
            private:
                static LONG WINAPI callback(EXCEPTION_POINTERS* ) { return 0; }
                static char* get_file_name() { return "NULL"; }
                static LPTOP_LEVEL_EXCEPTION_FILTER& get_previous_filter() {
                    static LPTOP_LEVEL_EXCEPTION_FILTER dummy;
                    return dummy;
                }
            };

        }
    }
}


#endif  /** OS_WINDOWS */


#endif
