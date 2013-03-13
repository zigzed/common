
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef CXX_CON_DETAIL_FCONTEXT_X86_64_H
#define CXX_CON_DETAIL_FCONTEXT_X86_64_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <cstddef>

namespace cxx {
    namespace con {

        extern "C" {

            struct stack_t {
                void    *   sp;
                std::size_t size;
                void    *   limit;

                stack_t() :
                    sp( 0), size( 0), limit( 0)
                {}
            };

            struct fcontext_t {
                uint64_t     fc_greg[10];
                stack_t      fc_stack;
                void*        fc_local_storage;
                uint64_t     fc_fp[24];

                fcontext_t() :
                    fc_greg(),
                    fc_stack(),
                    fc_local_storage( 0),
                    fc_fp()
                {}
            };

        }

    }
}


#endif
