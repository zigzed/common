/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_LUA_SCRIPTCALL_H
#define	CXX_LUA_SCRIPTCALL_H
#include "common/lua/scriptvm.h"
#include "common/lua/scriptbind.h"
#include <sstream>

namespace cxx {
    namespace lua {

        struct call_base {
        protected:
            call_base();
            static int  prepare(ScriptVM* v, const char* name);
            static void onerror(ScriptVM* v, const char* name);
        };

        template<typename R >
        struct script : public call_base {
            static R call(ScriptVM* v, const char* name) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    s.call(0, ret_<R >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
                return pop<R >(v);
            }

            template<typename P1 >
            static R call(ScriptVM* v, const char* name, P1 p1) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    push(v, p1);
                    s.call(1, ret_<R >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
                return pop<R >(v);
            }

            template<typename P1, typename P2 >
            static R call(ScriptVM* v, const char* name, P1 p1, P2 p2) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    push(v, p1);
                    push(v, p2);
                    s.call(2, ret_<R >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
                return pop<R >(v);
            }

            template<typename P1, typename P2, typename P3 >
            static R call(ScriptVM* v, const char* name, P1 p1, P2 p2, P3 p3) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    push(v, p1);
                    push(v, p2);
                    push(v, p3);
                    s.call(3, ret_<R >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
                return pop<R >(v);
            }

            template<typename P1, typename P2, typename P3, typename P4 >
            static R call(ScriptVM* v, const char* name, P1 p1, P2 p2, P3 p3, P4 p4) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    push(v, p1);
                    push(v, p2);
                    push(v, p3);
                    push(v, p4);
                    s.call(4, ret_<R >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
                return pop<R >(v);
            }

            template<typename P1, typename P2, typename P3, typename P4, typename P5 >
            static R call(ScriptVM* v, const char* name, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    push(v, p1);
                    push(v, p2);
                    push(v, p3);
                    push(v, p4);
                    push(v, p5);
                    s.call(5, ret_<R >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
                return pop<R >(v);
            }
        };

        template<>
        struct script<void > : public call_base {
            static void call(ScriptVM* v, const char* name) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    s.call(0, ret_<void >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
            }

            template<typename P1 >
            static void call(ScriptVM* v, const char* name, P1 p1) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    push(v, p1);
                    s.call(1, ret_<void >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
            }

            template<typename P1, typename P2 >
            static void call(ScriptVM* v, const char* name, P1 p1, P2 p2) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    push(v, p1);
                    push(v, p2);
                    s.call(2, ret_<void >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
            }

            template<typename P1, typename P2, typename P3 >
            static void call(ScriptVM* v, const char* name, P1 p1, P2 p2, P3 p3) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    push(v, p1);
                    push(v, p2);
                    push(v, p3);
                    s.call(3, ret_<void >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
            }

            template<typename P1, typename P2, typename P3, typename P4 >
            static void call(ScriptVM* v, const char* name, P1 p1, P2 p2, P3 p3, P4 p4) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    push(v, p1);
                    push(v, p2);
                    push(v, p3);
                    push(v, p4);
                    s.call(4, ret_<void >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
            }

            template<typename P1, typename P2, typename P3, typename P4, typename P5 >
            static void call(ScriptVM* v, const char* name, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
                ScriptStack	s(v);
                int errfunc = prepare(v, name);
                if(s.isfunction(-1)) {
                    push(v, p1);
                    push(v, p2);
                    push(v, p3);
                    push(v, p4);
                    push(v, p5);
                    s.call(5, ret_<void >::value, errfunc);
                }
                else {
                    onerror(v, name);
                }
                s.remove(-2);
            }
        };


    }
}

#endif


