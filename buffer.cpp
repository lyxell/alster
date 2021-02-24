#include <algorithm>
#include "buffer.h"
#include "unicode.h"

/**
 * All of these are pure functions modulo memory allocations.
 */

/**
 * Report whether we have an underflow, overflow or
 * a balance of brackets on a line.
 */
static int bracket_balance(const buffer_line& line) {
    int count = 0;
    for (auto c : line) {
        if (c == '(' || c == '[' || c == '{') {
            count++;
        } else if (count > 0 && (c == ')' || c == ']' || c == '}')) {
            count--;
        }
    }
    return count;
}

static int indentation_level(const buffer_line& line) {
    int count = 0;
    for (auto c : line) {
        if (c == ' ') count++;
        else break;
    }
    return count;
}

static buffer_char opposite_bracket(buffer_char c) {
    switch (c) {
        case '(': return ')';
        case '[': return ']';
        case '{': return '}';
        default: break;
    }
    return '\0';
}

buffer buffer_move_start(buffer buf) {
    return { 
        std::move(buf.lines),
        {0, 0}
    };
}

buffer buffer_move_end(buffer buf) {
    auto y = buf.lines.size() - 1;
    return {
        std::move(buf.lines),
        {buf.pos.x, y}
    };
}

buffer buffer_move_start_of_line(buffer buf) {
    return {
        std::move(buf.lines),
        {0, buf.pos.y}
    };
}

buffer buffer_move_end_of_line(buffer buf) {
    auto x = buf.lines[buf.pos.y]->size();
    return {
        std::move(buf.lines),
        {x, buf.pos.y}
    };
}

buffer buffer_move_left(buffer buf, size_t n) {
    auto x = std::min(buf.lines[buf.pos.y]->size(), buf.pos.x);
    return {
        std::move(buf.lines),
        {n > x ? 0 : x - n, buf.pos.y}
    };
}

buffer buffer_move_right(buffer buf, size_t n) {
    return {
        std::move(buf.lines),
        {buf.pos.x + n, buf.pos.y}
    };
}

buffer buffer_move_down(buffer buf, size_t n) {
    auto y = std::min(buf.pos.y + n, buf.lines.size() - 1);
    return {
        std::move(buf.lines),
        {buf.pos.x, y}
    };
}

buffer buffer_move_up(buffer buf, size_t n) {
    return {
        std::move(buf.lines),
        {buf.pos.x, n > buf.pos.y ? 0 : buf.pos.y - n}
    };
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
    if (is_regex_space(ch)) {
        do {
            if (buffer_is_at_end_of_line(buf) && buffer_is_at_last_line(buf)) {
                return buf;
            } else if (buffer_is_at_end_of_line(buf)) {
                y++;
                x = 0;
            } else {
                x++;
            }
        } while (is_regex_space(buffer_get_char(buf, x, y)));
    } else {
        do {
            if (buffer_is_at_end_of_line(buf) && buffer_is_at_last_line(buf)) {
                return buf;
            } else if (buffer_is_at_end_of_line(buf)) {
                y++;
                x = 0;
            } else {
                x++;
            }
        } while (!is_regex_space(buffer_get_char(buf, x, y)) &&
                 is_regex_word(buffer_get_char(buf, x, y))
                 == is_regex_word(ch));
        if (is_regex_space(buffer_get_char(buf, x, y))) {
            return buffer_move_next_word(buf);
        }
    }
    return buf;
}

buffer buffer_erase_current_line(buffer buf) {
    if (buf.lines.size() == 1) {
        return {{std::make_shared<buffer_line>()}, buf.pos};
    }
    buf.lines.erase(buf.lines.begin() + buf.pos.y);
    auto y = std::min(buf.pos.y, buf.lines.size() - 1);
    return {
        std::move(buf.lines),
        {buf.pos.x, y}
    };
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

buffer buffer_break_line(buffer buf) {
    auto& [lines, pos] = buf;
    auto& [x, y] = pos;
    x = std::min(lines[y]->size(), x);
    /* clone line if someone else holds a reference to it */
    if (lines[y].use_count() > 1) {
        lines[y] = std::make_shared<buffer_line>(*lines[y]);
    }
    lines.insert(lines.begin() + y + 1, std::make_shared<buffer_line>());
    auto& upper_line = *lines[y];
    auto& lower_line = *lines[y+1];
    lower_line.insert(lower_line.size(), upper_line.substr(x));
    upper_line.erase(x);
    x = 0;
    y++;
    /* use same indentation as previous line */
    lower_line.insert(0, indentation_level(upper_line), ' ');
    x += indentation_level(upper_line);
    /* increase indentation if the previous line has an open bracket */
    /* TODO: improve */
    if (bracket_balance(upper_line) > 0) {
        if (lower_line.substr(x) == U"}" || lower_line.substr(x) == U")") {
            lines.insert(lines.begin() + y + 1, std::make_shared<buffer_line>(lower_line));
            lower_line.erase(x);
        }
        lower_line.insert(0, 4, ' ');
        x += 4;
    }
    return buf;
}

buffer buffer_erase(buffer buf) {
    auto& [lines, pos] = buf;
    auto& [x, y] = pos;
    x = std::min(x, lines[y]->size());
    if (x > 0) {
        x--;
        /* clone line if someone else holds a reference to it */
        if (lines[y].use_count() > 1) {
            lines[y] = std::make_shared<buffer_line>(*lines[y]);
        }
        auto& line = *lines[y];
        /* remove corresponding bracket, if any */
        if (line.size() > x + 1 && opposite_bracket(line[x]) == line[x + 1]) {
            line.erase(x + 1, 1);
        }
        line.erase(x, 1);
    } else if (y > 0) {
        y--;
        x = lines[y]->size();
        /* clone line if someone else holds a reference to it */
        if (lines[y].use_count() > 1) {
            lines[y] = std::make_shared<buffer_line>(*lines[y]);
        }
        lines[y]->insert(lines[y]->end(), lines[y+1]->begin(),
                                          lines[y+1]->end());
        lines.erase(lines.begin() + y + 1);
    }
    return buf;
}

buffer buffer_indent(buffer buf) {
    for (int i = 0; i < 4; i++)
        buf = buffer_insert(std::move(buf), ' ');
    return buf;
}
