#include "cxxlib/lua/scriptcall.h"
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

        call_base::call_base()
        {
        }

        int call_base::prepare(ScriptVM* v, const char* name)
        {
            ScriptStack	s(v);
            s.putclosure(OnError, 0);
            int errfunc = s.gettop();
            s.putstring(name);
            s.gettable(LUA_GLOBALSINDEX);
            return errfunc;
        }

        void call_base::onerror(ScriptVM* v, const char* name)
        {
            std::ostringstream	str;
            str << "attempt to call global \'" << name << "\' (not a function)";
            throw ScriptError(v, str.str());
        }

    }
}



