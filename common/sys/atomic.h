/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ATOMIC_H
#define CXX_ATOMIC_H
#include "common/config.h"

namespace cxx {
    namespace sys {

        class atomic_t {
        public:
            explicit atomic_t(long val);
            long operator++();
            long operator--();
            operator long() const;
        private:
            atomic_t(const atomic_t& rhs);
            atomic_t& operator= (const atomic_t& rhs);
            mutable long value_;
        };
    }

}

#endif
