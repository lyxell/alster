#pragma once

#include "editor.h"

lua_State* lua_initialize();
void lua_initialize_state(lua_State* L, const char* filename);
void lua_update_state(lua_State* L);
void lua_push_state(lua_State* L);
void lua_state_to_editor(lua_State* L, editor& ed);
