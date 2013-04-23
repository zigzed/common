/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_CON_FD_H
#define CXX_CON_FD_H
#include "common/con/coroutine.h"

namespace cxx {
    namespace con {

        class file_error : public std::runtime_error {
        public:
            file_error(int code, const std::string& msg);
            int code() const;
        private:
            int code_;
        };

        class filer {
        public:
            filer(coroutine* c, const char* name);
            off_t   seek(off_t offset, int from);
            int     load(char* data, int size, int ms = -1);
            int     save(const char* data, int size);
            void    close();
        private:
            int         fd_;
            coroutine*  cr_;
        };

    }
}

#endif
