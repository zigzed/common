/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_SYS_UUID_H
#define CXX_SYS_UUID_H
#include "common/sys/uuid.h"
#include <cstddef>
#include <algorithm>
#include <iosfwd>

namespace cxx {
    namespace sys {

        struct uuid {
        public:
            typedef unsigned char           value_type;
            typedef unsigned char&          reference;
            typedef const unsigned char     const_reference;
            typedef unsigned char*          iterator;
            typedef const unsigned char*    const_iterator;
            typedef std::size_t             size_type;
            typedef std::ptrdiff_t          difference_type;
            enum variant_type {
                variant_unknown,
                variant_ncs,        //< NCS backward compatibility
                variant_rfc_4122,   //< defined RFC 4122
                variant_microsoft   //< microsoft compatibility
            };
            enum version_type {
                version_unknown,
                version_time_based,
                version_dce_security,
                version_name_based_md5,
                version_random_based,
                version_name_based_sha1
            };

            static size_type    size() { return 16; }

            uuid();
            iterator        begin() { return data_; }
            iterator        end()   { return data_ + size(); }
            const_iterator  begin() const { return data_; }
            const_iterator  end() const   { return data_ + size(); }

            bool            is_null() const;
            variant_type    variant() const;
            version_type    version() const;
            /** return the hash value of the UUID */
            inline std::size_t hash() const;
            void            swap(uuid& rhs);

            inline bool operator== (const uuid& rhs) const;
            inline bool operator!= (const uuid& rhs) const;
            inline bool operator<  (const uuid& rhs) const;
            inline bool operator>  (const uuid& rhs) const;
            inline bool operator<= (const uuid& rhs) const;
            inline bool operator>= (const uuid& rhs) const;
        private:
            unsigned char   data_[16];
        };

        std::ostream& operator<< (std::ostream& os, const uuid& u);
        std::istream& operator>> (std::istream& os, uuid& u);
        std::string   to_string(const uuid& rhs);

        /** generate a null UUID */
        struct nil_uuid {
            typedef uuid    result_type;

            uuid operator()() const;
        };

        /** generate a UUID with available system api */
        struct sys_uuid {
            typedef uuid    result_type;

            uuid operator()() const;
        };


        ////////////////////////////////////////////////////////////////////////
        inline bool uuid::operator ==(const uuid& rhs) const
        {
            return std::equal(begin(), end(), rhs.begin());
        }

        inline bool uuid::operator !=(const uuid& rhs) const
        {
            return !(*this == rhs);
        }

        inline bool uuid::operator <(const uuid& rhs) const
        {
            return std::lexicographical_compare(begin(), end(), rhs.begin(), rhs.end());
        }

        inline bool uuid::operator >(const uuid& rhs) const
        {
            return rhs < *this;
        }

        inline bool uuid::operator <=(const uuid& rhs) const
        {
            return !(*this > rhs);
        }

        inline bool uuid::operator >=(const uuid& rhs) const
        {
            return !(*this < rhs);
        }

        inline std::size_t uuid::hash() const
        {
            std::size_t seed = 0;
            for(const_iterator it = begin(); it != end(); ++it) {
                seed ^= static_cast<std::size_t >(*it) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }

    }
}

#endif
