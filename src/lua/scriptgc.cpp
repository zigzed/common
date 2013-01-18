#include "cxxlib/lua/scriptvm.h"
extern "C" {
#include "jit/LuaJIT-2.0.0-beta7/src/lua.h"
#include "jit/LuaJIT-2.0.0-beta7/src/lualib.h"
#include "jit/LuaJIT-2.0.0-beta7/src/lauxlib.h"
//#include "lua/lua.h"
//#include "lua/lualib.h"
//#include "lua/lauxlib.h"
}

namespace cxx {
    namespace lua {

        ScriptGC::ScriptGC(ScriptVM* vm) : vm_(vm)
        {
        }

        void ScriptGC::suspend()
        {
            lua_State* L = vm_->stack();
            lua_gc(L, LUA_GCSTOP, 0);
        }

        void ScriptGC::restart()
        {
            lua_State* L = vm_->stack();
            lua_gc(L, LUA_GCRESTART, 0);
        }

        void ScriptGC::collect()
        {
            lua_State* L = vm_->stack();
            lua_gc(L, LUA_GCCOLLECT, 0);
        }

        size_t ScriptGC::usedmem()
        {
            lua_State* L = vm_->stack();
            return lua_gc(L, LUA_GCCOUNT, 0);
        }

        size_t ScriptGC::remaind()
        {
            lua_State* L = vm_->stack();
            return lua_gc(L, LUA_GCCOUNTB, 0);
        }

        void ScriptGC::destroy()
        {
            delete this;
        }

        ScriptGC::~ScriptGC()
        {
        }

    }
}


