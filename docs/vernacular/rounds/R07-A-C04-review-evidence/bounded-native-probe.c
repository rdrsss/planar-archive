#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
static unsigned long instructions=0;
static size_t used=0,peak=0;
static void hook(lua_State *L,lua_Debug *ar){(void)L;(void)ar;instructions++;}
static void *alloc(void *ud,void *p,size_t old,size_t n){(void)ud;if(!p)old=0;if(!n){free(p);used-=old;return NULL;}void *q=realloc(p,n);if(q){used=used-old+n;if(used>peak)peak=used;}return q;}
static int probe(const char *operation) {
 instructions=0;used=peak=0;
 lua_State *L=lua_newstate(alloc,NULL,0);
 luaL_requiref(L,"_G",luaopen_base,1);lua_pop(L,1);
 luaL_requiref(L,"table",luaopen_table,1);lua_pop(L,1);
 const char *script="local t=table.create(0,128); local i=1; for k=0,16 do t[i]=true;i=i*2 end;return t";
 int status=luaL_loadstring(L,script);if(!status)status=lua_pcall(L,0,1,0);
 if(status){lua_close(L);return 1;}
 lua_Integer length=lua_rawlen(L,1);
 lua_getglobal(L,"table");lua_getfield(L,-1,operation);lua_remove(L,-2);lua_pushvalue(L,1);
 int args=2;
 if(operation[0]=='s')lua_getglobal(L,"type");
 else {lua_pushinteger(L,1);if(operation[0]=='i'){lua_pushboolean(L,0);args=3;}}
 lua_sethook(L,hook,LUA_MASKCOUNT,1);
 status=lua_pcall(L,args,0,0);
 printf("operation=%s length=%lld stored_initially=17 status=%d vm_instructions=%lu peak_lua_bytes=%zu",operation,(long long)length,status,instructions,peak);
 if(status)printf(" error=%s",lua_tostring(L,-1));
 puts("");lua_close(L);return 0;
}
int main(void){return probe("insert")||probe("remove")||probe("sort");}
