#include <algorithm>
#include "buffer.h"
#include "unicode.h"

/**
 * All of these are pure functions modulo memory allocations.
 */

static buffer_char opposite_bracket(buffer_char c) {
    switch (c) {
        case '(': return ')';
        case '[': return ']';
        case '{': return '}';
        default: break;
    }
    return '\0';
}

bool buffer_is_at_end_of_line(const buffer& buf) {
    return buf.pos.x == buf.lines[buf.pos.y]->size();
}

bool buffer_is_at_last_line(const buffer& buf) {
    return buf.pos.y == buf.lines.size() - 1;
}

buffer_char buffer_get_char(const buffer& buf, size_t x, size_t y) {
    if (buffer_is_at_end_of_line(buf)) return '\n';
    auto& [lines, pos] = buf;
    auto& line = *lines[pos.y];
    return line[x];
}

buffer buffer_move_next_word(buffer buf) {
    auto& [lines, pos] = buf;
    auto& [x, y] = pos;
    buffer_char ch = buffer_get_char(buf, x, y);
    auto char_class = [](char32_t c) {
        if (is_regex_space(c)) return 0;
        if (is_regex_word(c)) return 1;
        return 2;
    };
    /* advance until the character class changes */
    while (char_class(buffer_get_char(buf, x, y)) == char_class(ch)) {
        if (buffer_is_at_end_of_line(buf) && buffer_is_at_last_line(buf)) {
            return buf;
        } else if (buffer_is_at_end_of_line(buf)) {
            y++;
            x = 0;
        } else {
            x++;
        }
    }
    /* if we ended on a space, recurse to go to next non-space */
    if (is_regex_space(buffer_get_char(buf, x, y))) {
        return buffer_move_next_word(buf);
    }
    return buf;
}

buffer buffer_insert(buffer buf, buffer_char c) {
    auto& [lines, pos] = buf;
    auto& [x, y] = pos;
    x = std::min(lines[y]->size(), x);
    /* clone line if someone else holds a reference to it */
    if (lines[y].use_count() > 1) {
        lines[y] = std::make_shared<buffer_line>(*lines[y]);
    }
    auto& line = *lines[y];
    /* insert opposite bracket, if any */
    if (opposite_bracket(c)) {
        line.insert(x, 1, opposite_bracket(c));
    }
    /* skip if matching right bracket, TODO: improve */
    if (x > 0 && opposite_bracket(line[x-1]) == c
        && x < line.size() && line[x] == c) {
        x++;
        return buf;
    }
    line.insert(x, 1, c);
    x = x + 1;
    return buf;
}

