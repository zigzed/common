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
            protected:
                void* do_xchg(void* val);
                void* do_cas(void* cmp, void* val);
                volatile void* ptr_;
            };
        }

        template<typename T >
        class atomic_ptr_t : private detail::atomic_ptr_base_t {
        public:
            atomic_ptr_t() { this->ptr_ = 0; }
            atomic_ptr_t(T* ptr) { set(ptr); }
            ~atomic_ptr_t() {}
            void    set(T* ptr) { this->ptr_ = ptr; }
            /** perform atomic 'exchange pointer' operation. pointer is set
             * to 'val' value, and the old value is returned
             */
            T*      xchg(T* val) {
                return (T* )do_xchg((void* )val);
            }
            /** perform atomic 'compare and swap' operation. pointer is compared
             * to 'cmp' argument and if they are equal, it's value set to 'val',
             * old value of the pointer is returned
             */
            T*      cas(T* cmp, T* val) {
                return (T* )do_cas((void* )cmp, (void* )val);
            }
        private:
            atomic_ptr_t(const atomic_ptr_t& );
            atomic_ptr_t& operator= (const atomic_ptr_t& );
        };
    }

}

#endif
