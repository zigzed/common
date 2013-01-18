#include "cxxlib/lua/scriptutil.h"
#include "cxxlib/lua/scriptbind.h"
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

namespace cxx {
    namespace lua {

        static void invoke_parent(ScriptVM* v)
        {
            ScriptStack	s(v);
            s.putstring("__parent");
            s.rawget(-2);
            if(s.istable(-1)) {
                s.putvalue(2);
                s.rawget(-2);
                if(!s.isnull(-1)) {
                    s.remove(-2);
                }
                else {
                    s.remove(-1);
                    invoke_parent(v);
                    s.remove(-2);
                }
            }
        }

        void class_helper::setup(ScriptVM* v, const char* name)
        {
            ScriptStack	s(v);
            s.putstring(name);
            s.newtable();

            s.putstring("__name");
            s.putstring(name);
            s.rawset(-3);

            s.putstring("__index");
            s.putclosure(meta_get, 0);
            s.rawset(-3);

            s.putstring("__newindex");
            s.putclosure(meta_set, 0);
            s.rawset(-3);
        }

        int class_helper::meta_get(lua_State* L)
        {
            ScriptVM	v(L);
            ScriptStack	s(&v);
            s.getmetatab(1);
            s.putvalue(2);
            s.rawget(-2);

            if(s.isuserdata(-1)) {
                user2type<var_base* >::invoke(&v, -1)->get(&v);
                s.remove(-2);
            }
            else if(s.isnull(-1)) {
                s.remove(-1);
                invoke_parent(&v);
                if(s.isnull(-1)) {
                    std::ostringstream	str;
                    str << "can't find class \'" << s.getstring(2) << "\' variable. "
                        << "(forgot registering class variable?)";
                    v.pusherror(str.str().c_str());
                }
            }
            s.remove(-2);
            return 1;
        }

        int class_helper::meta_set(lua_State* L)
        {
            ScriptVM	v(L);
            ScriptStack	s(&v);
            s.getmetatab(1);
            s.putvalue(2);
            s.rawget(-2);

            if(s.isuserdata(-1)) {
                user2type<var_base* >::invoke(&v, -1)->set(&v);
            }
            else if(s.isnull(-1)) {
                s.putvalue(2);
                s.putvalue(3);
                s.rawset(-4);
            }

            s.settop(3);
            return 0;
        }

        void class_helper::meta_push(ScriptVM* v, const char* name)
        {
            ScriptStack	s(v);
            s.putstring(name);
            s.gettable(LUA_GLOBALSINDEX);
        }

    }
}



