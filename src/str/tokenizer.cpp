/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include <string>
#include <algorithm>
#include "common/str/tokenizer.h"

namespace cxx {
    namespace str {

        namespace detail {

            class char_separator {
            private:
                typedef std::string::traits_type	Traits;
                typedef Traits::char_type			value_type;

                struct char_eq {
                    value_type	c_;
                    char_eq(value_type c) : c_(c) {}
                    bool operator()(value_type c) {
                        return Traits::eq(c_, c);
                    }
                };
                bool is_delims(value_type s) const {
                    char_eq	f(s);
                    return std::find_if(delims_.begin(), delims_.end(), f) != delims_.end();
                }
            public:
                explicit char_separator(const std::string& separator)
                    : delims_(separator), finish_(false) {}
                void	reset() {}
                template<typename InputIterator, typename Token>
                bool operator()(InputIterator& next, InputIterator end, Token& tok) {
                    tok = Token();
                    if(next == end) {
                        if(finish_) {
                            finish_ = false;
                            return true;
                        }
                        else
                            return false;
                    }
                    finish_ = false;
                    for(; next != end; ++next) {
                        if(is_delims(*next)) {
                            ++next;
                            finish_ = true;
                            return true;
                        }
                        else
                            tok += *next;
                    }
                    return true;
                }
            private:
                std::string	delims_;
                bool		finish_;
            };

            class escape_separator {
            private:
                typedef std::string::traits_type	Traits;
                typedef Traits::char_type			value_type;

                struct char_eq {
                    value_type	c_;
                    char_eq(value_type c) : c_(c) {}
                    bool operator()(value_type c) {
                        return Traits::eq(c_, c);
                    }
                };
                bool is_escape(value_type e) const {
                    char_eq	f(e);
                    return std::find_if(escape_.begin(), escape_.end(), f) != escape_.end();
                }
                bool is_quotes(value_type q) const {
                    char_eq	f(q);
                    return std::find_if(quotes_.begin(), quotes_.end(), f) != quotes_.end();
                }
                bool is_delims(value_type s) const {
                    char_eq	f(s);
                    return std::find_if(delims_.begin(), delims_.end(), f) != delims_.end();
                }
            public:
                escape_separator(const std::string& escape, const std::string& delims,
                                 const std::string& quotes)
                    : escape_(escape), delims_(delims), quotes_(quotes), finish_(false)
                {
                }
                void	reset() { finish_ = false; }

                template<typename InputIterator, typename Token >
                bool operator()(InputIterator& next, InputIterator end, Token& tok) {
                    bool inquote = false;
                    tok = Token();

                    if(next == end) {
                        if(finish_) {
                            finish_ = false;
                            return true;
                        }
                        else
                            return false;
                    }
                    finish_ = false;
                    for(; next != end; ++next) {
                        if(is_escape(*next)) {
                            do_escape(next, end, tok);
                        }
                        else if(is_delims(*next)) {
                            if(!inquote) {
                                // if we're not in quote, then we are done
                                ++next;
                                finish_ = true;
                                return true;
                            }
                            else
                                tok += *next;
                        }
                        else if(is_quotes(*next)) {
                            inquote = !inquote;
                        }
                        else {
                            tok += *next;
                        }
                    }
                    return true;
                }

                template<typename InputIterator, typename Token>
                void do_escape(InputIterator& next, InputIterator end, Token& tok) {
                    if(++next == end)
                        throw TokenizerError("cannot end with escape");
                    if(Traits::eq(*next, 'n')) {
                        tok += '\n';
                        return;
                    }
                    else if(is_quotes(*next)) {
                        tok += *next;
                        return;
                    }
                    else if(is_delims(*next)) {
                        tok += *next;
                        return;
                    }
                    else if(is_escape(*next)) {
                        tok += *next;
                        return;
                    }
                    else
                        throw TokenizerError("unknown escape sequence");
                }
            private:
                std::string escape_;
                std::string delims_;
                std::string quotes_;
                bool		finish_;
            };

        }


        tokenizer::tokenizer(const std::string& line, const std::string& separator)
            : skipempty_(false)
        {
            token(line, separator);
        }

        tokenizer::tokenizer(const std::string& line, const std::string& separator,
                             bool skip_empty)
            : skipempty_(skip_empty)
        {
            token(line, separator);
        }

        tokenizer::tokenizer(const std::string& line, const std::string& separator,
                             const std::string& quote, const std::string& escape)
            : skipempty_(false)
        {
            token(line, separator, quote, escape);
        }

        tokenizer::tokenizer(const std::string& line, const std::string& separator,
                                         const std::string& quote, const std::string& escape,
                                         bool skip_empty)
            : skipempty_(skip_empty)
        {
            token(line, separator, quote, escape);
        }

        bool tokenizer::empty() const
        {
            return container_.empty();
        }

        std::size_t tokenizer::size() const
        {
            return container_.size();
        }

        const std::string& tokenizer::operator[](size_t off) const
        {
            return container_.at(off);
        }

        std::string tokenizer::operator[](size_t off)
        {
            return container_.at(off);
        }

        void tokenizer::token(const std::string& line, const std::string& separator)
        {
            detail::char_separator		DelimsFunc(separator);
            std::string::const_iterator	it = line.begin();
            std::string					result;
            while(DelimsFunc(it, line.end(), result)) {
                if(!result.empty() || !skipempty_) {
                    container_.push_back(result);
                }
                DelimsFunc.reset();
            }
        }

        void tokenizer::token(const std::string& line, const std::string& separator,
                              const std::string& quote, const std::string& escape)
        {
            detail::escape_separator	DelimsFunc(escape, separator, quote);
            std::string::const_iterator	it = line.begin();
            std::string					result;
            while(DelimsFunc(it, line.end(), result)) {
                if(!result.empty() || !skipempty_) {
                    container_.push_back(result);
                }
                DelimsFunc.reset();
            }
        }

    }
}

