/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_STR_TRIM_H
#define CXX_STR_TRIM_H
#include <iterator>
#include <cctype>

namespace cxx {
    namespace str {

        namespace detail {
            template<typename ForwardIterator, typename Predicate >
            inline ForwardIterator trim_end_iter_select(
                    ForwardIterator begin,
                    ForwardIterator end,
                    Predicate       pred,
                    std::forward_iterator_tag
                    )
            {
                ForwardIterator trim_it = begin;
                for(ForwardIterator it = begin; it != end; ++it) {
                    if(!pred(*it)) {
                        trim_it = it;
                        ++trim_it;
                    }
                }

                return trim_it;
            }

            template<typename ForwardIterator, typename Predicate >
            inline ForwardIterator trim_end_iter_select(
                    ForwardIterator begin,
                    ForwardIterator end,
                    Predicate       pred,
                    std::bidirectional_iterator_tag)
            {
                for(ForwardIterator it = end; it != begin; ) {
                    if(!pred(*(--it))) {
                        return ++it;
                    }
                }
                return begin;
            }

            template<typename ForwardIterator, typename Predicate >
            inline ForwardIterator trim_begin(
                    ForwardIterator begin,
                    ForwardIterator end,
                    Predicate       pred)
            {
                ForwardIterator it = begin;
                for(; it != end; ++it) {
                    if(!pred(*it)) {
                        return it;
                    }
                }
                return it;
            }

            template<typename ForwardIterator, typename Predicate >
            inline ForwardIterator trim_end(
                    ForwardIterator begin,
                    ForwardIterator end,
                    Predicate       pred)
            {
                typedef typename ForwardIterator::iterator_category category;
                return trim_end_iter_select(begin, end, pred, category());
            }

        }

        template<typename Sequence, typename Predicate >
        inline Sequence trim_left_copy_if(const Sequence& input, Predicate pred)
        {
            return Sequence(detail::trim_begin(input.begin(), input.end(), pred), input.end());
        }

        template<typename Sequence >
        inline Sequence trim_left_copy(const Sequence& input)
        {
            return Sequence(detail::trim_begin(input.begin(), input.end(), isspace), input.end());
        }

        template<typename Sequence, typename Predicate >
        inline Sequence& trim_left_if(Sequence& input, Predicate pred)
        {
            input.erase(input.begin(), detail::trim_begin(input.begin(), input.end(), pred));
            return input;
        }

        template<typename Sequence>
        inline Sequence& trim_left(Sequence& input)
        {
            input.erase(input.begin(), detail::trim_begin(input.begin(), input.end(), isspace));
            return input;
        }

        template<typename Sequence, typename Predicate >
        inline Sequence trim_right_copy_if(const Sequence& input, Predicate pred)
        {
            return Sequence(input.begin(), detail::trim_end(input.begin(), input.end(), pred));
        }

        template<typename Sequence >
        inline Sequence trim_right_copy(const Sequence& input)
        {
            return Sequence(input.begin(), detail::trim_end(input.begin(), input.end(), isspace));
        }

        template<typename Sequence, typename Predicate >
        inline Sequence& trim_right_if(Sequence& input, Predicate pred)
        {
            input.erase(detail::trim_end(input.begin(), input.end(), pred), input.end());
            return input;
        }

        template<typename Sequence >
        inline Sequence& trim_right(Sequence& input)
        {
            input.erase(detail::trim_end(input.begin(), input.end(), isspace), input.end());
            return input;
        }

        template<typename Sequence, typename Predicate >
        inline Sequence trim_copy_if(const Sequence& input, Predicate pred)
        {
            typename Sequence::iterator end = detail::trim_end(input.begin(), input.end(), pred);
            return Sequence(detail::trim_begin(input.begin(), end, pred), end);
        }

        template<typename Sequence >
        inline Sequence trim_copy(const Sequence& input)
        {
            typename Sequence::iterator end = detail::trim_end(input.begin(), input.end(), isspace);
            return Sequence(detail::trim_begin(input.begin(), end, isspace), end);
        }

        template<typename Sequence, typename Predicate >
        inline Sequence& trim_if(Sequence& input, Predicate pred)
        {
            trim_right_if(input, pred);
            trim_left_if(input, pred);
            return input;
        }

        template<typename Sequence >
        inline Sequence& trim(Sequence& input)
        {
            trim_right(input);
            trim_left(input);
            return input;
        }

    }
}

#endif
