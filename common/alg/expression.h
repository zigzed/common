/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ALG_EXPRESSION_H
#define CXX_ALG_EXPRESSION_H
#include <string>
#include <vector>
#include <map>
#include "common/config.h"


namespace cxx {
    namespace alg {


        class resolver;
        class expression {
        public:
            typedef int64_t                     value_type;
            expression();
            enum ErrorType { ET_EXCEPTION,   ET_ERRNO };

            void        err_type(ErrorType type);
            void        external(resolver* reso);
            void        variable(const std::string& name, value_type value);
            value_type  evaluate(const std::string& expr);
            const std::string& error() const { return errorinfo_; }
        private:
            typedef std::vector<value_type >    value_stack;
            typedef std::vector<char >          op_stack;
            typedef std::map<std::string, value_type >  memory;

            value_type  parse(std::istream& is);
            value_type  get_operand(std::istream& is);
            void        evaluate(char op, value_type& result, value_type operand) const;
            void        evaluate(value_stack& values, op_stack& ops) const;
            bool        should_evaluate(char op, const op_stack& ops) const;
            void        report(const std::string& msg) const;

            memory      variables_;
            resolver*   externals_;
            ErrorType   errortype_;
            mutable std::string errorinfo_;
        };

        class resolver {
        public:
            virtual ~resolver() {}
            virtual bool get(const std::string& name, expression::value_type& value) = 0;
        };

    }
}


#endif
