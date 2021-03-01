#include "buffer.h"
#include "lua.h"
#include "utf8.h"
#include <cassert>
#include <cstring>
#include <algorithm>

/**
 * Copies the elements in the range, defined by [first, last)
 */
template<typename It>
static void create_line(lua_State *L, It first, It last) {
    assert(last >= first);
    size_t length = last - first;
    size_t num_bytes = sizeof(buffer_char) * (length + 1);
    auto line = (buffer_char*) lua_newuserdata(L, num_bytes);
    line[0] = (buffer_char) length;
    std::copy(first, last, line + 1);
    luaL_newmetatable(L, "line");
    lua_pushstring(L, "__len");
    lua_pushcfunction(L, lua_line_len);
    lua_settable(L, -3);
    lua_pushstring(L, "__eq");
    lua_pushcfunction(L, lua_line_eq);
    lua_settable(L, -3);
    lua_pushstring(L, "__concat");
    lua_pushcfunction(L, lua_line_concat);
    lua_settable(L, -3);
    lua_pushstring(L, "__index");
    lua_getglobal(L, "line");
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
}

int lua_line_concat(lua_State *L) {
    assert(lua_gettop(L) == 2);
    assert(lua_isuserdata(L, -1));
    assert(lua_isuserdata(L, -2));
    auto l1 = (buffer_char*) lua_touserdata(L, -1);
    auto l2 = (buffer_char*) lua_touserdata(L, -2);
    auto res = buffer_line(l2 + 1, l2[0]) + buffer_line(l1 + 1, l1[0]);
    create_line(L, res.begin(), res.end());
    return 1; 
}

int lua_line_eq(lua_State *L) {
    assert(lua_gettop(L) == 2);
    assert(lua_isuserdata(L, -1));
    assert(lua_isuserdata(L, -2));
    auto l1 = (buffer_char*) lua_touserdata(L, -1);
    auto l2 = (buffer_char*) lua_touserdata(L, -2);
    if (l1[0] != l2[0]) {
        lua_pushboolean(L, 0);
        return 1; 
    }
    lua_pushboolean(L, std::memcmp(l1+1, l2+1, l1[0]*sizeof(buffer_char)) == 0);
    return 1; 
}

int lua_line_sub(lua_State *L) {
    buffer_char *line;
    long skip;
    long length;
    if (lua_gettop(L) == 3) {
        assert(lua_isnumber(L, -1));
        assert(lua_isnumber(L, -2));
        assert(lua_isuserdata(L, -3));
        line = (buffer_char*) lua_touserdata(L, -3);
        skip = lua_tointeger(L, -2) - 1;
        length = lua_tointeger(L, -1) - lua_tointeger(L, -2) + 1;
    } else {
        assert(lua_isnumber(L, -1));
        assert(lua_isuserdata(L, -2));
        line = (buffer_char*) lua_touserdata(L, -2);
        skip = lua_tointeger(L, -1) - 1;
        length = line[0] - skip;
    }
    buffer_char* start = line + 1 + skip;
    assert(start >= line + 1);
    create_line(L, start, start + length);
    return 1; 
}

int lua_line_char(lua_State *L) {
    std::string res;
    while (lua_gettop(L) > 0) {
        assert(lua_isnumber(L, -1));
        auto c = lua_tointeger(L, -1);
        res.push_back((char) c);
        lua_pop(L, 1);
    }
    std::reverse(res.begin(), res.end());
    buffer_line b = utf8_decode(res);
    create_line(L, b.begin(), b.end());
    return 1; 
}

int lua_lines_insert(lua_State *L) {
    assert(lua_gettop(L) == 3);
    assert(lua_isuserdata(L, -1));
    assert(lua_isnumber(L, -2));
    assert(lua_isuserdata(L, -3));
    auto line = (buffer_char*) lua_touserdata(L, -1);
    auto line_index = lua_tointeger(L, -2) - 1;
    auto& lines = **((buffer_lines**) lua_touserdata(L, -3));
    lines.insert(lines.begin() + line_index, std::make_shared<buffer_line>(
        buffer_line(line + 1, line[0])));
    return 0;
}

int lua_lines_remove(lua_State *L) {
    assert(lua_gettop(L) == 2);
    assert(lua_isnumber(L, -1));
    assert(lua_isuserdata(L, -2));
    auto line_index = lua_tointeger(L, -1) - 1;
    auto& lines = **((buffer_lines**) lua_touserdata(L, -2));
    lines.erase(lines.begin() + line_index);
    return 0;
}

int lua_lines_len(lua_State *L) {
    assert(lua_gettop(L) == 2);
    assert(lua_isnil(L, -1));
    assert(lua_isuserdata(L, -2));
    auto& lines = **((buffer_lines**) lua_touserdata(L, -2));
    lua_pushinteger(L, (int) lines.size());
    return 1;
}

int lua_line_len(lua_State *L) {
    assert(lua_gettop(L) == 2);
    assert(lua_isnil(L, -1));
    assert(lua_isuserdata(L, -2));
    auto line = (buffer_char*) lua_touserdata(L, -2);
    lua_pushinteger(L, (int) line[0]);
    return 1;
}

int lua_lines_newindex(lua_State *L) {
    assert(lua_gettop(L) == 3);
    assert(lua_isuserdata(L, -1));
    assert(lua_isnumber(L, -2));
    assert(lua_isuserdata(L, -3));
    auto line = (buffer_char*) lua_touserdata(L, -1);
    auto line_index = lua_tointeger(L, -2) - 1;
    auto& lines = **((buffer_lines**) lua_touserdata(L, -3));
    lines[line_index] = std::make_shared<buffer_line>(line + 1, line[0]);
    return 0;
}

int lua_lines_index(lua_State *L) {
    assert(lua_gettop(L) == 2);
    assert(lua_isnumber(L, -1));
    assert(lua_isuserdata(L, -2));
    long int line_index = lua_tointeger(L, -1) - 1;
    if (line_index < 0) {
        lua_pushnil(L);
        return 1;
    }
    auto& lines = **((buffer_lines**) lua_touserdata(L, -2));
    create_line(L, lines[line_index]->begin(), lines[line_index]->end());
    return 1;
}

