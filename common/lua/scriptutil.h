/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_LUA_SCRIPTUTIL_H
#define CXX_LUA_SCRIPTUTIL_H
#include <string.h>

namespace cxx {
    namespace lua {

        template<bool C, typename A, typename B >
        struct if_ {
            typedef A type;
        };
        template<typename A, typename B >
        struct if_<false, A, B > {
            typedef B type;
        };

        template<bool T1, bool T2, bool T3=false, bool T4=false, bool T5=false, bool T6=false, bool T7=false >
        struct or_;
        template<bool T1, bool T2, bool T3, bool T4, bool T5, bool T6, bool T7 >
        struct or_ {
            static const bool	value = true;
        };
        template<>
        struct or_<false, false, false, false, false, false, false > {
            static const bool	value = false;
        };

        template<bool T1, bool T2, bool T3=false, bool T4=false, bool T5=false, bool T6=false, bool T7=false >
        struct and_;
        template<bool T1, bool T2, bool T3, bool T4, bool T5, bool T6, bool T7 >
        struct and_ {
            static const bool	value = false;
        };
        template<>
        struct or_<true, true, true, true, true, true, true > {
            static const bool	value = true;
        };

        template<bool T >
        struct not_ {
            static const bool	value = true;
        };
        template<>
        struct not_<true > {
            static const bool	value = false;
        };

        template<typename T >
        struct TypeSelect {
        };

        template<typename A >
        struct is_ptr {
            static const bool value = false;
        };
        template<typename A >
        struct is_ptr<A* > {
            static const bool value = true;
        };

        template<typename A >
        struct is_ref {
            static const bool value = false;
        };
        template<typename A >
        struct is_ref<A& > {
            static const bool value = true;
        };

        template<typename A >
        struct remove_const {
            typedef A type;
        };
        template<typename A >
        struct remove_const<const A > {
            typedef A type;
        };

        template<typename A >
        struct base_type {
            typedef A type;
        };
        template<typename A >
        struct base_type<A* > {
            typedef A type;
        };
        template<typename A >
        struct base_type<A& > {
            typedef A type;
        };

        template<typename A >
        struct class_type {
            typedef typename remove_const<typename base_type<A >::type >::type type;
        };

        typedef	char	yes_type;
        struct no_type {
            char padding[8];
        };

        struct int_conv_type {
            int_conv_type(int);
        };

        no_type 	int_conv_tester(...);
        yes_type	int_conv_tester(int_conv_type);

        no_type		vfnd_ptr_tester(const volatile char* );
        no_type		vfnd_ptr_tester(const volatile short* );
        no_type		vfnd_ptr_tester(const volatile int* );
        no_type		vfnd_ptr_tester(const volatile long* );
        no_type		vfnd_ptr_tester(const volatile float* );
        no_type		vfnd_ptr_tester(const volatile double* );
        no_type		vfnd_ptr_tester(const volatile bool* );
        yes_type	vfnd_ptr_tester(const volatile void* );

        template<typename T >
        T* add_ptr(T& );

        template<bool C >
        struct bool_to_yesno {
            typedef no_type		type;
        };
        template<>
        struct bool_to_yesno<true > {
            typedef	yes_type	type;
        };

        template<typename T >
        struct is_enum {
            static T			arg;
            static const bool 	value = ((sizeof(int_conv_tester(arg)) == sizeof(yes_type)) &&
                                         (sizeof(vfnd_ptr_tester(add_ptr(arg))) == sizeof(yes_type)));
        };

        template<typename T >
        struct void2val {
            static T	invoke(void* input) {
                return *(T* )input;
            }
        };
        template<typename T >
        struct void2ptr {
            static T*	invoke(void* input) {
                return (T* )input;
            }
        };
        template<typename T >
        struct void2ref {
            static T&	invoke(void* input) {
                return *(T* )input;
            }
        };

        template<typename T >
        struct void2type {
            static T	invoke(void* ptr) {
                return if_<is_ptr<T >::value,
                            void2ptr<typename base_type<T >::type >,
                            typename if_<is_ref<T >::value,
                                void2ref<typename base_type<T >::type >,
                                void2val<typename base_type<T >::type >
                            >::type
                        >::type::invoke(ptr);
            }
        };

        template<typename T>
        struct class_name {
        public:
            static const char* name(const char* n = 0) {
                static char name_[MAX_CLASS_NAME_LEN + 1];
                if(n) {
                    strncpy(name_, n, MAX_CLASS_NAME_LEN);
                }
                return name_;
            }
        private:
            enum { MAX_CLASS_NAME_LEN = 255 };
        };

    }
}

#endif
/**
 * $Id: scriptutil.h,v 1.1.1.1 2007/06/19 02:58:59 wilbur Exp $
 * $Log: scriptutil.h,v $
 * Revision 1.1.1.1  2007/06/19 02:58:59  wilbur
 * initial import
 *
 */

