/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/sys/uuid.h"
#include "common/sys/error.h"
#include <string.h> // for memset
#include <iomanip>
#if defined(OS_LINUX)
    #include <unistd.h>
    #include <fcntl.h>
#elif defined(OS_WINDOWS)
    #include <rpc.h>
    #pragma comment(lib, "rpcrt4.lib")
#endif

namespace cxx {
    namespace sys {

        uuid::uuid()
        {
            memset(&data_, 0, size());
        }

        bool uuid::is_null() const
        {
            for(size_t i = 0; i < size(); ++i) {
                if(data_[i] != 0U)
                    return false;
            }
            return true;
        }

        uuid::variant_type uuid::variant() const
        {
            // variant is stored in octet 7
            // which is index 8, since index is count backwards
            unsigned char octet = data_[8];
            if((octet & 0x80) == 0x00)
                return variant_ncs;
            else if((octet & 0xC0) == 0x80)
                return variant_rfc_4122;
            else if((octet & 0xE0) == 0xC0)
                return variant_microsoft;
            ENFORCE(false)("unknown UUID variant");
            return variant_unknown;
        }

        uuid::version_type uuid::version() const
        {
            // version is stored in octet 9
            // which is index 6, since index is count backwards
            unsigned char octet = data_[6];
            if((octet & 0xF0) == 0x10)
                return version_time_based;
            else if((octet & 0xF0) == 0x20)
                return version_dce_security;
            else if((octet & 0xF0) == 0x30)
                return version_name_based_md5;
            else if((octet & 0xF0) == 0x40)
                return version_random_based;
            else if((octet & 0xF0) == 0x50)
                return version_name_based_sha1;
            else
                return version_unknown;
        }

        void uuid::swap(uuid &rhs)
        {
            std::swap_ranges(begin(), end(), rhs.begin());
        }

        std::ostream& operator<< (std::ostream& os, const uuid& u)
        {
            os << to_string(u);
            return os;
        }

        std::istream& operator>> (std::istream& is, uuid& u)
        {
            unsigned char data[16];
            const char* xdigit = "0123456789ABCDEF";
            char c;
            for(std::size_t i = 0; i < u.size() && is; ++i) {
                is >> c;
                c = std::toupper(c);
                const char* f = std::find(xdigit, xdigit + 16, c);
                if(f == xdigit + 16) {
                    is.setstate(std::ios_base::failbit);
                    break;
                }
                unsigned char byte = static_cast<unsigned char >(std::distance(xdigit, f));

                is >> c;
                c = std::toupper(c);
                f = std::find(xdigit, xdigit + 16, c);
                if(f == xdigit + 16) {
                    is.setstate(std::ios_base::failbit);
                    break;
                }

                byte <<= 4;
                byte |= static_cast<unsigned char >(std::distance(xdigit, f));
                data[i] = byte;

                if(is) {
                    if(i == 3 || i == 5 || i == 7 || i == 9) {
                        is >> c;
                        if(c != is.widen('-'))
                            is.setstate(std::ios_base::failbit);
                    }
                }
            }
            if(is) {
                std::copy(data, data + 16, u.begin());
            }
        }

        static char to_char(size_t i)
        {
            if(i <= 9)
                return '0' + i;
            else
                return 'a' + (i - 10);
        }

        std::string to_string(const uuid &u)
        {
            std::string result;
            result.reserve(36);

            std::size_t i = 0;
            for(uuid::const_iterator it = u.begin(); it != u.end(); ++it, ++i) {
                size_t hi = ((*it) >> 4) & 0x0F;
                size_t lo = ((*it) & 0x0F);
                result += to_char(hi);
                result += to_char(lo);

                if(i == 3 || i == 5 || i == 7 || i == 9) {
                    result += "-";
                }
            }
            return result;
        }

        ////////////////////////////////////////////////////////////////////////
        uuid nil_uuid::operator ()() const
        {
            return uuid();
        }

        struct uuid_file {
        public:
            uuid_file() : fd_(-1) {
                fd_ = open("/dev/urandom", O_RDONLY);
            }
            ~uuid_file() {
                if(fd_ != -1) {
                    close(fd_);
                }
            }
            int fd() const { return fd_; }
        private:
            int fd_;
        };

        uuid sys_uuid::operator ()() const
        {
            uuid        u;
#if defined(OS_LINUX)
            uuid_file   uf;
            if(uf.fd() == -1) {
                ENFORCE(uf.fd() != -1)("can't open random device to generate UUID");
                return u;
            }

            ssize_t bytes = 0;
            ssize_t reads = 0;
            while(reads <= 20 && bytes < uuid::size()) {
                ssize_t c = read(uf.fd(), u.begin() + bytes, uuid::size() - bytes);
                if(c == -1) {
                    ENFORCE(c != -1)(c)(cxx::sys::err::get())("reading random device failed");
                    return u;
                }
                else {
                    bytes += c;
                    ++reads;
                }
            }

            if(bytes != uuid::size()) {
                ENFORCE(bytes == uuid::size())(reads)("retry to read random device failed");
                return u;
            }

            // set variant
            *(u.begin() + 8) &= 0xBF;
            *(u.begin() + 8) |= 0x80;

            // set version
            *(u.begin() + 6) &= 0x4F;
            *(u.begin() + 6) |= 0x40;

            return u;
#elif defined(OS_WINDOWS)
            uuid    u;
            ENFORCE(sizeof(UUID) == sizeof(u))(sizeof(UUID))(sizeof(u))("UUID size mismatched");
            ::UuidCreate((UUID* )&u);
            return u;
#endif
        }

    }
}
