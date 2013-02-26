/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/alg/crc.h"
#include <stdlib.h>

namespace cxx {
    namespace alg {

        lrc08::value_type lrc08::sum(const void *buf, const void *end) const
        {
            value_type sum = 0;
            const unsigned char* ptr = (const unsigned char* )buf;
            while(ptr < (const unsigned char*)end) {
                sum += *ptr++;
            }
            return (value_type)((char)-sum);
        }

        lrc08::value_type lrc08::sum(const void *buf, size_t len) const
        {
            value_type sum = 0;
            const unsigned char* ptr = (const unsigned char* )buf;
            while(len > 0) {
                sum += *ptr++;
                len--;
            }
            return (value_type)((char)-sum);
        }

        //--------------------------------------------------------------------//
        void cleanup_crc32_table()
        {
            static crc32::value_type* table = crc32::table();
            delete[] table;
        }

        crc32::crc32()
        {
            table();
        }

        crc32::value_type* crc32::table()
        {
            static value_type* crc_table = NULL;
            if(crc_table == NULL) {
                crc_table = new value_type[256];
                for(int i = 0; i < 256; ++i) {
                    value_type c = i;
                    for(int j = 0; j < 8; ++j) {
                        c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
                    }
                    crc_table[i] = c;
                }
                atexit(cleanup_crc32_table);
            }
            return crc_table;
        }

        crc32::value_type crc32::sum(const void *buf, const void *end) const
        {
            const unsigned char* ptr = (const unsigned char* )buf;
            std::ptrdiff_t       len = (const unsigned char* )end - ptr;
            return sum(ptr, len);
        }

        crc32::value_type crc32::sum(const void *buf, std::size_t len) const
        {
            value_type* crc_table = table();
            value_type           sum = 0xFFFFFFFF;
            const unsigned char* ptr = (const unsigned char* )buf;

            for(size_t i = 0; i < len; ++i) {
                sum = crc_table[(sum ^ ptr[i]) & 0xFF] ^ (sum >> 8);
            }
            return sum ^ 0xFFFFFFFF;
        }

    }
}
