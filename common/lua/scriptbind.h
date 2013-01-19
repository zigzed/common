/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_LUA_SCRIPTBIND_H
#define	CXX_LUA_SCRIPTBIND_H
#include <string>
#include <vector>
#include <stdexcept>
#include "common/lua/scriptvm.h"
#include "common/lua/scriptutil.h"

struct lua_State;
namespace cxx {
    namespace lua {

        struct user {
            user(void* p) : ptr(p) {}
            virtual ~user() {}
            void* ptr;
        };

        struct table;

        template<typename T >
        struct user2type {
            static T	invoke(ScriptVM* v, int index) {
                return void2type<T>::invoke(ScriptStack(v).getuserdata(index));
            }
        };
        template<typename T >
        struct lua2enum {
            static T	invoke(ScriptVM* v, int index) {
                return (T)(int)ScriptStack(v).getnumber(index);
            }
        };
        template<typename T >
        struct lua2object {
            static T 	invoke(ScriptVM* v, int index) {
                ScriptStack	st(v);
                if(!st.isuserdata(index)) {
                    v->pusherror("no class at first argument. (forgot ':' expression?)");
                }
                return void2type<T >::invoke((user2type<user* >::invoke(v, index))->ptr);
            }
        };
        template<typename T >
        T lua2type(ScriptVM* v, int index) {
            return if_<is_enum<T >::value,
                        lua2enum<T >,
                        lua2object<T >
                    >::type::invoke(v, index);
        }

        template<typename T >
        struct val2user : public user {
            val2user() : user(new T) {}
            template<typename T1 >
            val2user(T1 t1) : user(new T(t1)) {}
            template<typename T1, typename T2 >
            val2user(T1 t1, T2 t2) : user(new T(t1, t2)) {}
            template<typename T1, typename T2, typename T3 >
            val2user(T1 t1, T2 t2, T3 t3) : user(new T(t1, t2, t3)) {}
            template<typename T1, typename T2, typename T3, typename T4 >
            val2user(T1 t1, T2 t2, T3 t3, T4 t4) : user(new T(t1, t2, t3, t4)) {}
            template<typename T1, typename T2, typename T3, typename T4, typename T5 >
            val2user(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) : user(new T(t1, t2, t3, t4, t5)) {}
            ~val2user() { delete ((T* )ptr); }
        };
        template<typename T >
        struct ptr2user : public user {
            ptr2user(T* t) : user((void* )t) {}
        };
        template<typename T >
        struct ref2user : public user {
            ref2user(T& t) : user(&t) {}
        };

        template<typename T >
        struct val2lua {
            static void	invoke(ScriptVM* v, T& input) {
                new(v->getmemory(sizeof(val2user<T >))) val2user<T >(input);
            }
        };
        template<typename T >
        struct ptr2lua {
            static void invoke(ScriptVM* v, T* input) {
                if(input) {
                    new(v->getmemory(sizeof(ptr2user<T >))) ptr2user<T >(input);
                }
                else {
                    ScriptStack(v).putnull();
                }
            }
        };
        template<typename T >
        struct ref2lua {
            static void invoke(ScriptVM* v, T& input) {
                new(v->getmemory(sizeof(ref2user<T >))) ref2user<T >(input);
            }
        };
        template<typename T >
        struct enum2lua {
            static void invoke(ScriptVM* v, T val) {
                ScriptStack(v).putnumber((int)val);
            }
        };

        template<typename T >
        struct object2lua {
            static void invoke(ScriptVM* v, T val) {
                if_<is_ptr<T >::value,
                    ptr2lua<typename base_type<T >::type >,
                    typename if_<is_ref<T >::value,
                        ref2lua<typename base_type<T >::type >,
                        val2lua<typename base_type<T >::type >
                    >::type
                >::type::invoke(v, val);

                ScriptStack ss(v);
                ss.putmeta(class_name<typename class_type<T >::type >::name());
                ss.setmetatab(-2);
            }
        };
        template<typename T >
        static void type2lua(ScriptVM* v, T val) {
            if_<is_enum<T >::value,
                enum2lua<T >,
                object2lua<T >
            >::type::invoke(v, val);
        }

        template<typename T >
        T upvalue(ScriptVM* v)
        {
            return user2type<T >::invoke(v, ScriptStack(v).getupvalidx(1));
        }

        template<typename T >
        T	read(ScriptVM* v, int index, TypeSelect<T >) {
            return lua2type<T >(v, index);
        }
        char*			read(ScriptVM* v, int index, TypeSelect<char* >);
        const char*		read(ScriptVM* v, int index, TypeSelect<const char* >);
        char			read(ScriptVM* v, int index, TypeSelect<char >);
        unsigned char	read(ScriptVM* v, int index, TypeSelect<unsigned char >);
        short			read(ScriptVM* v, int index, TypeSelect<short >);
        unsigned short	read(ScriptVM* v, int index, TypeSelect<unsigned short >);
        int				read(ScriptVM* v, int index, TypeSelect<int >);
        unsigned int	read(ScriptVM* v, int index, TypeSelect<unsigned int >);
        long			read(ScriptVM* v, int index, TypeSelect<long >);
        unsigned long	read(ScriptVM* v, int index, TypeSelect<unsigned long >);
        float			read(ScriptVM* v, int index, TypeSelect<float >);
        double			read(ScriptVM* v, int index, TypeSelect<double >);
        bool			read(ScriptVM* v, int index, TypeSelect<bool >);
        std::string     read(ScriptVM* v, int index, TypeSelect<std::string >);
        void			read(ScriptVM* v, int index, TypeSelect<void >);
        table			read(ScriptVM* v, int index, TypeSelect<table >);

        template<typename T>
        void			push(ScriptVM* v, T val) {
            type2lua<T >(v, val);
        }
        void			push(ScriptVM* v, char val);
        void			push(ScriptVM* v, unsigned char val);
        void			push(ScriptVM* v, short val);
        void			push(ScriptVM* v, unsigned short val);
        void			push(ScriptVM* v, int val);
        void			push(ScriptVM* v, unsigned int val);
        void			push(ScriptVM* v, long val);
        void			push(ScriptVM* v, unsigned long val);
        void			push(ScriptVM* v, float val);
        void			push(ScriptVM* v, double val);
        void			push(ScriptVM* v, bool val);
        void			push(ScriptVM* v, char* val);
        void			push(ScriptVM* v, const char* val);
        void			push(ScriptVM* v, table tab);

        template<typename T>
        T				pop(ScriptVM* v) {
            T t = read(v, -1, TypeSelect<T >());
            ScriptStack(v).pop(1);
            return t;
        }
        template<>
        inline void		pop<void >(ScriptVM* v) {
            ScriptStack(v).pop(1);
        }

        template<typename T >
        struct ret_ {
            static const int value = 1;
        };
        template<>
        struct ret_<void > {
            static const int value = 0;
        };

        // functor
        template<typename R >
        struct functor {
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, upvalue<R (*)() >(&v)());
                return ret_<R >::value;
            }
            template<typename P1 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, upvalue<R (*)(P1 )>(&v)(
                    read(&v, 1, TypeSelect<P1 >())));
                return ret_<R >::value;
            }
            template<typename P1, typename P2 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, upvalue<R (*)(P1, P2) >(&v)(
                    read(&v, 1, TypeSelect<P1 >()),
                    read(&v, 2, TypeSelect<P2 >())));
                return ret_<R >::value;
            }
            template<typename P1, typename P2, typename P3 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, upvalue<R (*)(P1, P2, P3) >(&v)(
                    read(&v, 1, TypeSelect<P1 >()),
                    read(&v, 2, TypeSelect<P2 >()),
                    read(&v, 3, TypeSelect<P3 >())));
                return ret_<R >::value;
            }
            template<typename P1, typename P2, typename P3, typename P4 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, upvalue<R (*)(P1, P2, P3, P4) >(&v)(
                    read(&v, 1, TypeSelect<P1 >()),
                    read(&v, 2, TypeSelect<P2 >()),
                    read(&v, 3, TypeSelect<P3 >()),
                    read(&v, 4, TypeSelect<P4 >())));
                return ret_<R >::value;
            }
            template<typename P1, typename P2, typename P3, typename P4, typename P5 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, upvalue<R (*)(P1, P2, P3, P4, P5) >(&v)(
                    read(&v, 1, TypeSelect<P1 >()),
                    read(&v, 2, TypeSelect<P2 >()),
                    read(&v, 3, TypeSelect<P3 >()),
                    read(&v, 4, TypeSelect<P4 >()),
                    read(&v, 5, TypeSelect<P5 >())));
                return ret_<R >::value;
            }
        };

        template<>
        struct functor<void > {
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                upvalue<void (*)() >(&v)();
                return 0;
            }
            template<typename P1 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                upvalue<void (*)(P1 )>(&v)(
                    read(&v, 1, TypeSelect<P1 >()));
                return 0;
            }
             template<typename P1, typename P2 >
             static int invoke(lua_State* L) {
                 ScriptVM    v(L);
                 upvalue<void (*)(P1, P2) >(&v)(
                     read(&v, 1, TypeSelect<P1 >()),
                     read(&v, 2, TypeSelect<P2 >()));
                 return 0;
             }
             template<typename P1, typename P2, typename P3 >
             static int invoke(lua_State* L) {
                 ScriptVM    v(L);
                 upvalue<void (*)(P1, P2, P3) >(&v)(
                     read(&v, 1, TypeSelect<P1 >()),
                     read(&v, 2, TypeSelect<P2 >()),
                     read(&v, 3, TypeSelect<P3 >()));
                 return 0;
             }
             template<typename P1, typename P2, typename P3, typename P4 >
             static int invoke(lua_State* L) {
                 ScriptVM    v(L);
                 upvalue<void (*)(P1, P2, P3, P4) >(&v)(
                     read(&v, 1, TypeSelect<P1 >()),
                     read(&v, 2, TypeSelect<P2 >()),
                     read(&v, 3, TypeSelect<P3 >()),
                     read(&v, 4, TypeSelect<P4 >()));
                 return 0;
             }
             template<typename P1, typename P2, typename P3, typename P4, typename P5 >
             static int invoke(lua_State* L) {
                 ScriptVM    v(L);
                 upvalue<void (*)(P1, P2, P3, P4, P5) >(&v)(
                     read(&v, 1, TypeSelect<P1 >()),
                     read(&v, 2, TypeSelect<P2 >()),
                     read(&v, 3, TypeSelect<P3 >()),
                     read(&v, 4, TypeSelect<P4 >()),
                     read(&v, 5, TypeSelect<P5 >()));
                 return 0;
             }
        };


        template<typename R >
        void push_functor(ScriptVM* v, R (*func)()) {
            ScriptStack(v).putclosure(functor<R >::invoke, 1);
        }
        template<typename R, typename P1 >
        void push_functor(ScriptVM* v, R (*func)(P1 )) {
            ScriptStack(v).putclosure(functor<R >::template invoke<P1 >, 1);
        }
        template<typename R, typename P1, typename P2 >
        void push_functor(ScriptVM* v, R (*func)(P1, P2 )) {
            ScriptStack(v).putclosure(functor<R >::template invoke<P1, P2 >, 1);
        }
        template<typename R, typename P1, typename P2, typename P3 >
        void push_functor(ScriptVM* v, R (*func)(P1, P2, P3 )) {
            ScriptStack(v).putclosure(functor<R >::template invoke<P1, P2, P3 >, 1);
        }
        template<typename R, typename P1, typename P2, typename P3, typename P4 >
        void push_functor(ScriptVM* v, R (*func)(P1, P2, P3, P4 )) {
            ScriptStack(v).putclosure(functor<R >::template invoke<P1, P2, P3, P4 >, 1);
        }
        template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5 >
        void push_functor(ScriptVM* v, R (*func)(P1, P2, P3, P4, P5 )) {
            ScriptStack(v).putclosure(functor<R >::template invoke<P1, P2, P3, P4, P5 >, 1);
        }

        struct var_base {
            virtual void get(ScriptVM* v) = 0;
            virtual void set(ScriptVM* v) = 0;
            virtual ~var_base() {}
        };

        template<typename T, typename V >
        struct mem_var : public var_base {
            V	T::*var_;
            mem_var(V T::*var) : var_(var) {}
            void get(ScriptVM* v) {
                push(v, read(v, 1, TypeSelect<T* >())->*(var_));
            }
            void set(ScriptVM* v) {
                read(v, 1, TypeSelect<T* >())->*(var_) = read(v, 3, TypeSelect<V >());
            }
        };

        // member function
        template<typename R >
        struct mem_functor {
            template<typename T >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, (read(&v, 1, TypeSelect<T* >())->*upvalue<R (T::*)() >(&v))());
                return ret_<R >::value;
            }
            template<typename T, typename P1 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, (read(&v, 1, TypeSelect<T* >())->*upvalue<R (T::*)(P1 ) >(&v))(
                                read(&v, 2, TypeSelect<P1 >())));
                return ret_<R >::value;
            }
            template<typename T, typename P1, typename P2 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, (read(&v, 1, TypeSelect<T* >())->*upvalue<R (T::*)(P1, P2) >(&v))(
                                read(&v, 2, TypeSelect<P1 >()),
                                read(&v, 3, TypeSelect<P2 >())));
                return ret_<R >::value;
            }
            template<typename T, typename P1, typename P2, typename P3 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, (read(&v, 1, TypeSelect<T* >())->*upvalue<R (T::*)(P1, P2, P3) >(&v))(
                                read(&v, 2, TypeSelect<P1 >()),
                                read(&v, 3, TypeSelect<P2 >()),
                                read(&v, 4, TypeSelect<P3 >())));
                return ret_<R >::value;
            }
            template<typename T, typename P1, typename P2, typename P3, typename P4>
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, (read(&v, 1, TypeSelect<T* >())->*upvalue<R (T::*)(P1, P2, P3, P4) >(&v))(
                                read(&v, 2, TypeSelect<P1 >()),
                                read(&v, 3, TypeSelect<P2 >()),
                                read(&v, 4, TypeSelect<P3 >()),
                                read(&v, 5, TypeSelect<P4 >())));
                return ret_<R >::value;
            }
            template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                push(&v, (read(&v, 1, TypeSelect<T* >())->*upvalue<R (T::*)(P1, P2, P3, P4, P5) >(&v))(
                                read(&v, 2, TypeSelect<P1 >()),
                                read(&v, 3, TypeSelect<P2 >()),
                                read(&v, 4, TypeSelect<P3 >()),
                                read(&v, 5, TypeSelect<P4 >()),
                                read(&v, 6, TypeSelect<P5 >())));
                return ret_<R >::value;
            }
        };

        template<>
        struct mem_functor<void > {
            template<typename T >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                (read(&v, 1, TypeSelect<T* >())->*upvalue<void (T::*)() >(&v))();
                return 1;
            }
            template<typename T, typename P1 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                (read(&v, 1, TypeSelect<T* >())->*upvalue<void (T::*)(P1 ) >(&v))(
                                read(&v, 2, TypeSelect<P1 >()));
                return 1;
            }
            template<typename T, typename P1, typename P2 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                (read(&v, 1, TypeSelect<T* >())->*upvalue<void (T::*)(P1, P2) >(&v))(
                                read(&v, 2, TypeSelect<P1 >()),
                                read(&v, 3, TypeSelect<P2 >()));
                return 1;
            }
            template<typename T, typename P1, typename P2, typename P3 >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                (read(&v, 1, TypeSelect<T* >())->*upvalue<void (T::*)(P1, P2, P3) >(&v))(
                                read(&v, 2, TypeSelect<P1 >()),
                                read(&v, 3, TypeSelect<P2 >()),
                                read(&v, 4, TypeSelect<P3 >()));
                return 1;
            }
            template<typename T, typename P1, typename P2, typename P3, typename P4>
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                (read(&v, 1, TypeSelect<T* >())->*upvalue<void (T::*)(P1, P2, P3, P4) >(&v))(
                                read(&v, 2, TypeSelect<P1 >()),
                                read(&v, 3, TypeSelect<P2 >()),
                                read(&v, 4, TypeSelect<P3 >()),
                                read(&v, 5, TypeSelect<P4 >()));
                return 1;
            }
            template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                (read(&v, 1, TypeSelect<T* >())->*upvalue<void (T::*)(P1, P2, P3, P4, P5) >(&v))(
                                read(&v, 2, TypeSelect<P1 >()),
                                read(&v, 3, TypeSelect<P2 >()),
                                read(&v, 4, TypeSelect<P3 >()),
                                read(&v, 5, TypeSelect<P4 >()),
                                read(&v, 6, TypeSelect<P5 >()));
                return 1;
            }
        };

        template<typename R, typename T >
        void push_functor(ScriptVM* v, R (T::*func)())
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T >, 1);
        }
        template<typename R, typename T >
        void push_functor(ScriptVM* v, R (T::*func)() const)
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T >, 1);
        }
        template<typename R, typename T, typename P1 >
        void push_functor(ScriptVM* v, R (T::*func)(P1))
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T, P1 >, 1);
        }
        template<typename R, typename T, typename P1 >
        void push_functor(ScriptVM* v, R (T::*func)(P1) const)
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T, P1 >, 1);
        }
        template<typename R, typename T, typename P1, typename P2 >
        void push_functor(ScriptVM* v, R (T::*func)(P1, P2))
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T, P1, P2 >, 1);
        }
        template<typename R, typename T, typename P1, typename P2 >
        void push_functor(ScriptVM* v, R (T::*func)(P1, P2) const)
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T, P1, P2 >, 1);
        }
        template<typename R, typename T, typename P1, typename P2, typename P3 >
        void push_functor(ScriptVM* v, R (T::*func)(P1, P2, P3))
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T, P1, P2, P3 >, 1);
        }
        template<typename R, typename T, typename P1, typename P2, typename P3 >
        void push_functor(ScriptVM* v, R (T::*func)(P1, P2, P3) const)
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T, P1, P2, P3 >, 1);
        }
        template<typename R, typename T, typename P1, typename P2, typename P3, typename P4 >
        void push_functor(ScriptVM* v, R (T::*func)(P1, P2, P3, P4))
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T, P1, P2, P3, P4 >, 1);
        }
        template<typename R, typename T, typename P1, typename P2, typename P3, typename P4 >
        void push_functor(ScriptVM* v, R (T::*func)(P1, P2, P3, P4) const)
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T, P1, P2, P3, P4 >, 1);
        }
        template<typename R, typename T, typename P1, typename P2, typename P3, typename P4, typename P5 >
        void push_functor(ScriptVM* v, R (T::*func)(P1, P2, P3, P4, P5))
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T, P1, P2, P3, P4, P5 >, 1);
        }
        template<typename R, typename T, typename P1, typename P2, typename P3, typename P4, typename P5 >
        void push_functor(ScriptVM* v, R (T::*func)(P1, P2, P3, P4, P5) const)
        {
            ScriptStack(v).putclosure(mem_functor<R >::template invoke<T, P1, P2, P3, P4, P5 >, 1);
        }

        // constructor
        template<typename T, typename P1 = void, typename P2 = void, typename P3 = void, typename P4 = void, typename P5 = void >
        struct constructor;

        template<typename T >
        struct constructor<T > {
            static void invoke(ScriptVM* v) {
                new(v->getmemory(sizeof(val2user<T >))) val2user<T >();
            }
        };
        template<typename T, typename P1 >
        struct constructor<T, P1 > {
            static void invoke(ScriptVM* v) {
                new(v->getmemory(sizeof(val2user<T >))) val2user<T >(
                    read(v, 2, TypeSelect<P1 >()));
            }
        };
        template<typename T, typename P1, typename P2 >
        struct constructor<T, P1, P2 > {
            static void invoke(ScriptVM* v) {
                new(v->getmemory(sizeof(val2user<T >))) val2user<T >(
                    read(v, 2, TypeSelect<P1 >()),
                    read(v, 3, TypeSelect<P2 >()));
            }
        };
        template<typename T, typename P1, typename P2, typename P3 >
        struct constructor<T, P1, P2, P3 > {
            static void invoke(ScriptVM* v) {
                new(v->getmemory(sizeof(val2user<T >))) val2user<T >(
                    read(v, 2, TypeSelect<P1 >()),
                    read(v, 3, TypeSelect<P2 >()),
                    read(v, 4, TypeSelect<P3 >()));
            }
        };
        template<typename T, typename P1, typename P2, typename P3, typename P4 >
        struct constructor<T, P1, P2, P3, P4 > {
            static void invoke(ScriptVM* v) {
                new(v->getmemory(sizeof(val2user<T >))) val2user<T >(
                    read(v, 2, TypeSelect<P1 >()),
                    read(v, 3, TypeSelect<P2 >()),
                    read(v, 4, TypeSelect<P3 >()),
                    read(v, 5, TypeSelect<P4 >()));
            }
        };
        template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5 >
        struct constructor {
            static void invoke(ScriptVM* v) {
                new(v->getmemory(sizeof(val2user<T >))) val2user<T >(
                    read(v, 2, TypeSelect<P1 >()),
                    read(v, 3, TypeSelect<P2 >()),
                    read(v, 4, TypeSelect<P3 >()),
                    read(v, 5, TypeSelect<P4 >()),
                    read(v, 6, TypeSelect<P5 >()));
            }
        };
        template<typename T >
        struct creator {
            template<typename CONSTRUCTOR >
            static int invoke(lua_State* L) {
                ScriptVM	v(L);
                ScriptStack	s(&v);
                CONSTRUCTOR::invoke(&v);
                s.putmeta(class_name<typename class_type<T >::type >::name());
                s.setmetatab(-2);
                return 1;
            }
        };


        // destructor
        template<typename T >
        static int destroyer(lua_State* L) {
            ScriptVM	v(L);
            ScriptStack	s(&v);
            ((user* )s.getuserdata(1))->~user();
            return 0;
        };

        class class_helper {
        public:
            static void setup(ScriptVM* v, const char* name);
            static int	meta_get(lua_State* L);
            static int	meta_set(lua_State* L);
            static void meta_push(ScriptVM* v, const char* name);
        };

        class declare {
        public:
            explicit declare(ScriptVM* v);
        protected:
            ScriptVM*	vm_;
        };

        template<typename T >
        class class_declare : public declare {
        public:
            class_declare(ScriptVM* v, const char* name) : declare(v) {
                class_name<T >::name(name);
                class_helper::setup(vm_, name);
                ScriptStack	s(vm_);
                s.putstring("__gc");
                s.putclosure(destroyer<T >, 0);
                s.rawset(-3);
                s.settable(-10002);
            }
            /** declare the parent of a class. please note, parent must be
             *  defined first.
             */
            template<typename B >
            class_declare& parent() {
                ScriptStack	s(vm_);
                class_helper::meta_push(vm_, class_name<T >::name());
                if(s.istable(-1)) {
                    s.putstring("__parent");
                    class_helper::meta_push(vm_, class_name<B >::name());
                    s.rawset(-3);
                }
                s.pop(1);
                return *this;
            }
            template<typename CON >
            class_declare& create(CON ) {
                ScriptStack	s(vm_);
                class_helper::meta_push(vm_, class_name<T >::name());
                if(s.istable(-1)) {
                    s.newtable();
                    s.putstring("__call");
                    s.putclosure(creator<T >::template invoke<CON >, 0);
                    s.rawset(-3);
                    s.setmetatab(-2);
                }
                s.pop(1);
                return *this;
            }
            template<typename FUNC >
            class_declare& method(const char* name, FUNC func) {
                ScriptStack	s(vm_);
                class_helper::meta_push(vm_, class_name<T >::name());
                if(s.istable(-1)) {
                    s.putstring(name);
                    new(vm_->getmemory(sizeof(FUNC))) FUNC(func);
                    push_functor(vm_,func );
                    s.rawset(-3);
                }
                s.pop(1);
                return *this;
            }
            template<typename B, typename V >
            class_declare& member(const char* name, V B::*var) {
                ScriptStack	s(vm_);
                class_helper::meta_push(vm_, class_name<T >::name());
                if(s.istable(-1)) {
                    s.putstring(name);
                    new(vm_->getmemory(sizeof(mem_var<B, V >))) mem_var<B, V >(var);
                    s.rawset(-3);
                }
                s.pop(1);
                return *this;
            }
        };

        class module_ : public declare {
        public:
            explicit module_(ScriptVM* v);
            template<typename FUNC >
            module_& function(const char* name, FUNC func) {
                ScriptStack	s(vm_);
                s.putstring(name);
                s.putuserdata((void* )func);
                push_functor(vm_, func);
                s.settable(-10002);
                return *this;
            }
            template<typename T >
            module_& variable(const char* name, T var) {
                ScriptStack	s(vm_);
                s.putstring(name);
                push(vm_, var);
                s.settable(-10002);
                return *this;
            }
            template<typename T >
            class_declare<T > class_(const char* name) {
                return class_declare<T >(vm_, name);
            }
        };

        struct table_obj {
        public:
            table_obj(ScriptVM* vm, int index);
            ~table_obj();
            void inc_ref();
            void dec_ref();
            bool validate();
            template<typename T >
            void set(const char* name, T value) {
                if(validate()) {
                    ScriptStack	st(svm);
                    st.putstring(name);
                    push(svm, value);
                    st.settable(idx);
                }
            }
            template<typename T >
            T get(const char* name) {
                ScriptStack	st(svm);
                if(validate()) {
                    st.putstring(name);
                    st.gettable(idx);
                }
                else {
                    st.putnull();
                }
                return pop<T >(svm);
            }
            template<typename T >
            T get(int index) {
                ScriptStack st(svm);
                if(validate()) {
                    st.putnumber(index);
                    st.gettable(idx);
                }
                else {
                    st.putnull();
                }
                return pop<T >(svm);
            }

            ScriptVM*	svm;
            int			idx;
            const void*	ptr;
            int			ref;
        };

        struct table {
            explicit table(ScriptVM* vm);
            table(ScriptVM* vm, int index);
            table(ScriptVM* vm, const char* name);
            table(const table& rhs);
            ~table();

            template<typename T >
            void set(const char* name, T value) {
                obj->set(name, value);
            }
            template<typename T >
            T get(const char* name) {
                return obj->get<T >(name);
            }
            template<typename T >
            T get(int index) {
                return obj->get<T >(index);
            }

            table_obj*  obj;
        };

        template<>
        inline table	pop<table >(ScriptVM* v) {
            return table(v, ScriptStack(v).gettop());
        }

        template<typename T >
        inline T        global(ScriptVM* v, const char* name) {
            ScriptStack(v).getglobal(name);
            return pop<T >(v);
        }

        template<typename T >
        inline void     global(ScriptVM* v, const char* name, const T& val) {
            push<T >(v, val);
            ScriptStack(v).setglobal(name);
        }

        template<typename T>
        inline std::vector<T > vector(ScriptVM* v, const char* name)
        {
            std::vector<T > val;
            ScriptStack s(v);
            s.getglobal(name);
            int size = s.objlen(s.gettop());
            table tb = pop<table >(v);
            for(int i = 1; i <= size; ++i) {
                val.push_back(tb.get<T >(i));
            }
            return val;
        }

        inline bool     isnull(ScriptVM* v, const char* name) {
            ScriptStack ss(v);
            ss.getglobal(name);
            return ss.isnull(-2) == 1;
        }

    }
}

#endif

