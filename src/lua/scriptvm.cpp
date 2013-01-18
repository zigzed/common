#include "cxxlib/lua/scriptvm.h"
extern "C" {
#include "jit/LuaJIT-2.0.0-beta7/src/lua.h"
#include "jit/LuaJIT-2.0.0-beta7/src/lualib.h"
#include "jit/LuaJIT-2.0.0-beta7/src/lauxlib.h"
//#include "lua/lua.h"
//#include "lua/lualib.h"
//#include "lua/lauxlib.h"
}
#include <sstream>
#include <iomanip>
#include <string.h>


namespace cxx {
    namespace lua {

        static const char* error_msg[] = {
            "OK",
            "script yield failed",						// LUA_YIELD
            "script runtime error",						// LUA_ERRRUN
            "script syntax error",						// LUA_ERRSYNTAX
            "memory allocation error",					// LUA_ERRMEM
            "error while running the error handling"	// LUA_ERRERR
        };

        const char* get_error_msg(int code)
        {
            int elems = sizeof(error_msg)/sizeof(error_msg[0]);
            if(code < 0 || code >= elems) {
                return "unknown error code";
            }
            return error_msg[code];
        }

        ScriptError::ScriptError(ScriptVM* v)
        {
            const char* s = 0;
            if(v != 0) {
                s = ScriptStack(v).getstring(-1);
                if(s) {
                    what_ = s;
                }
                lua_pop(v->stack(), 1);
            }
            else {
                what_ = "failed with non-initialized VM";
            }
        }

        ScriptError::ScriptError(ScriptVM* v, const std::string& msg)
        {
            const char* s = 0;
            if(v != 0) {
                s = ScriptStack(v).getstring(-1);
                if(s) {
                    what_ = s;
                    what_ += ": ";
                }
                lua_pop(v->stack(), 1);
            }
            else {
                what_ = "failed with non-initialized VM";
            }
            what_ += msg;
        }

        const char* ScriptError::what() const throw()
        {
            return what_.c_str();
        }

        ScriptError::~ScriptError() throw()
        {
        }

        static void print_error(lua_State* L, const char* fmt, ...)
        {
            char text[4096];

            va_list args;
            va_start(args, fmt);
            vsprintf(text, fmt, args);
            va_end(args);

            lua_pushstring(L, "_ALERT");
            lua_gettable(L, LUA_GLOBALSINDEX);
            if(lua_isfunction(L, -1)) {
                lua_pushstring(L, text);
                lua_call(L, 1, 0);
            }
            else {
                lua_pushstring(L, text);
            }
        }

        static void show_stack(lua_State* st, int n)
        {
            lua_Debug	ar;
            if(lua_getstack(st, n, &ar) == 1) {
                lua_getinfo(st, "nSlu", &ar);
                const char* indent;
                if(n == 0) {
                    indent = "->\t";
                    print_error(st, "\t<call stack>");
                }
                else {
                    indent = "\t";
                }
                if(ar.name)
                    print_error(st, "%s%s(): line %d [%s: line %d]",
                                indent, ar.name, ar.currentline, ar.source,
                                ar.linedefined);
                else
                    print_error(st, "%sunknown: line %d [%s: line %d]",
                                indent, ar.currentline, ar.source,
                                ar.linedefined);

                show_stack(st, n + 1);
            }
        }

        int OnError(lua_State* L)
        {
//            lua_Debug			d;
//            std::ostringstream 	msg;
//            if(lua_getstack(L, 1, &d) == 1) {
//                lua_getinfo(L, "Sln", &d);
//                msg << "src: " << d.short_src << " line: " << d.currentline << " ";
//                if(d.name != 0) {
//                    msg << "(" << d.namewhat << " " << d.name << ")";
//                }
//            }
//            const char* err = lua_tostring(L, -1);
//            lua_pop(L, 1);
//            if(err) {
//                msg << " " << err;
//            }
//            lua_pushstring(L, msg.str().c_str());

            return 1;
        }


        ScriptVM::ScriptVM(int libs)  throw(ScriptError) : stack_(0), owner_(true)
        {
            stack_ = luaL_newstate();
            if(stack_ == 0) {
                throw ScriptError(0, "can not create script VM");
            }
            luaL_openlibs(stack_);
        }

        ScriptVM::ScriptVM(lua_State* state) : stack_(state), owner_(false)
        {
        }

        ScriptVM::~ScriptVM()
        {
            if(stack_ && owner_) {
                lua_close(stack_);
            }
        }

        void ScriptVM::interpret(const char* file)  throw(ScriptError)
        {
            int status = luaL_loadfile(stack_, file) || do_invoke(0, 0);
            if(status != 0) {
                throw ScriptError(this);
            }
        }

        void ScriptVM::interpret(const char* buf, int len)  throw(ScriptError)
        {
            if(len == -1) {
                len = strlen(buf);
            }
            int status = luaL_loadbuffer(stack_, buf, len, "cxx::lua::vm") || do_invoke(0, 0);
            if(status != 0) {
                throw ScriptError(this);
            }
        }

        void* ScriptVM::getmemory(size_t size)
        {
            return lua_newuserdata(stack_, size);
        }

        void ScriptVM::pusherror(const char* msg)
        {
            lua_Debug ar;
            lua_getstack(stack_, 0, &ar);
            lua_getinfo(stack_, "n", &ar);
            if (ar.name == NULL)
                ar.name = "?";
            luaL_error(stack_, "assert fail: %s `%s' (%s)", ar.namewhat, ar.name, msg);
        }

        ScriptGC* ScriptVM::collector()
        {
            return new ScriptGC(this);
        }

        int ScriptVM::traceback(lua_State *state)
        {
            // 'message' is not a string?
            if(!lua_isstring(state, 1)) {
                return 1;
            }
            lua_getfield(state, LUA_GLOBALSINDEX, "debug");
            if(!lua_istable(state, -1)) {
                lua_pop(state, 1);
                return 1;
            }
            lua_getfield(state, -1, "traceback");
            if(!lua_isfunction(state, -1)) {
                lua_pop(state, 2);
                return 1;
            }
            // pass error message
            lua_pushvalue(state, 1);
            // skip this function and traceback
            lua_pushinteger(state, 2);
            // call debug.traceback
            lua_call(state, 2, 1);
            return 1;
        }

        int ScriptVM::do_invoke(int nargs, int clear)
        {
            int status = 0;
            lua_State* state = stack_;
            // get function index
            int base = lua_gettop(state) - nargs;
            // push traceback function
            lua_pushcfunction(state, traceback);
            lua_insert(state, base);
            status = lua_pcall(state, nargs, (clear ? 0 : LUA_MULTRET), base);
            lua_remove(state, base);
            // force a complete garbage collection in case of error
            if(status != 0) {
                lua_gc(state, LUA_GCCOLLECT, 0);
            }
            return status;
        }

    }
}


