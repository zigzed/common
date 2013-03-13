
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef CXX_CON_DETAIL_FCONTEXT_I386H
#define CXX_CON_DETAIL_FCONTEXT_I386H

#include <cstddef>


namespace cxx {
    namespace con {

        extern "C" {

            struct stack_t
            {
                void            *   sp;
                std::size_t         size;

                stack_t() :
                    sp( 0), size( 0)
                {}
            };

            struct fcontext_t
            {
                uint32_t     fc_greg[6];
                stack_t             fc_stack;
                uint32_t     fc_freg[2];

                fcontext_t() :
                    fc_greg(),
                    fc_stack(),
                    fc_freg()
                {}
            };

        }

    }
}


#endif
