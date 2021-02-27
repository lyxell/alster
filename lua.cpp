#include "buffer.h"
#include "lua.h"
#include <cassert>

int lua_line_sub(lua_State *L) {
    assert(lua_gettop(L) == 3); /* number of arguments */
    assert(lua_isnumber(L, -1));
    assert(lua_isnumber(L, -2));
    long int from = lua_tointeger(L, -2) - 1; // lua 1-indexed
    long int to = lua_tointeger(L, -1) - 1; // lua 1-indexed
    size_t len = to - from + 1;
    assert(len >= 0);
    assert(lua_isuserdata(L, -3));
    buffer_char* line = (buffer_char*) lua_touserdata(L, -3);
    buffer_char* subline = (buffer_char*) lua_newuserdata(L, sizeof(buffer_char) * len + 1);
    std::copy(line + 1 + from, line + 1 + from + len, subline + 1);
    subline[0] = (buffer_char) len;
    lua_newtable(L);
    lua_pushstring(L, "__len");
    lua_pushcfunction(L, lua_line_len);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
    return 1; 
}

int lua_line_create(lua_State *L) {
    buffer_char* line = (buffer_char*) lua_newuserdata(L, sizeof(buffer_char));
    line[0] = 0;
    lua_newtable(L);
    lua_pushstring(L, "__len");
    lua_pushcfunction(L, lua_line_len);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
    return 1; 
}

int lua_lines_insert(lua_State *L) {
    // TODO: Why is this value 2? Shouldn't it be 1?
    assert(lua_gettop(L) == 3); /* number of arguments */
    assert(lua_isuserdata(L, -1)); // value
    assert(lua_isnumber(L, -2)); // pos
    assert(lua_isuserdata(L, -3)); // lines
    buffer_char* line = (buffer_char*) lua_touserdata(L, -1);
    long int pos = lua_tointeger(L, -2) - 1;
    buffer_lines& lines = **((buffer_lines**) lua_touserdata(L, -3));
    lines.insert(lines.begin() + pos, std::make_shared<buffer_line>(
        buffer_line(line + 1, line[0])));
    return 0; /* number of results */
}

int lua_lines_len(lua_State *L) {
    // TODO: Why is this value 2? Shouldn't it be 1?
    assert(lua_gettop(L) == 2); /* number of arguments */
    assert(lua_isuserdata(L, -2));
    buffer_lines** lines = (buffer_lines**) lua_touserdata(L, -2);
    lua_pushnumber(L, (int) (*lines)->size());
    return 1; /* number of results */
}

int lua_line_len(lua_State *L) {
    // TODO: Why is this value 2? Shouldn't it be 1?
    assert(lua_gettop(L) == 2); /* number of arguments */
    assert(lua_isuserdata(L, -2));
    buffer_char* line = (buffer_char*) lua_touserdata(L, -2);
    lua_pushnumber(L, (int) line[0]);
    return 1; /* number of results */
}

int lua_lines_newindex(lua_State *L) {
    assert(lua_gettop(L) == 3); /* number of arguments */
    assert(lua_isuserdata(L, -1)); // value
    assert(lua_isnumber(L, -2)); // key
    assert(lua_isuserdata(L, -3)); // table
    // get the actual line index
    long int idx = lua_tointeger(L, -2) - 1; // subtract 1, lua is 1-indexed
    // extract the pointer to the lines from the user data
    buffer_lines& lines = **((buffer_lines**) lua_touserdata(L, -3));
    // extract the pointer to the string in memory
    buffer_char* mem = (buffer_char*) lua_touserdata(L, -1);
    lines[idx] = std::make_shared<buffer_line>(mem + 1, mem[0]);
    return 0;
}

int lua_lines_index(lua_State *L) {
    assert(lua_gettop(L) == 2); /* number of arguments */
    assert(lua_isnumber(L, -1));
    assert(lua_isuserdata(L, -2));
    // get the actual line index
    long int idx = lua_tointeger(L, -1) - 1; // subtract 1, lua is 1-indexed
    // extract the pointer to the lines from the user data
    buffer_lines& lines = **((buffer_lines**) lua_touserdata(L, -2));
    // create line and metatable
    buffer_line& line = *lines[idx];
    // allocate space for the string and one extra slot for size
    buffer_char* mem = (buffer_char*) lua_newuserdata(L, sizeof(buffer_char) * line.size() + 1);
    std::copy(line.begin(), line.end(), mem + 1);
    mem[0] = (buffer_char) line.size();
    lua_newtable(L);
    lua_pushstring(L, "__len");
    lua_pushcfunction(L, lua_line_len);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
    return 1; /* number of results */
}

