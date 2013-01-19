/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_LUA_SCRIPTVM_H
#define	CXX_LUA_SCRIPTVM_H
#include <string>
#include <stdexcept>
#include <cassert>

struct lua_State;
namespace cxx {
    namespace lua {

        class ScriptVM;
        class ScriptGC;
        class ScriptError : public std::exception {
        public:
            explicit ScriptError(ScriptVM* v);
            ScriptError(ScriptVM* v, const std::string& msg);
            ~ScriptError() throw();
            const char* what() const throw();
        private:
            std::string	what_;
        };

        int			OnError(lua_State* L);
        const char*	get_error_msg(int code);

        class ScriptVM {
        public:
            enum {
                LIB_STRING	= (0x01 << 0),
                LIB_MATH	= (0x01 << 1),
                LIB_IO		= (0x01 << 2),
                LIB_OS		= (0x01 << 3),
                LIB_DEBUG	= (0x01 << 4)
            };
            explicit ScriptVM(int libs) throw(ScriptError);
            ScriptVM(lua_State* state);
            ~ScriptVM();
            inline const lua_State* stack() const { return stack_; }
            inline lua_State*		stack() { return stack_; }
            operator lua_State* () { return stack_; }
            void				interpret(const char* file) throw(ScriptError);
            void				interpret(const char* buf, int len) throw(ScriptError);
            void				pusherror(const char* msg);
            void*				getmemory(size_t size);
            ScriptGC*			collector();
            int                 do_invoke(int nargs, int clear);
            static int          traceback(lua_State* state);
        private:
            ScriptVM(const ScriptVM& rhs);
            ScriptVM& operator= (const ScriptVM& rhs);
            lua_State*	stack_;
            bool		owner_;
        };

        class ScriptGC {
        public:
            /** stop the garbage collector of the script */
            void	suspend();
            /** restart the garbage collector */
            void	restart();
            /** perform a full garbage collection */
            void	collect();
            /** return the current used memory in KBytes for script */
            size_t	usedmem();
            /** return the remaind memory for script */
            size_t	remaind();
            /** release the ScriptGC object */
            void	destroy();
        private:
            friend class ScriptVM;
            explicit ScriptGC(ScriptVM* vm);
            ~ScriptGC();
            ScriptGC(const ScriptGC& rhs);
            ScriptGC& operator= (const ScriptGC& rhs);
            ScriptVM*	vm_;
        };

        class ScriptStack {
        public:
            typedef int (*FuncPtr )(lua_State* );
            explicit inline ScriptStack(ScriptVM* vm) : vm_(vm) {
                assert(vm != 0);
            }
            bool		getboolean (int index);
            long		getinteger (int index);
            double		getnumber  (int index);
            const void*	getpointer (int index);
            const char*	getstring  (int index);
            void*		getuserdata(int index);
            int			gettype    (int index);
            FuncPtr		getfunction(int index);
            int			getupvalidx(int index);

            bool		isuserdata (int index);
            bool		isnull	   (int index);
            bool		istable	   (int index);
            bool		isfunction (int index);
            bool		isboolean  (int index);

            void		putnull    ();
            void		putnumber  (double 		val);
            void		putstring  (const char* val);
            void		putboolean (bool 		val);
            void		putclosure (FuncPtr func, int n);
            void		putuserdata(void*		val);
            void		putmeta	   (const char* val);
            void		putvalue   (int         idx);

            void		setmetatab (int index);
            void		getmetatab (int index);

            void		newtable   ();
            void		settable   (int index);
            void		gettable   (int index);

            void		pop		   (int index);
            void		rawset	   (int index);
            void		rawget	   (int index);
            void		settop	   (int index);
            int			gettop	   ();

            void		remove     (int index);

            void        getglobal  (const char* name);
            void        setglobal  (const char* name);
            size_t      objlen     (int index);

            void		call	   (int nargs, int nrets, int errfunc) throw(ScriptError);
        private:
            ScriptVM*	vm_;
        };

    }
}

#endif


