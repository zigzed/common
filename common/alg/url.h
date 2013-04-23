/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ALG_URL_H
#define CXX_ALG_URL_H
#include "common/config.h"
#include <string>

namespace cxx {
    namespace alg {

        class url {
        public:
            static std::string encode(const std::string& u);
            static std::string decode(const std::string& u);
        };

    }
}

#endif
