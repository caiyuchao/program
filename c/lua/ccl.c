/*
 * call lua from c
 */

#include <stdio.h>
#include <string.h>
#include "lua5.1/lua.h"
#include "lua5.1/lualib.h"
#include "lua5.1/lauxlib.h"

int main()
{
    lua_State *L;
    int sum = 0;
	int sub = 0;
	char hostname[32];

    L = lua_open();
    luaL_openlibs(L);
    luaL_dofile(L, "func.lua");
    lua_getglobal(L, "add");  /* call add*/
    lua_pushnumber(L, 1);
    lua_pushnumber(L, 2);
    lua_call(L, 2, 1);
    sum = lua_tonumber(L, 1);
    lua_pop(L, 1);
    printf("%d\n", sum);

    lua_getglobal(L, "add_sub");  /* call add_sub */
    lua_pushnumber(L, 1);
    lua_pushnumber(L, 2);
    lua_call(L, 2, 2);
    sum = lua_tonumber(L, 1);
    sub = lua_tonumber(L, 2);
    lua_pop(L, 2);
    printf("%d, %d\n", sum, sub);

	lua_getglobal(L, "get_hostname");
	lua_pushstring(L, "hello ");
	lua_pcall(L, 1, 1, 0);
	strncpy(hostname, lua_tostring(L,1), 32);
	lua_pop(L, 1);

    
    lua_close(L);
	printf("hostname:%s\n", hostname);
    return 0;
}
