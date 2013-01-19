/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_DLL_DYNLIB_H
#define CXX_DLL_DYNLIB_H
#include <string>

namespace cxx {
    namespace sys {

        class dynlib {
        public:
            enum DL_FLAG {
                DLL_LAZY    = 0x0001,
                DLL_NOW     = 0x0002,
                DLL_NOLOAD  = 0x0004,
                DLL_DEEPBIND= 0x0008,
                DLL_LOCAL   = 0x0100,
                DLL_GLOBAL  = 0x0200,
                DLL_NONDEL  = 0x1000
            };

            dynlib();
            dynlib(const char* name, int flag = DLL_NOW | DLL_GLOBAL);
            dynlib(const dynlib& rhs);
            dynlib& operator= (const dynlib& rhs);

            ~dynlib();
            bool    open(const char* name, int flag = DLL_NOW | DLL_GLOBAL);
            void*   load(const char* func);
            std::string error();
        private:
            bool    close();
            void*   handle_;
            int*    refcnt_;
        };

    }
}

#endif
