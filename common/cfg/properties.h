/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_PROPERTIES_H
#define CXX_PROPERTIES_H
#include <map>
#include <string>
#include "common/config.h"
#include <stdint.h>

namespace cxx {
    namespace cfg {

        class value {
        public:
            value();
            explicit value(const std::string& val);
            /// 按照 bool 类型取值
            bool        as_bool(bool* def = NULL);
            /// 按照 int 类型取值
            int         as_int (int* def = NULL);
            /// 按照 int64 类型取值
            int64_t     as_int (int64_t* def = NULL);
            /// 从字符串转换出来
            std::string as_str (const std::string* def = NULL);
            /// 按照 hex 整数取值
            int         as_hex (int* def = NULL);
        private:
            std::string value_;
        };

        class properties {
        public:
            properties();
            std::string name() const;
            std::string getProperty(const std::string& key);
            std::string getPropertyWithDefault(const std::string& key, const std::string& def);
            int         getPropertyAsInt(const std::string& key);
            bool        getPropertyAsBool(const std::string& key);
            int         getPropertyAsIntWithDefault(const std::string& key, int def);
            void        setProperty(const std::string& key, const std::string& val);
            void        setProperty(const std::string& key, int val);
            void        setProperty(const std::string& key, bool val);
            void        setProperty(const std::string &key, int64_t val);

            void        load(const std::string& conf);
        private:
            bool        expandEnv(std::string& line) const;
            bool        isexpress(const std::string& line) const;
            std::string evaluate (const std::string& expr) const;

            properties(const properties& rhs);
            properties& operator= (const properties& rhs);

            void setProperty_i(bool extension, const std::string& key, const std::string& val);
            void parseLine(const std::string& line);
            void loadConfig();
            void appendval(const std::string& key, const std::string& val);
            std::string file_;
            std::map<std::string, std::string>  properties_;
            std::map<std::string, std::string>  extensions_;
        };

    }
}

#endif

