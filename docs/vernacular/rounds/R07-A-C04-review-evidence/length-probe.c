#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
static void *alloc(void *ud, void *p,size_t old,size_t n) { (void)ud;(void)old;if(!n){free(p);return NULL;}return realloc(p,n); }
int main(void) {
 lua_State *L=lua_newstate(alloc,NULL,0);
 luaL_requiref(L,"_G",luaopen_base,1);lua_pop(L,1);
 luaL_requiref(L,"table",luaopen_table,1);lua_pop(L,1);
 const char *script="local t=table.create(0,128); local i=1; for k=0,60 do t[i]=true;i=i*2 end; local n=0;for _ in pairs(t) do n=n+1 end;return #t,n";
 int status=luaL_loadstring(L,script);if(!status)status=lua_pcall(L,0,2,0);
 if(status){fprintf(stderr,"%s\n",lua_tostring(L,-1));lua_close(L);return 1;}
 printf("length=%lld entries=%lld\n",(long long)lua_tointeger(L,-2),(long long)lua_tointeger(L,-1));lua_close(L);return 0;
}
