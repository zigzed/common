/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/alg/variant.h"
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include "common/sys/error.h"

namespace cxx {
    namespace alg {

        static bool is_digit(const std::string& s)
        {
            for(std::string::const_iterator i = s.begin(); i != s.end(); ++i)
                if(!isdigit(*i))
                    return false;
            return true;
        }

        static bool converts(const std::string& s, int64_t& i)
        {
            const char* ptr = s.c_str();
            char* endptr = (char* )ptr + s.length();
            i = strtoll(ptr, &endptr, 0);
            int e = cxx::sys::err::get();
            if((e == ERANGE || (i == LONG_LONG_MAX || i == LONG_LONG_MIN))
                || (e != 0 && i == 0))
                return false;
            if(endptr == ptr)
                return false;
            return true;
        }

        static bool converts(const std::string& s, double& i)
        {
            const char* ptr = s.c_str();
            char* endptr = (char* )ptr + s.length();
            i = strtod(ptr, &endptr);
            int e = cxx::sys::err::get();
            if((e == ERANGE || (i == HUGE_VAL || i == -HUGE_VAL))
                || (e != 0 && i == 0))
                return false;
            if(endptr == ptr)
                return false;
            return true;
        }

        static bool converts(int64_t i, std::string& s)
        {
            char temp[256];
            sprintf(temp, "%lld", i);
            s = temp;
            return true;
        }

        static bool converts(double d, std::string& s)
        {
            char temp[256];
            sprintf(temp, "%f", d);
            s = temp;
            return true;
        }

        variant_t::variant_t() : type_(Nil)
        {
        }

        variant_t::variant_t(int i) : type_(Number)
        {
            data_.iValue = i;
        }

        variant_t::variant_t(int64_t i) : type_(Number)
        {
            data_.iValue = i;
        }

        variant_t::variant_t(double d) : type_(Double)
        {
            data_.dValue = d;
        }

        variant_t::variant_t(const std::string &s) : type_(String)
        {
            str_ = s;
        }

        variant_t::meta_type variant_t::get_mtype() const
        {
            return type_;
        }

        int64_t variant_t::as_number() const
        {
            int64_t r = 0;
            switch(type_) {
            case Nil:
                assert(false);
                return 0;
            case Number:
                return data_.iValue;
            case String:
                if(converts(str_, r))
                    return r;
                assert(false);
                return 0;
            case Double:
                return data_.dValue;
            default:
                assert(false);
                return 0;
            }
        }

        double variant_t::as_double() const
        {
            double r = 0;
            switch(type_) {
            case Nil:
                assert(false);
                return 0;
            case Number:
                return data_.iValue;
            case String:
                if(converts(str_, r))
                    return r;
                assert(false);
                return 0;
            case Double:
                return data_.dValue;
            default:
                assert(false);
                return 0;
            }
        }

        std::string variant_t::as_string() const
        {
            std::string r;
            switch(type_) {
            case Nil:
                assert(false);
                return r;
            case Number:
                if(converts(data_.iValue, r))
                    return r;
                assert(false);
                return r;
            case Double:
                if(converts(data_.dValue, r))
                    return r;
                assert(false);
                return r;
            case String:
                return str_;
            default:
                assert(false);
                return r;
            }
        }

        variant_t operator+(const variant_t& lhs, const variant_t& rhs)
        {
            switch(lhs.get_mtype()) {
            case variant_t::Nil:
                return variant_t();
            case variant_t::Number:
                switch(rhs.get_mtype()) {
                case variant_t::Nil:
                    return variant_t();
                case variant_t::Number:
                    return lhs.data_.iValue + rhs.data_.iValue;
                case variant_t::Double:
                    return lhs.data_.iValue + rhs.data_.dValue;
                case variant_t::String:
                    {
                    int64_t i;
                    double  d;
                    if(converts(rhs.str_, i)) {
                        return lhs.data_.iValue + i;
                    }
                    else if(converts(rhs.str_, d)) {
                        return lhs.data_.iValue + d;
                    }
                    else
                        return variant_t();
                    }
                default:
                    return variant_t();
                }
            case variant_t::Double:
                switch(rhs.get_mtype()) {
                case variant_t::Nil:
                    return variant_t();
                case variant_t::Number:
                    return lhs.data_.dValue + rhs.data_.iValue;
                case variant_t::Double:
                    return lhs.data_.dValue + rhs.data_.dValue;
                case variant_t::String:
                    {
                    int64_t i;
                    double  d;
                    if(converts(rhs.str_, i)) {
                        return lhs.data_.dValue + i;
                    }
                    else if(converts(rhs.str_, d)) {
                        return lhs.data_.dValue + d;
                    }
                    else
                        return variant_t();
                    }
                default:
                    return variant_t();
                }
            case variant_t::String:
            default:
                return variant_t();
            }
        }

        variant_t operator-(const variant_t& lhs, const variant_t& rhs)
        {
            switch(lhs.get_mtype()) {
            case variant_t::Nil:
                return variant_t();
            case variant_t::Number:
                switch(rhs.get_mtype()) {
                case variant_t::Nil:
                    return variant_t();
                case variant_t::Number:
                    return lhs.data_.iValue - rhs.data_.iValue;
                case variant_t::Double:
                    return lhs.data_.iValue - rhs.data_.dValue;
                case variant_t::String:
                    {
                    int64_t i;
                    double  d;
                    if(converts(rhs.str_, i)) {
                        return lhs.data_.iValue - i;
                    }
                    else if(converts(rhs.str_, d)) {
                        return lhs.data_.iValue - d;
                    }
                    else
                        return variant_t();
                    }
                default:
                    return variant_t();
                }
            case variant_t::Double:
                switch(rhs.get_mtype()) {
                case variant_t::Nil:
                    return variant_t();
                case variant_t::Number:
                    return lhs.data_.dValue - rhs.data_.iValue;
                case variant_t::Double:
                    return lhs.data_.dValue - rhs.data_.dValue;
                case variant_t::String:
                    {
                    int64_t i;
                    double  d;
                    if(converts(rhs.str_, i)) {
                        return lhs.data_.dValue - i;
                    }
                    else if(converts(rhs.str_, d)) {
                        return lhs.data_.dValue - d;
                    }
                    else
                        return variant_t();
                    }
                default:
                    return variant_t();
                }
            case variant_t::String:
            default:
                return variant_t();
            }
        }

        variant_t operator*(const variant_t& lhs, const variant_t& rhs)
        {
            switch(lhs.get_mtype()) {
            case variant_t::Nil:
                return variant_t();
            case variant_t::Number:
                switch(rhs.get_mtype()) {
                case variant_t::Nil:
                    return variant_t();
                case variant_t::Number:
                    return lhs.data_.iValue * rhs.data_.iValue;
                case variant_t::Double:
                    return lhs.data_.iValue * rhs.data_.dValue;
                case variant_t::String:
                    {
                    int64_t i;
                    double  d;
                    if(converts(rhs.str_, i)) {
                        return lhs.data_.iValue * i;
                    }
                    else if(converts(rhs.str_, d)) {
                        return lhs.data_.iValue * d;
                    }
                    else
                        return variant_t();
                    }
                default:
                    return variant_t();
                }
            case variant_t::Double:
                switch(rhs.get_mtype()) {
                case variant_t::Nil:
                    return variant_t();
                case variant_t::Number:
                    return lhs.data_.dValue * rhs.data_.iValue;
                case variant_t::Double:
                    return lhs.data_.dValue * rhs.data_.dValue;
                case variant_t::String:
                    {
                    int64_t i;
                    double  d;
                    if(converts(rhs.str_, i)) {
                        return lhs.data_.dValue * i;
                    }
                    else if(converts(rhs.str_, d)) {
                        return lhs.data_.dValue * d;
                    }
                    else
                        return variant_t();
                    }
                default:
                    return variant_t();
                }
            case variant_t::String:
            default:
                return variant_t();
            }
        }

        variant_t operator/(const variant_t& lhs, const variant_t& rhs)
        {
            switch(lhs.get_mtype()) {
            case variant_t::Nil:
                return variant_t();
            case variant_t::Number:
                switch(rhs.get_mtype()) {
                case variant_t::Nil:
                    return variant_t();
                case variant_t::Number:
                    if(rhs.data_.iValue != 0)
                        return lhs.data_.iValue / rhs.data_.iValue;
                    return variant_t();
                case variant_t::Double:
                    if(rhs.data_.dValue != 0)
                        return lhs.data_.iValue / rhs.data_.dValue;
                    return variant_t();
                case variant_t::String:
                    {
                    int64_t i;
                    double  d;
                    if(converts(rhs.str_, i)) {
                        if(i != 0)
                            return lhs.data_.iValue / i;
                        return variant_t();
                    }
                    else if(converts(rhs.str_, d)) {
                        if(d != 0)
                            return lhs.data_.iValue / d;
                        return variant_t();
                    }
                    else
                        return variant_t();
                    }
                default:
                    return variant_t();
                }
            case variant_t::Double:
                switch(rhs.get_mtype()) {
                case variant_t::Nil:
                    return variant_t();
                case variant_t::Number:
                    if(rhs.data_.iValue != 0)
                        return lhs.data_.dValue / rhs.data_.iValue;
                    return variant_t();
                case variant_t::Double:
                    if(rhs.data_.dValue != 0)
                        return lhs.data_.dValue / rhs.data_.dValue;
                    return variant_t();
                case variant_t::String:
                    {
                    int64_t i;
                    double  d;
                    if(converts(rhs.str_, i)) {
                        if(i != 0)
                            return lhs.data_.dValue / i;
                        return variant_t();
                    }
                    else if(converts(rhs.str_, d)) {
                        if(d != 0)
                            return lhs.data_.dValue / d;
                        return variant_t();
                    }
                    else
                        return variant_t();
                    }
                default:
                    return variant_t();
                }
            case variant_t::String:
            default:
                return variant_t();
            }
        }

        std::ostream& operator<< (std::ostream& os, const variant_t& v)
        {
            switch(v.type_) {
            case variant_t::Nil:
                os << "Nil";
                break;
            case variant_t::Number:
                os << v.data_.iValue;
                break;
            case variant_t::Double:
                os << v.data_.dValue;
                break;
            case variant_t::String:
                os << v.str_;
                break;
            default:
                os << "variant_t";
            }
        }

    }
}
