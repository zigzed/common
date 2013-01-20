/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/sys/dynlib.h"
#include "common/config.h"
#if     defined(OS_WINDOWS)
	#include <Windows.h>
#elif   defined(OS_LINUX)
    #include <dlfcn.h>
#endif

namespace cxx {
    namespace sys {

#if     defined(OS_WINDOWS)

        void* open_library(const char* name, int flag)
        {
            return LoadLibrary(name);
        }

        void* load_function(void* lib, const char* func)
        {
            return GetProcAddress((HINSTANCE)lib, func);
        }

        bool free_library(void* lib)
        {
            return FreeLibrary((HINSTANCE)lib) != 0;
        }

        std::string last_error(void* lib)
        {
            std::string errors;
            DWORD dw = GetLastError();
            LPVOID lpMsgBuf;
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL );
            errors = (LPTSTR)lpMsgBuf;
            return errors;
        }

#elif   defined(OS_LINUX)

        void* open_library(const char* name, int flag)
        {
            int oflag = 0;
            if(flag & dynlib::DLL_LAZY)     oflag |= RTLD_LAZY;
            if(flag & dynlib::DLL_NOW)      oflag |= RTLD_NOW;
            if(flag & dynlib::DLL_NOLOAD)   oflag |= RTLD_NOLOAD;
            if(flag & dynlib::DLL_DEEPBIND) oflag |= RTLD_DEEPBIND;
            if(flag & dynlib::DLL_LOCAL)    oflag |= RTLD_LOCAL;
            if(flag & dynlib::DLL_GLOBAL)   oflag |= RTLD_GLOBAL;
            return dlopen(name, oflag);
        }

        void* load_function(void* lib, const char* func)
        {
            dlerror();
            return dlsym(lib, func);
        }

        bool free_library(void* lib)
        {
            return dlclose(lib) == 0;
        }

        std::string last_error(void* lib)
        {
            const char* msg = dlerror();
            if(msg) {
                return msg;
            }
            return std::string();
        }

#else
    #error not implemented for unknown platform
#endif

        dynlib::dynlib() : handle_(NULL), refcnt_(new int(1))
        {
        }

        dynlib::dynlib(const char *name, int flag) : handle_(NULL), refcnt_(new int(1))
        {
            handle_ = open_library(name, flag);
        }

        dynlib::dynlib(const dynlib &rhs) : handle_(rhs.handle_), refcnt_(rhs.refcnt_)
        {
            ++*refcnt_;
        }

        dynlib& dynlib::operator =(const dynlib& rhs)
        {
            if(this == &rhs) {
                return *this;
            }

            if(--*refcnt_ < 1) {
                close();
                delete refcnt_;
                refcnt_ = 0;
            }
            handle_     = rhs.handle_;
            refcnt_     = rhs.refcnt_;
            ++*refcnt_;
            return *this;
        }

        bool dynlib::open(const char *name, int flag)
        {
            handle_ = open_library(name, flag);
            return handle_ != NULL;
        }

        void* dynlib::load(const char *func)
        {
            if(handle_) {
                return load_function(handle_, func);
            }
            return NULL;
        }

        bool dynlib::close()
        {
            if(handle_) {
                bool ret = (free_library(handle_) == 0);
                handle_ = NULL;
                return ret;
            }
            return false;
        }

        dynlib::~dynlib()
        {
            if(--*refcnt_ < 1) {
                close();
                delete refcnt_;
                refcnt_ = 0;
            }
        }

        std::string dynlib::error()
        {
            return last_error(handle_);
        }

    }
}
