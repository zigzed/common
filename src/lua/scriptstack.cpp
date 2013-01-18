#include "cxxlib/lua/scriptvm.h"
extern "C" {
#include "jit/LuaJIT-2.0.0-beta7/src/lua.h"
#include "jit/LuaJIT-2.0.0-beta7/src/lualib.h"
#include "jit/LuaJIT-2.0.0-beta7/src/lauxlib.h"
//#include "lua/lua.h"
//#include "lua/lualib.h"
//#include "lua/lauxlib.h"
}
#include <cassert>
#include <sstream>

namespace cxx {
    namespace lua {

        bool ScriptStack::getboolean(int index)
        {
            return lua_toboolean(vm_->stack(), index) != 0;
        }

        long ScriptStack::getinteger(int index)
        {
            return lua_tointeger(vm_->stack(), index);
        }

        double ScriptStack::getnumber(int index)
        {
            return lua_tonumber(vm_->stack(), index);
        }

        const void* ScriptStack::getpointer(int index)
        {
            return lua_topointer(vm_->stack(), index);
        }

        const char* ScriptStack::getstring(int index)
        {
            return lua_tostring(vm_->stack(), index);
        }

        void* ScriptStack::getuserdata(int index)
        {
            return lua_touserdata(vm_->stack(), index);
        }

        int ScriptStack::gettype(int index)
        {
            return lua_type(vm_->stack(), index);
        }

        ScriptStack::FuncPtr ScriptStack::getfunction(int index)
        {
            return lua_tocfunction(vm_->stack(), index);
        }

        int ScriptStack::getupvalidx(int index)
        {
            return lua_upvalueindex(index);
        }

        bool ScriptStack::isuserdata(int index)
        {
            return lua_isuserdata(vm_->stack(), index) != 0;
        }

        bool ScriptStack::isfunction(int index)
        {
            return lua_isfunction(vm_->stack(), index) != 0;
        }

        bool ScriptStack::istable(int index)
        {
            return lua_istable(vm_->stack(), index) != 0;
        }

        bool ScriptStack::isnull(int index)
        {
            return lua_isnil(vm_->stack(), -1) != 0;
        }

        bool ScriptStack::isboolean(int index)
        {
            return lua_isboolean(vm_->stack(), -1) != 0;
        }

        void ScriptStack::putnull()
        {
            lua_pushnil(vm_->stack());
        }

        void ScriptStack::putnumber(double val)
        {
            lua_pushnumber(vm_->stack(), val);
        }

        void ScriptStack::putstring(const char* val)
        {
            lua_pushstring(vm_->stack(), val);
        }

        void ScriptStack::putboolean(bool val)
        {
            lua_pushboolean(vm_->stack(), val);
        }

        void ScriptStack::putclosure(FuncPtr func, int n)
        {
            lua_pushcclosure(vm_->stack(), func, n);
        }

        void ScriptStack::putuserdata(void* val)
        {
            lua_pushlightuserdata(vm_->stack(), val);
        }

        void ScriptStack::putmeta(const char* val)
        {
            lua_pushstring(vm_->stack(), val);
            lua_gettable(vm_->stack(), LUA_GLOBALSINDEX);
        }

        void ScriptStack::putvalue(int idx)
        {
            lua_pushvalue(vm_->stack(), idx);
        }

        void ScriptStack::setmetatab(int index)
        {
            lua_setmetatable(vm_->stack(), index);
        }

        void ScriptStack::getmetatab(int index)
        {
            lua_getmetatable(vm_->stack(), index);
        }

        void ScriptStack::pop(int index)
        {
            lua_pop(vm_->stack(), index);
        }

        void ScriptStack::newtable()
        {
            lua_newtable(vm_->stack());
        }

        void ScriptStack::settable(int index)
        {
            lua_settable(vm_->stack(), index);
        }

        void ScriptStack::gettable(int index)
        {
            lua_gettable(vm_->stack(), index);
        }

        void ScriptStack::rawset(int index)
        {
            lua_rawset(vm_->stack(), index);
        }

        void ScriptStack::rawget(int index)
        {
            lua_rawget(vm_->stack(), index);
        }

        void ScriptStack::remove(int index)
        {
            lua_remove(vm_->stack(), index);
        }

        void ScriptStack::settop(int index)
        {
            lua_settop(vm_->stack(), index);
        }

        int ScriptStack::gettop()
        {
            return lua_gettop(vm_->stack());
        }

        void ScriptStack::getglobal(const char *name)
        {
            return lua_getglobal(vm_->stack(), name);
        }

        void ScriptStack::setglobal(const char *name)
        {
            return lua_setglobal(vm_->stack(), name);
        }

        size_t ScriptStack::objlen(int index)
        {
            return lua_objlen(vm_->stack(), index);
        }

        void ScriptStack::call(int nargs, int nrets, int errfunc) throw(ScriptError)
        {
            int status = lua_pcall(vm_->stack(), nargs, nrets, errfunc);
            if(status != 0) {
                lua_pop(vm_->stack(), 1);
                std::ostringstream	msg;
                msg << "invoking function failed: " << get_error_msg(status);
                throw ScriptError(vm_, msg.str());
            }
//            int status = vm_->do_invoke(nargs, nrets);
//            if(status != 0) {
//                throw ScriptError(vm_);
//            }
        }

    }
}

