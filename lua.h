#pragma once

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>

int lua_line_len(lua_State *L);
int lua_line_sub(lua_State *L);
int lua_line_eq(lua_State *L);
int lua_line_char(lua_State *L);
int lua_line_concat(lua_State *L);
