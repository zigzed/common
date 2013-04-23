/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ALG_RPNEXP_H
#define CXX_ALG_RPNEXP_H
#include <vector>
#include <string>
#include <map>
#include <stdexcept>

namespace cxx {
    namespace alg {

        class rpn_error : public std::runtime_error {
        public:
            enum CODE {
                RPN_OK,
                RPN_MISMATCHED,
                RPN_INV_TOKEN,
                RPN_MISSING_ARG
            };

            rpn_error(int code, const char* msg, int pos);
            int err() const;
            int pos() const;
        private:
            int err_;
            int pos_;
        };

        class rpn_function {
        public:
            typedef std::vector<rpn_function* > arg_t;

            virtual ~rpn_function() {}
        };

        class rpn_symbols {
        public:
            virtual ~rpn_symbols() {}
            virtual rpn_function* resolve(const char* name, const rpn_function::arg_t& args) const = 0;
        };

        class rpn_express {
        public:
            /// 设置 rpn_express 符号查询接口。符号查询包括变量、函数、表达式
            void   lookup(rpn_symbols* symbols);
            /// 设置函数参数个数
            void   setarg(const std::string& func, int nargs);

            rpn_function* create(const std::string& input) const throw(rpn_error);
        private:
            int     is_operator(const char* input, size_t len) const;
            int     is_function(const char* input, size_t len) const;
            int     is_numberic(const char* input, size_t len) const;
            int     is_identies(const char* input, size_t len) const;
            int     num_args(const std::string& input) const;

            typedef std::vector<std::pair<std::string, int > >   expr_t;
            /// 将表达式解析通过 shunting yard 算法解析
            expr_t parser(const std::string& input) const throw(rpn_error);
            /// 将 shunting yard 解析的结果通过符号表查询创建为表达式
            rpn_function *create(const expr_t& exp) const throw(rpn_error);

            typedef std::map<std::string, int >    func_table;
            func_table      function_;
            rpn_symbols*    resolver_;
        };

    }
}

#endif
