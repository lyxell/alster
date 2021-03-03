#pragma once

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>

int lua_lines_len(lua_State *L);
int lua_line_len(lua_State *L);
int lua_lines_index(lua_State *L);
int lua_lines_newindex(lua_State *L);
int lua_lines_insert(lua_State *L);
int lua_line_sub(lua_State *L);
int lua_line_eq(lua_State *L);
int lua_line_char(lua_State *L);
int lua_line_concat(lua_State *L);
int lua_lines_remove(lua_State *L);
int lua_buffer_sub(lua_State *L);
int lua_buffer_len(lua_State *L);
int lua_buffer_concat(lua_State *L);
