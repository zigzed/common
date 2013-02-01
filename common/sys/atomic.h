/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ATOMIC_H
#define CXX_ATOMIC_H
#include "common/config.h"

namespace cxx {
    namespace sys {

        class atomic_t {
        public:
            explicit atomic_t(long val);
            long operator++();
            long operator--();
            operator long() const;
        private:
            atomic_t(const atomic_t& rhs);
            atomic_t& operator= (const atomic_t& rhs);
            mutable long value_;
        };

        namespace detail {
            class atomic_ptr_base_t {
            public:
                static void* do_xchg(volatile void* ptr, void* val);
                static void* do_cas(volatile void* ptr, void* cmp, void* val);
            };
        }

        template<typename T >
        class atomic_ptr_t : private detail::atomic_ptr_base_t {
        public:
            atomic_ptr_t() : ptr_(0) {}
            atomic_ptr_t(T* ptr) : ptr_(0) { set(ptr); }
            ~atomic_ptr_t() {}
            void    set(T* ptr) { this->ptr_ = ptr; }
            /** perform atomic 'exchange pointer' operation. pointer is set
             * to 'val' value, and the old value is returned
             */
            T*      xchg(T* val) {
                return (T* )do_xchg(&ptr_, val);
            }
            /** perform atomic 'compare and swap' operation. pointer is compared
             * to 'cmp' argument and if they are equal, it's value set to 'val',
             * old value of the pointer is returned
             */
            T*      cas(T* cmp, T* val) {
                return (T* )do_cas(&ptr_, cmp, val);
            }
        private:
            volatile T* ptr_;

            atomic_ptr_t(const atomic_ptr_t& );
            atomic_ptr_t& operator= (const atomic_ptr_t& );
        };
    }

}

#endif
