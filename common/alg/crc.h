/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ALG_CRC_H
#define CXX_ALG_CRC_H
#include "common/config.h"
#include <cstddef>

namespace cxx {
    namespace alg {

        /** Longitudinal Redundary Check calculator.
         * LRC checksum  is equivalent to polynormial x^8+1
         * LRC checksum is compliance with ISO 1155
         * DATA: 02 30 30 31 23 03  LRC: 47
         */
        class lrc08 {
        public:
            typedef unsigned char   value_type;
            value_type  sum(const void* buf, std::size_t len) const;
            value_type  sum(const void *buf, const void* end) const;
        };


        class crc08 {
        public:
            typedef unsigned char   value_type;

            crc08();
            value_type  sum(const void* buf, std::size_t len) const;
            value_type  sum(const void* buf, const void* end) const;

            static value_type* table();
        };


        class crc16 {
        public:
            typedef unsigned short  value_type;

            explicit crc16();
            value_type  sum(const void* buf, std::size_t len) const;
            value_type  sum(const void* buf, const void* end) const;
        };

        class crc32 {
        public:
            typedef unsigned int    value_type;
            crc32();
            value_type  sum(const void* buf, std::size_t len) const;
            value_type  sum(const void* buf, const void* end) const;

            static value_type* table();
        };

    }
}

#endif
