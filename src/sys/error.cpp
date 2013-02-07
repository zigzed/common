/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/sys/error.h"
#include <string.h>

namespace cxx {
    namespace sys {

        namespace err {

#ifndef OS_WINDOWS
            int         get()
            {
                int code = errno;
                return code;
            }

            void        set(int code)
            {
                errno = code;
            }

            std::string str(int code)
            {
                char buf[1024];
                strerror_r(code, buf, sizeof(buf));
                return buf;
            }
#else

            int         get()
            {
                return GetLastError();
            }

            void        set(int code)
            {
                SetLastError(code);
            }

            std::string str(int code)
            {
                char buf[1024];
                int  len = 1024;
                DWORD rc = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                         NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                         (LPSTR)buf, len, NULL);
                return buf;
            }
#endif

        }

    }
}
