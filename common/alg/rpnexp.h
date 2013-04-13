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
            virtual void push_arg(rpn_function* fn) = 0;
            virtual void done_arg() = 0;
            virtual ~rpn_function() {}
        };

        class rpn_symbols {
        public:
            virtual ~rpn_symbols() {}
            virtual rpn_function* resolve(const char* name) const = 0;
        };

        class rpn_express {
        public:
            typedef std::vector<std::pair<std::string, int > >   expr_t;
            /// ���� rpn_express ���Ų�ѯ�ӿڡ����Ų�ѯ�������������������ʽ
            void   symbol(rpn_symbols* symbols);
            /// ���ú�����������
            void   setarg(const std::string& func, int nargs);
            /// �����ʽ����ͨ�� shunting yard �㷨����
            expr_t parser(const std::string& input) const throw(rpn_error);
            /// �� shunting yard �����Ľ��ͨ�����ű��ѯ����Ϊ���ʽ
            rpn_function *create(const expr_t& exp) const throw(rpn_error);
        private:
            int     is_operator(const char* input, size_t len) const;
            int     is_function(const char* input, size_t len) const;
            int     is_numberic(const char* input, size_t len) const;
            int     is_identies(const char* input, size_t len) const;
            int     num_args(const std::string& input) const;
            bool    is_ident(char c);

            typedef std::map<std::string, int >    func_table;
            func_table      function_;
            rpn_symbols*    resolver_;
        };

    }
}

#endif
