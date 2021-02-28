#include "buffer.h"
#include "lua.h"
#include "utf8.h"
#include <cassert>

template<typename It>
static void create_line(lua_State *L, It start, It end) {
    size_t length = end - start;
    size_t num_bytes = sizeof(buffer_char) * (length + 1);
    auto line = (buffer_char*) lua_newuserdata(L, num_bytes);
    line[0] = (buffer_char) length;
    std::copy(start, end, line + 1);
    lua_newtable(L);
    lua_pushstring(L, "__len");
    lua_pushcfunction(L, lua_line_len);
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

int lua_line_sub(lua_State *L) {
    buffer_char *line;
    long start;
    long end;
    if (lua_gettop(L) == 3) {
        assert(lua_isnumber(L, -1));
        assert(lua_isnumber(L, -2));
        assert(lua_isuserdata(L, -3));
        line = (buffer_char*) lua_touserdata(L, -3);
        start = lua_tointeger(L, -2) - 1;
        end = lua_tointeger(L, -1) - 1;
    } else {
        assert(lua_isnumber(L, -1));
        assert(lua_isuserdata(L, -2));
        line = (buffer_char*) lua_touserdata(L, -2);
        start = lua_tointeger(L, -1) - 1;
        end = line[0] - 1;
    }
    create_line(L, line + 1 + start, line + 1 + end + 1);
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
    return 0; /* number of results */
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
    assert(lua_gettop(L) == 3);     // number of arguments
    assert(lua_isuserdata(L, -1));  // value
    assert(lua_isnumber(L, -2)); // key
    assert(lua_isuserdata(L, -3)); // table
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
    auto& lines = **((buffer_lines**) lua_touserdata(L, -2));
    create_line(L, lines[line_index]->begin(), lines[line_index]->end());
    return 1;
}

