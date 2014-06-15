#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>
#include "merc.h"

/* gcc -o luadd luadd.c -I/usr/local/include -L/usr/local/lib -llua -lm -ldl */

/* the Lua interpreter */
lua_State* state;

int luaadd ( int x, int y )
{
        int sum;

        /* the function name */
        lua_getglobal(state, "add");

        /* the first argument */
        lua_pushnumber(state, x);

        /* the second argument */
        lua_pushnumber(state, y);

        /* call the function with 2 arguments, return 1 result */

        lua_call(state, 2, 1);

        /* get the result */
        sum = (int)lua_tointeger(state, -1);

        lua_pop(state, 1);

        return sum;
}

int lua_lua (CHAR_DATA *ch)
{
        int sum;
		char buf[MSL];
		int x = 10;
		int y = 5;

        gen_call_lua("add.lua", "add", "ii>i", x, y, &sum);
        /* print the result */
        sprintf( buf, "The sum is %d\n", sum );
		send_to_char(buf, ch);
		lua_pop(state, 1);

        /* cleanup Lua */
        lua_close(state);

        /* pause */
        sprintf(buf, "Press enter to exit..." );
		send_to_char(buf, ch);
        return 0;
}
    
void gen_call_lua (const char *file, const char *func, const char *sig, ...) 
{
      va_list vl;
      int narg, nres;  /* number of arguments and results */
		
		state = luaL_newstate();
		luaL_openlibs(state);
		luaL_dofile(state, file);
		va_start(vl, sig);
		lua_getglobal(state, func);  /* get function */
    
      /* push arguments */
		  narg = 0;
		  while (*sig) {  /* push arguments */
			switch (*sig++) {
		
			  case 'd':  /* double argument */
				lua_pushnumber(state, va_arg(vl, double));
				break;
		
			  case 'i':  /* int argument */
				lua_pushnumber(state, va_arg(vl, int));
				break;
		
			  case 's':  /* string argument */
				lua_pushstring(state, va_arg(vl, char *));
				break;
		
			  case '>':
				goto endwhile;
		
			  default:
				error(state, "invalid option (%c)", *(sig - 1));
			}
			narg++;
			luaL_checkstack(state, 1, "too many arguments");
		  } endwhile:
		
		  /* do the call */
		  nres = strlen(sig);  /* number of expected results */
		  if (lua_pcall(state, narg, nres, 0) != 0)  /* do the call */
			error(state, "error running function `%s': %s",
					 func, lua_tostring(state, -1));
		
		  /* retrieve results */
		  nres = -nres;  /* stack index of first result */
		  while (*sig) {  /* get results */
			switch (*sig++) {
		
			  case 'd':  /* double result */
				if (!lua_isnumber(state, nres))
				  error(state, "wrong result type");
				*va_arg(vl, double *) = lua_tonumber(state, nres);
				break;
		
			  case 'i':  /* int result */
				if (!lua_isnumber(state, nres))
				  error(state, "wrong result type");
				*va_arg(vl, int *) = (int)lua_tonumber(state, nres);
				break;
		
			  case 's':  /* string result */
				if (!lua_isstring(state, nres))
				  error(state, "wrong result type");
				*va_arg(vl, const char **) = lua_tostring(state, nres);
				break;
		
			  default:
				error(state, "invalid option (%c)", *(sig - 1));
			}
			nres++;
		  }
		  va_end(vl);
}