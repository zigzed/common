/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/alg/expression.h"
#include <sstream>

namespace cxx {
    namespace alg {

        expression::expression() : externals_(NULL), errortype_(expression::ET_EXCEPTION)
        {
        }

        void expression::report(const std::string &msg) const
        {
            if(errortype_ == ET_EXCEPTION) {
                throw std::string(msg);
            }
            errorinfo_ = msg;
        }

        void expression::err_type(ErrorType type)
        {
            errortype_ = type;
        }

        void expression::external(resolver *reso)
        {
            externals_ = reso;
        }

        void expression::variable(const std::string &name, value_type value)
        {
            variables_[name] = value;
        }

        expression::value_type expression::evaluate(const std::string &expr)
        {
            std::istringstream is(expr);
            return parse(is);
        }

        expression::value_type expression::parse(std::istream &is)
        {
            is >> std::skipws;
            value_stack values;
            op_stack    ops;
            values.push_back(get_operand(is));
            if(!errorinfo_.empty())
                return expression::value_type();

            char op = ')';
            while((is >> op) && op != ')') {
                if(should_evaluate(op, ops)) {
                    evaluate(values, ops);
                    if(!errorinfo_.empty())
                        return expression::value_type();
                }
                values.push_back(get_operand(is));
                if(!errorinfo_.empty())
                    return expression::value_type();

                ops.push_back(op);
            }
            evaluate(values, ops);
            if(!errorinfo_.empty())
                return expression::value_type();

            return values.back();
        }

        ////////////////////////////////////////////////////////////////////////
        void expression::evaluate(char op, value_type &result, value_type operand) const
        {
            switch(op) {
            case '+': result += operand; break;
            case '-': result -= operand; break;
            case '*': result *= operand; break;
            case '/': result /= operand; break;
            case '%': result %= operand; break;
            default: report("syntax error: invalid operand"); break;
            }
        }

        void expression::evaluate(value_stack &values, op_stack &ops) const
        {
            while(ops.size() && values.size() > 1) {
                char op = ops.back();
                ops.pop_back();
                value_type operand = values.back();
                values.pop_back();
                evaluate(op, values.back(), operand);
                if(!errorinfo_.empty())
                    return;
            }
        }

        static int peek(std::istream& is)
        {
            int next = -1;
            while(isspace(next = is.peek())) is.get();
            return next;
        }

        static bool parse_assignment(std::istream& is)
        {
            int next = peek(is);
            if('=' != next) {
                return false;
            }
            is.get();
            return true;
        }

        static std::string parse_variable(std::istream& is)
        {
            std::string variable;
            char next = 0;
            char curr = 0;
            while((is >> next) &&
                  ((isalpha(next) || next == '.' || next == '_') ||
                  ((isalpha(curr) || curr == '.' || curr == '_') && isdigit(next)))) {
                variable += next;
                curr = next;
            }
            if(variable.size() == 0) {
                return "";
            }
            is.unget();

            return variable;
        }

        expression::value_type expression::get_operand(std::istream& is)
        {
            int next = peek(is);
            if(next == -1) {
                report("syntax error: operand expected");
                return expression::value_type();
            }
            if(isdigit(next)) {
                value_type operand = 0;
                if(!(is >> operand)) {
                    report("syntax error: number expected");
                    return expression::value_type();
                }
                return operand;
            }
            else if('(' == next) {
                is.get();
                return parse(is);
            }
            else if(isalpha(next)) {
                std::string variable = parse_variable(is);
                if(variable.empty()) {
                    report("syntax error: internal variable parse error");
                    return expression::value_type();
                }
                if(parse_assignment(is)) {
                    variables_[variable] = parse(is);
                }
                else {
                    value_type value;
                    if(variables_.count(variable)) {
                        return variables_[variable];
                    }
                    else if(externals_ && externals_->get(variable, value)) {
                        variables_[variable] = value;
                        return value;
                    }
                    else {
                        report(std::string("syntax error: undefined variable: ") + variable);
                        return expression::value_type();
                    }
                }
                return variables_[variable];
            }
            report("syntax error: unknown");
            return expression::value_type();
        }

        struct AlgPrio {
            char    ops;
            char    pri;
        };

        static const AlgPrio alg_prio[] = {
            {'=', 0},
            {'+', 1},
            {'-', 1},
            {'*', 2},
            {'/', 2},
            {'%', 2},
            {'(', 3},
            {')', 3}
        };

        static bool higher_precedence_or_left_associated(char last, char current)
        {
            char lhs = 0;
            char rhs = 0;
            for(size_t i = 0; i < sizeof(alg_prio)/sizeof(alg_prio[0]); ++i) {
                if(alg_prio[i].ops == last) {
                    lhs = alg_prio[i].pri;
                }
                if(alg_prio[i].ops == current) {
                    rhs = alg_prio[i].pri;
                }
            }
            return lhs >= rhs;
            //return (last == current) || (last == '*' || last == '/');
        }

        bool expression::should_evaluate(char op, const op_stack &ops) const
        {
            return ops.size() > 0 && higher_precedence_or_left_associated(ops.back(), op);
        }

    }
}
