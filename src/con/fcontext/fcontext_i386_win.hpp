
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef CXX_CON_DETAIL_FCONTEXT_I386H
#define CXX_CON_DETAIL_FCONTEXT_I386H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <cstddef>

namespace cxx {
    namespace con {

        extern "C" {

            struct stack_t
            {
                void    *   sp;
                std::size_t size;
                void    *   limit;

                stack_t() :
                    sp( 0), size( 0), limit( 0)
                {}
            };

            struct fp_t
            {
                uint32_t     fc_freg[2];

                fp_t() :
                    fc_freg()
                {}
            };

            struct fcontext_t
            {
                uint32_t     fc_greg[6];
                stack_t             fc_stack;
                void            *   fc_excpt_lst;
                void            *   fc_local_storage;
                fp_t                fc_fp;

                fcontext_t() :
                    fc_greg(),
                    fc_stack(),
                    fc_excpt_lst( 0),
                    fc_local_storage( 0),
                    fc_fp()
                {}
            };

        }

    }
}


#endif

