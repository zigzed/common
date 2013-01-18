#include "cxxlib/lua/scriptbind.h"
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

        void push(ScriptVM* v, bool val)
        {
            lua_pushboolean(v->stack(), val);
        }

        void push(ScriptVM* v, char val)
        {
            lua_pushnumber(v->stack(), val);
        }

        void push(ScriptVM* v, unsigned char val)
        {
            lua_pushnumber(v->stack(), val);
        }

        void push(ScriptVM* v, short val)
        {
            lua_pushnumber(v->stack(), val);
        }

        void push(ScriptVM* v, unsigned short val)
        {
            lua_pushnumber(v->stack(), val);
        }

        void push(ScriptVM* v, int val)
        {
            lua_pushnumber(v->stack(), val);
        }

        void push(ScriptVM* v, unsigned int val)
        {
            lua_pushnumber(v->stack(), val);
        }

        void push(ScriptVM* v, long val)
        {
            lua_pushnumber(v->stack(), val);
        }

        void push(ScriptVM* v, unsigned long val)
        {
            lua_pushnumber(v->stack(), val);
        }

        void push(ScriptVM* v, float val)
        {
            lua_pushnumber(v->stack(), val);
        }

        void push(ScriptVM* v, double val)
        {
            lua_pushnumber(v->stack(), val);
        }

        void push(ScriptVM* v, char* val)
        {
            lua_pushstring(v->stack(), val);
        }

        void push(ScriptVM* v, const char* val)
        {
            lua_pushstring(v->stack(), val);
        }

        void push(ScriptVM* v, table tab)
        {
            lua_pushvalue(v->stack(), tab.obj->idx);
        }

        char* 			read(ScriptVM* v, int index, TypeSelect<char* >)
        {
            return (char* )ScriptStack(v).getstring(index);
        }

        const char*		read(ScriptVM* v, int index, TypeSelect<const char* >)
        {
            return ScriptStack(v).getstring(index);
        }
        char			read(ScriptVM* v, int index, TypeSelect<char >)
        {
            return (char)ScriptStack(v).getnumber(index);
        }
        unsigned char	read(ScriptVM* v, int index, TypeSelect<unsigned char >)
        {
            return (unsigned char)ScriptStack(v).getnumber(index);
        }
        short			read(ScriptVM* v, int index, TypeSelect<short >)
        {
            return (short)ScriptStack(v).getnumber(index);
        }
        unsigned short	read(ScriptVM* v, int index, TypeSelect<unsigned short >)
        {
            return (unsigned short)ScriptStack(v).getnumber(index);
        }
        int				read(ScriptVM* v, int index, TypeSelect<int >)
        {
            return (int)ScriptStack(v).getnumber(index);
        }
        unsigned int	read(ScriptVM* v, int index, TypeSelect<unsigned int >)
        {
            return (unsigned int)ScriptStack(v).getnumber(index);
        }
        long			read(ScriptVM* v, int index, TypeSelect<long >)
        {
            return (long)ScriptStack(v).getnumber(index);
        }
        unsigned long	read(ScriptVM* v, int index, TypeSelect<unsigned long >)
        {
            return (unsigned long)ScriptStack(v).getnumber(index);
        }
        float			read(ScriptVM* v, int index, TypeSelect<float >)
        {
            return (float)ScriptStack(v).getnumber(index);
        }
        double			read(ScriptVM* v, int index, TypeSelect<double >)
        {
            return (double)ScriptStack(v).getnumber(index);
        }
        bool			read(ScriptVM* v, int index, TypeSelect<bool >)
        {
            if(ScriptStack(v).isboolean(index))
                return ScriptStack(v).getboolean(index) != 0;
            else
                return ScriptStack(v).getnumber(index) != 0;
        }
        std::string     read(ScriptVM* v, int index, TypeSelect<std::string >)
        {
            ScriptStack s(v);
            return std::string(s.getstring(index), s.objlen(index));
        }

        void			read(ScriptVM* v, int index, TypeSelect<void >)
        {
            return;
        }

        table_obj::table_obj(ScriptVM* v, int i)
            : svm(v), idx(i), ref(0)
        {
            ptr = ScriptStack(v).getpointer(idx);
        }

        table_obj::~table_obj()
        {
            if(validate())
                lua_remove(svm->stack(), idx);
        }

        void table_obj::inc_ref()
        {
            ++ref;
        }

        void table_obj::dec_ref()
        {
            --ref;
        }

        bool table_obj::validate()
        {
            ScriptStack	st(svm);
            if(ptr != NULL) {
                if(ptr == st.getpointer(idx))
                    return true;
                else {
                    int top = st.gettop();
                    for(int i = 1; i <= top; ++i) {
                        if(ptr == st.getpointer(idx)) {
                            idx = i;
                            return true;
                        }
                    }
                    ptr = NULL;
                    return false;
                }
            }
            else
                return false;
        }

        table::table(ScriptVM* vm)
        {
            ScriptStack	st(vm);
            st.newtable();
            obj = new table_obj(vm, st.gettop());
            obj->inc_ref();
        }

        table::table(ScriptVM* vm, const char* name)
        {
            ScriptStack	st(vm);
            st.putstring(name);
            st.gettable(LUA_GLOBALSINDEX);
            if(st.istable(-1)) {
                st.pop(1);
                st.newtable();
                st.putstring(name);
                st.putvalue(-2);
                st.settable(LUA_GLOBALSINDEX);
            }
            obj = new table_obj(vm, st.gettop());
            obj->inc_ref();
        }

        table::table(ScriptVM* vm, int index)
        {
            ScriptStack	st(vm);
            if(index < 0) {
                index = st.gettop() + index + 1;
            }
            obj = new table_obj(vm, index);
            obj->inc_ref();
        }

        table::table(const table& rhs)
        {
            obj = rhs.obj;
            rhs.obj->inc_ref();
        }

        table::~table()
        {
            if(obj) {
                obj->dec_ref();
                if(obj->ref == 0) {
                    delete obj;
                }
            }
        }

        declare::declare(ScriptVM* v) : vm_(v)
        {
        }

        module_::module_(ScriptVM* v) : declare(v)
        {
        }

    }
}

