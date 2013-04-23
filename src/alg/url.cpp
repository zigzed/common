/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/alg/url.h"
#include <ctype.h>  // for isascii, idxdigit
#include <stdlib.h> // for strtol

namespace cxx {
    namespace alg {

        std::string url::encode(const std::string &u)
        {
            static const char*  hex = "0123456789ABCDEF";

            std::string result;
            result.reserve(u.size() * 3);
            for(size_t i = 0; i < u.size(); ++i) {
                unsigned char cc = u[i];
                if(isascii(cc)) {
                    if(cc == ' ')
                        result += "%20";
                    else
                        result += cc;
                }
                else {
                    result += "%";
                    result += hex[cc / 16];
                    result += hex[cc % 16];
                }
            }
            return result;
        }

        std::string url::decode(const std::string &u)
        {
            std::string result;
            result.reserve(u.size());

            for(size_t i = 0; i < u.size(); ++i) {
                switch(u[i]) {
                case '%':
                    if(i + 2 < u.size() && isxdigit(u[i + 1]) && isxdigit(u[i + 2])) {
                        std::string temp = u.substr(i + 1, 2);
                        long hex = strtol(temp.c_str(), 0, 16);
                        result += char(hex);
                        i += 2;
                    }
                    else
                        result += '%';
                    break;
                default:
                    result += u[i];
                    break;
                }
            }
            return result;
        }

    }
}
