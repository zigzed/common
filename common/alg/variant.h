/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ALG_VARIANT_H
#define CXX_ALG_VARIANT_H
#include "common/config.h"
#include <stdint.h>
#include <iosfwd>
#include <string>

namespace cxx {
    namespace alg {

        class variant_t {
        public:
            enum    meta_type { Nil, Number, String, Double };

            variant_t();
            variant_t(int i);
            variant_t(int64_t i);
            variant_t(double d);
            variant_t(const std::string& s);

            meta_type   get_mtype() const;
            int64_t     as_number() const;
            std::string as_string() const;
            double      as_double() const;
            bool        empty() const;

            bool operator== (const variant_t& rhs) const;
            bool operator!= (const variant_t& rhs) const;
            friend variant_t operator+(const variant_t& lhs, const variant_t& rhs);
            friend variant_t operator-(const variant_t& lhs, const variant_t& rhs);
            friend variant_t operator*(const variant_t& lhs, const variant_t& rhs);
            friend variant_t operator/(const variant_t& lhs, const variant_t& rhs);
            friend std::ostream& operator<<(std::ostream& os, const variant_t& rhs);
        private:
            union   meta_data {
                int64_t iValue;
                double  dValue;
            };
            meta_type   type_;
            meta_data   data_;
            std::string str_;
        };

    }
}

#endif

