/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "common/cfg/properties.h"
#include "common/alg/expression.h"
#include "common/sys/filesystem.h"
#include "common/str/tokenizer.h"

namespace cxx {
    namespace cfg {

        properties::properties()
        {
        }

        std::string properties::name() const
        {
            return file_;
        }

        std::string properties::getProperty(const std::string& key)
        {
            // 直接赋值的配置和根据变量展开的配置的优先级不同。前者拥有更高的优先级，这样可以
            // 支持普遍的配置中扩展特殊的配置。所以查找的顺序是先按照前者查询，然后按照变量
            // 展开查询
            std::map<std::string, std::string>::const_iterator it = properties_.find(key);
            if(it != properties_.end())
                return it->second;
            else {
                std::map<std::string, std::string>::const_iterator it = extensions_.find(key);
                if(it != extensions_.end())
                    return it->second;
                return std::string();
            }
        }

        std::string properties::getPropertyWithDefault(const std::string& key, const std::string& def)
        {
            std::string result = getProperty(key);
            if(result.empty()) {
                result = def;
            }
            return result;
        }

        int properties::getPropertyAsInt(const std::string& key)
        {
            return getPropertyAsIntWithDefault(key, 0);
        }

        static bool casecmp(const char* lhs, const char* rhs)
        {
            const char* iptr = lhs;
            const char* optr = rhs;
            int         ilen = strlen(lhs);
            int         olen = strlen(rhs);
            if(ilen != olen) {
                return false;
            }
            for(int i = 0; i < ilen; ++i) {
                if(tolower(*iptr) != tolower(*optr)) {
                    return false;
                }
            }
            return true;
        }

        bool properties::getPropertyAsBool(const std::string &key)
        {
            std::string result = getProperty(key);
            if(casecmp(result.c_str(), "true") ||
                    casecmp(result.c_str(), "yes") ||
                    casecmp(result.c_str(), "on") ||
                    casecmp(result.c_str(), "1")) {
                return true;
            }
            return false;
        }

        int properties::getPropertyAsIntWithDefault(const std::string& key, int def)
        {
            std::map<std::string, std::string>::const_iterator it = properties_.find(key);
            if(it != properties_.end()) {
                std::istringstream v(it->second);
                if(!(v >> def) || !v.eof())
                    return 0;
            }

            return def;
        }

        void properties::load(const std::string& conf)
        {
            cxx::sys::Path  path(conf.c_str());
            file_ = path.name();
            std::ifstream ifs(file_.c_str());
            if(!ifs) {
                std::ostringstream strmsg;
                strmsg << "requested configuration file \'" << file_ << "\' not found";
                    throw std::runtime_error(strmsg.str());
            }

            std::string line;
            while(std::getline(ifs, line)) {
                parseLine(line);
            }
        }

        // 对环境变量进行展开
        // 如果存在 ${*} 的字符串，那么将中间部分取出来和环境变量进行匹配，如果环境变量中
        // 有定义，则展开，否则不展开
        bool properties::expandEnv(std::string &line) const
        {
            bool    replaced = false;
            size_t  position = 0;
            while(true) {
                int         found = 0;
                std::string symbol;
                std::string::size_type beg = line.find("${", position);
                if(beg != std::string::npos && beg + 1 < line.length()) {
                    found++;
                }
                std::string::size_type end = line.find_first_of('}', beg);
                if(end != std::string::npos) {
                    found++;
                }

                if(found < 2) {
                    return replaced;
                }

                symbol = line.substr(beg + 2, end - beg - 2);
                position = beg + 1;
                char* env = getenv(symbol.c_str());
                if(env == NULL) {
                    fprintf(stderr, "properties::environment variable \'%s\' not found\n",
                            symbol.c_str());
                }
                else {
                    line.replace(beg, end - beg + 1, env);
                    replaced = true;
                }
            }
            return replaced;
        }

        static bool isoperand(int c)
        {
            switch(c) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '(':
            case ')':
                return true;
            default:
                return false;
            }
        }

        bool properties::isexpress(const std::string &line) const
        {
            for(size_t i = 0; i < line.size(); ++i) {
                char c = line[i];
                if(!isdigit(c) && !isspace(c) && !isoperand(c)) {
                    return false;
                }
            }
            return true;
        }

        std::string properties::evaluate(const std::string& expr) const
        {
            if(isexpress(expr)) {
                try {
                    cxx::alg::expression math;
                    int result = math.evaluate(expr);
                    std::ostringstream ostr;
                    ostr << result;
                    return ostr.str();
                }
                catch(const std::string& e) {
                    fprintf(stderr, "properties:expression \'%s\' failed: %s\n",
                            expr.c_str(), e.c_str());
                }
            }
            return expr;
        }

        void properties::parseLine(const std::string& line)
        {
            const std::string delim = " \t\r\n";
            std::string s = line;

            bool extension = expandEnv(s);

            std::string::size_type idx = s.find('#');
            if(idx != std::string::npos)
                s.erase(idx);

            idx = s.find_last_not_of(delim);
            if(idx != std::string::npos && idx + 1 < s.length())
                s.erase(idx + 1);

            std::string::size_type beg = s.find_first_not_of(delim);
            if(beg == std::string::npos)
                return;

            std::string::size_type end = s.find_first_of(delim + "=", beg);
            if(end == std::string::npos)
                return;

            std::string key = s.substr(beg, end - beg);

            end = s.find('=', end);
            if(end == std::string::npos)
                return;
            ++end;

            std::string val;
            beg = s.find_first_not_of(delim, end);
            if(beg != std::string::npos) {
                end = s.length();
                val = s.substr(beg, end - beg);
            }

            if(key == "INCLUDE" || key == "include") {
                cxx::str::tokenizer token(val, " ,;\t", "\"", "\\");
                for(size_t i = 0; i < token.size(); ++i) {
                    try {
                        this->load(token[i].c_str());
                    }
                    catch(const std::runtime_error& ) {
                        fprintf(stderr, "properties:included config file \'%s\' not exist\n",
                                token[i].c_str());
                    }
                }
            }
            else {
                setProperty_i(extension, key, val);
            }
        }

        void properties::setProperty(const std::string& key, const std::string& val)
        {
            if(!key.empty()) {
                if(!val.empty()) {
                    properties_[key] = val;
                    appendval(key, val);
                }
                else
                    properties_.erase(key);
            }
        }

        void properties::setProperty(const std::string &key, int val)
        {
            std::ostringstream ostr;
            ostr << val;
            setProperty(key, ostr.str());
        }

		void properties::setProperty(const std::string &key, int64_t val)
        {
            std::ostringstream ostr;
            ostr << val;
            setProperty(key, ostr.str());
        }


        void properties::setProperty(const std::string &key, bool val)
        {
            if(val) {
                setProperty(key, "true");
            }
            else {
                setProperty(key, "false");
            }
        }

        void properties::appendval(const std::string &key, const std::string &val)
        {
            if(!file_.empty()) {
                std::ofstream ofs(file_.c_str(), std::ios_base::app | std::ios_base::out);
                ofs << key << " = " << val << "\n";
                ofs.close();
            }
        }

        void properties::setProperty_i(bool extension, const std::string& key, const std::string& val)
        {
            if(!key.empty()) {
                if(!val.empty()) {
                    std::string result = evaluate(val);
                    if(extension) {
                        extensions_[key] = result;
                    }
                    else {
                        properties_[key] = result;
                    }
                }
                else {
                    if(extension) {
                        extensions_.erase(key);
                    }
                    else {
                        properties_.erase(key);
                    }
                }
            }
        }

    }
}

/**
 * $Id: properties.cpp,v 1.1.1.1 2006/12/08 10:38:15 wilbur Exp $
 * $Log: properties.cpp,v $
 * Revision 1.1.1.1  2006/12/08 10:38:15  wilbur
 * no message
 *
 * Revision 1.3  2005/06/27 10:41:42  wilbur
 * throw exceptions from std::runtime_error
 *
 * Revision 1.2  2004/10/10 02:24:55  wilbur
 * 1. adjust including base directory, so all of the cxxlib(s) should included as cxxlib/xxxxx
 * 2. adjust all of the WIN32/_WIN32 definition to WIN32 only
 *
 * Revision 1.1.1.1  2004/09/23 07:43:07  wilbur
 * initialize import the source
 *
 */

