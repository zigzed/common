/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_TOKENIZER_H
#define CXX_TOKENIZER_H
#include <vector>
#include <string>
#include <stdexcept>

namespace cxx {
    namespace str {

        class TokenizerError : public std::runtime_error {
        public:
            explicit TokenizerError(const std::string& msg) : std::runtime_error(msg) {}
        };

        class tokenizer {
        public:
            tokenizer(const std::string& line, const std::string& separator);
            tokenizer(const std::string &line, const std::string &separator, bool skip_empty);
            tokenizer(const std::string& line, const std::string& separator,
                      const std::string& quote, const std::string& escape);
            tokenizer(const std::string &line, const std::string &separator,
                      const std::string &quote, const std::string &escape,
                      bool skop_empty);
            std::string			operator[](size_t off);
            const std::string& 	operator[](size_t off) const;
            size_t      		size()  const;
            bool        		empty() const;
        private:
            void    token(const std::string& line, const std::string& separator);
            void	token(const std::string& line, const std::string& separator,
                          const std::string& quote, const std::string& escape);

            typedef std::vector<std::string > Container;
            Container   container_;
            bool        skipempty_;
        };

    }
}

#endif //

