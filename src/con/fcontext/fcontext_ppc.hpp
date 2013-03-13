
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef CXX_CON_DETAIL_FCONTEXT_PPC_H
#define CXX_CON_DETAIL_FCONTEXT_PPC_H

#include <cstddef>

namespace cxx {
    namespace con {

        extern "C" {


            struct stack_t
            {
                void    *   sp;
                std::size_t size;

                stack_t() :
                    sp( 0), size( 0)
                {}
            };

            struct fp_t
            {
                uint64_t     fc_freg[19];

                fp_t() :
                    fc_freg()
                {}
            };

            struct fcontext_t
            {
# if defined(__powerpc64__)
                uint64_t     fc_greg[23];
# else
                uint32_t     fc_greg[23];
# endif
                stack_t             fc_stack;
                fp_t                fc_fp;

                fcontext_t() :
                    fc_greg(),
                    fc_stack(),
                    fc_fp()
                {}
            };

        }

    }
}


#endif
