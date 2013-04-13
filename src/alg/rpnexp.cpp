/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/alg/rpnexp.h"
#include <deque>
#include <sstream>

namespace cxx {
    namespace alg {

        namespace {
            enum TOKEN_TYPE {
                TT_NUMBERIC,
                TT_IDENTIES,
                TT_FUNCTION,
                TT_OPERATOR,
                TT_OTHERS
            };
        }

        rpn_error::rpn_error(int err, const char *msg, int pos)
            : std::runtime_error(msg), err_(err), pos_(pos)
        {
        }

        int rpn_error::err() const
        {
            return err_;
        }

        int rpn_error::pos() const
        {
            return pos_;
        }

        int rpn_express::is_operator(const char* input, size_t len) const
        {
            char f = 0;
            char s = 0;
            if(len > 0) {
                f = *input;
            }
            if(len > 1) {
                s = *(input + 1);
            }

            int ret = 0;
            switch(f) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '=':
            case '%':
            case '!':
                ret = 1;
                break;
            case '&':
                if(s == '&')
                    ret = 2;
                break;
            case '|':
                if(s == '|')
                    ret = 2;
                break;
            }
            return ret;
        }

        int rpn_express::is_function(const char* input, size_t len) const
        {
            const char* c = input;
            int         r = 0;
            if(isalpha(*c++)) {
                r = 1;
                while(*c) {
                    if(isalnum(*c) || *c == '_') {
                        ++r;
                        ++c;
                    }
                    else
                        break;
                }
            }

            int t = r;
            while(t < len) {
                if(isspace(*(input + t))) {
                    t++;
                    continue;
                }
                if(*(input + t) == '(')
                    return r;
                else
                    return 0;
                t++;
            }
        }

        int rpn_express::is_identies(const char *input, size_t len) const
        {
            const char* c = input;
            int         r = 0;
            if(isdigit(*c))
                return is_numberic(input, len);
            if(isalpha(*c)) {
                while(isalnum(*c) || *c == '_') {
                    ++r;
                    ++c;
                }
                int t = r;
                while(t < len) {
                    if(isspace(*(input + t))) {
                        t++;
                        continue;
                    }
                    if(*(input + t) == '(')
                        return 0;
                    if(*(input + t) == ',')
                        return r;
                    if(*(input + t) == ')')
                        return r;
                    if(is_operator(input + t, len - t))
                        return r;
                    t++;
                }
            }
            return r;
        }

        int rpn_express::is_numberic(const char* input, size_t len) const
        {
            const char* c = input;
            int         r = 0;
            while(isdigit(*c++)) {
                r++;
            }
            return r;
        }

        int rpn_express::num_args(const std::string &input) const
        {
            char c = input[0];
            char s = 0;
            if(input.size() > 1)
                s = input[1];
            switch(c) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
            case '=':
                return 2;
            case '&':
                if(s == '&')
                    return 2;
            case '|':
                if(s == '|')
                    return 2;
            case '!':
                return 1;
            }

            func_table::const_iterator it = function_.find(input);
            if(it != function_.end())
                return it->second;
            return 0;
        }

        void rpn_express::symbol(rpn_symbols *symbols)
        {
            resolver_ = symbols;
        }

        void rpn_express::setarg(const std::string &func, int nargs)
        {
            function_.insert(std::make_pair(func, nargs));
        }

        static int op_pred(const char* c)
        {
            switch(*c) {
            case '!':
                return 5;
            case '*':
            case '/':
            case '%':
                return 4;
            case '+':
            case '-':
                return 3;
            case '&':
            case '|':
                return 2;
            case '=':
                return 1;
            }
            return 0;
        }

        static bool op_left_assoc(const char* c)
        {
            switch(*c) {
            case '*':
            case '/':
            case '%':
            case '+':
            case '-':
            case '&':
            case '|':
                return true;
            case '=':
            case '!':
                return false;
            }
            return false;
        }

        rpn_express::expr_t rpn_express::parser(const std::string &input) const throw(rpn_error)
        {
            std::deque<std::pair<std::string, int > >    op_stack;
            std::string                 sc;
            rpn_express::expr_t         rc;

            const char* strbeg = input.c_str();
            const char* strpos = strbeg;
            const char* strend = strpos + input.length();
            while(strpos < strend) {
                int  r = 0;
                if(*strpos != ' ') {
                    if((r = is_identies(strpos, strend - strpos))) {
                        if(is_numberic(strpos, r)) {
                            rc.push_back(std::make_pair(std::string(strpos, strpos + r), TT_NUMBERIC));
                        }
                        else {
                            rc.push_back(std::make_pair(std::string(strpos, strpos + r), TT_IDENTIES));
                        }
                        strpos += r;
                    }
                    else if((r = is_function(strpos, strend - strpos))) {
                        op_stack.push_back(std::make_pair(std::string(strpos, strpos + r), TT_FUNCTION));
                        strpos += r;
                    }
                    else if(*strpos == ',') {
                        bool pe = false;
                        while(!op_stack.empty()) {
                            sc = op_stack.back().first;
                            if(sc == "(") {
                                pe = true;
                                break;
                            }
                            else {
                                rc.push_back(op_stack.back());
                                op_stack.pop_back();
                            }
                        }
                        strpos += 1;
                        if(!pe) {
                            throw rpn_error(rpn_error::RPN_MISMATCHED,
                                            "separator or parentheses mismatched",
                                            strpos - strbeg);
                        }
                    }
                    else if((r = is_operator(strpos, strend - strpos))) {
                        while(!op_stack.empty()) {
                            sc = op_stack.back().first;
                            if(is_operator(sc.c_str(), sc.length()) &&
                                    ((op_left_assoc(strpos) && (op_pred(strpos) <= op_pred(sc.c_str()))) ||
                                    (op_pred(strpos) < op_pred(sc.c_str())))) {
                                rc.push_back(op_stack.back());
                                op_stack.pop_back();
                            }
                            else
                                break;
                        }
                        op_stack.push_back(std::make_pair(std::string(strpos, strpos + r), TT_OPERATOR));
                        strpos += r;
                    }
                    else if(*strpos == '(') {
                        op_stack.push_back(std::make_pair("(", TT_OTHERS));
                        strpos += 1;
                    }
                    else if(*strpos == ')') {
                        bool pe = false;
                        while(!op_stack.empty()) {
                            sc = op_stack.back().first;
                            if(sc == "(") {
                                pe = true;
                                break;
                            }
                            else {
                                rc.push_back(op_stack.back());
                                op_stack.pop_back();
                            }
                        }
                        if(!pe) {
                            throw rpn_error(rpn_error::RPN_MISMATCHED,
                                            "parentheses mismatched",
                                            strpos - strbeg);
                        }
                        op_stack.pop_back();

                        if(!op_stack.empty()) {
                            sc = op_stack.back().first;
                            if((r = is_function(sc.c_str(), sc.length()))) {
                                rc.push_back(std::make_pair(sc, TT_FUNCTION));
                                op_stack.pop_back();
                            }
                        }
                        strpos += 1;
                    }
                    else {
                        throw rpn_error(rpn_error::RPN_INV_TOKEN, "unknown token",
                                        strpos - strbeg);
                    }
                }
                else {
                    strpos += 1;
                }
            }
            while(!op_stack.empty()) {
                sc = op_stack.back().first;
                if(sc == "(" || sc == ")") {
                    throw rpn_error(rpn_error::RPN_MISMATCHED, "parenthese mismatched",
                                    strpos - strbeg);
                }
                rc.push_back(op_stack.back());
                op_stack.pop_back();
            }
            return rc;
        }

        rpn_function* rpn_express::create(const rpn_express::expr_t &expr) const throw(rpn_error)
        {
            size_t size = expr.size();
            size_t curp = 0;
            std::deque<rpn_function* >  stack;
            while(curp < size) {
                const std::string&  c = expr[curp].first;
                int                 d = expr[curp].second;
                if(d == TT_NUMBERIC || d == TT_IDENTIES) {
                    rpn_function* fn = resolver_->resolve(c.c_str());
                    fn->done_arg();
                    stack.push_back(fn);
                }
                else if(d == TT_OPERATOR || d == TT_FUNCTION) {
                    int n = num_args(c);
                    if(stack.size() < n) {
                        std::ostringstream str;
                        str << "missing parameter " << stack.size() << "/" << n << " for function " << c;
                        throw rpn_error(rpn_error::RPN_MISSING_ARG, str.str().c_str(), 0);
                        return NULL;
                    }

                    rpn_function* fn = resolver_->resolve(c.c_str());
                    while(n > 0) {
                        rpn_function* arg = stack[stack.size() - n];
                        fn->push_arg(arg);
                        --n;
                    }
                    fn->done_arg();
                    n = num_args(c);
                    while(n > 0) {
                        stack.pop_back();
                        --n;
                    }
                    stack.push_back(fn);
                }
                curp++;
            }
            if(stack.size() == 1) {
                return stack.back();
            }
            return NULL;
        }

    }
}
