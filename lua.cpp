#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#include "lua.h"
#include <cstdlib>
#include "utf8.h"
#include <assert.h>
#include <string.h>

static int API_READFILE;
static int API_WRITEFILE;
static int API_TOPIECETABLE;
static int API_FLATTENPIECETABLE;
static int API_STATE;
static int API_MERGESTATES;

lua_State* lua_initialize() {

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // update path
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    char newpath[300] = {0};
    strcat(newpath, lua_tostring(L, -1));
    strcat(newpath, ";");
    strcat(newpath, "lua/piecetable.lua");
    lua_pop(L, 1);
    lua_pushstring(L, newpath);
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);

    // load misc api
    assert(luaL_dofile(L, "lua/misc.lua") == 0);
    lua_getfield(L, -1, "mergestates");
    API_MERGESTATES = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_getfield(L, -1, "initialstate");
    API_STATE = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1); // pop misc api

    // load file api
    assert(luaL_dofile(L, "lua/file.lua") == 0);
    lua_getfield(L, -1, "read");
    API_READFILE = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_getfield(L, -1, "write");
    API_WRITEFILE = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1); // pop file api

    // load piecetable api
    assert(luaL_dofile(L, "lua/piecetable.lua") == 0);
    lua_getfield(L, -1, "topiecetable");
    API_TOPIECETABLE = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_getfield(L, -1, "flattenpiecetable");
    API_FLATTENPIECETABLE = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1); // pop piecetable api

    // load init
    assert(luaL_dofile(L, "config.lua") == 0);

    return L;

}

void lua_load_file(lua_State* L, const char* filename) {

    lua_rawgeti(L, LUA_REGISTRYINDEX, API_STATE);

    lua_pushstring(L, "buffer");
    lua_rawgeti(L, LUA_REGISTRYINDEX, API_TOPIECETABLE);
    lua_rawgeti(L, LUA_REGISTRYINDEX, API_READFILE);
    lua_pushstring(L, filename);
    assert(lua_pcall(L, 1, 1, 0) == 0);
    assert(lua_pcall(L, 1, 1, 0) == 0);
    lua_settable(L, -3);

}

// assumes that the new state is on the stack
void lua_update_state(lua_State* L) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, API_MERGESTATES);
    lua_insert(L, -2); 
    lua_rawgeti(L, LUA_REGISTRYINDEX, API_STATE);
    lua_insert(L, -2); 
    assert(lua_pcall(L, 2, 0, 0) == 0);
}

void lua_push_state(lua_State* L) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, API_STATE);
}

void lua_state_to_editor(lua_State* L, editor& e) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, API_STATE);

    // buffer
    lua_getfield(L, -1, "buffer");
    lua_rawgeti(L, LUA_REGISTRYINDEX, API_FLATTENPIECETABLE);
    lua_insert(L, -2);
    assert(lua_pcall(L, 1, 1, 0) == 0);
    // now there is a table of strings on the stack
    assert(lua_istable(L, -1));
    // traverse table of strings
    lua_pushnil(L);
    e.lines = {};
    while (lua_next(L, -2) != 0) {
        e.lines.push_back(utf8_decode(std::string(lua_tolstring(L, -1, NULL))));
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop table of strings

    // x
    lua_getfield(L, -1, "x");
    e.pos.x = lua_tointeger(L, -1);
    lua_pop(L, 1);

    // y
    lua_getfield(L, -1, "y");
    e.pos.y = lua_tointeger(L, -1);
    lua_pop(L, 1);

    // exiting
    lua_getfield(L, -1, "exiting");
    e.exiting = lua_toboolean(L, -1);
    lua_pop(L, 1);

    // mode
    lua_getfield(L, -1, "mode");
    e.mode = lua_tointeger(L, -1);
    lua_pop(L, 1);

    // pop state
    lua_pop(L, 1);

}
