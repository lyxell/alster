#include <algorithm>
#include "buffer.h"

static buffer_char bracket_left_to_right(buffer_char c) {
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
    return {
        std::move(buf.lines),
        {buf.pos.x, buf.lines.size() - 1}
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
    return {
        std::move(buf.lines),
        {buf.pos.x, std::min(buf.pos.y + n, buf.lines.size() - 1)}
    };
}

buffer buffer_move_up(buffer buf, size_t n) {
    return {
        std::move(buf.lines),
        {buf.pos.x, n > buf.pos.y ? 0 : buf.pos.y - n}
    };
}

buffer buffer_erase_current_line(buffer buf) {
    if (buf.lines.size() == 1) {
        return {{std::make_shared<buffer_line>()}, buf.pos};
    }
    buf.lines.erase(buf.lines.begin() + buf.pos.y);
    return {
        std::move(buf.lines),
        {buf.pos.x, std::min(buf.pos.y, buf.lines.size() - 1)}
    };
}

buffer buffer_insert(buffer buf, buffer_char c, size_t n) {
    auto& [lines, pos] = buf;
    auto& [x, y] = pos;
    x = std::min(lines[y]->size(), x);
    if (lines[y].use_count() > 1) {
        lines[y] = std::make_shared<buffer_line>(*lines[y]);
    }
    for (size_t i = 0; i < n; i++) {
        lines[y]->insert(lines[y]->begin() + x, c);
    }
    x = x + n;
    if (bracket_left_to_right(c)) {
        buf = buffer_insert(std::move(buf), bracket_left_to_right(c), n);
        x = x - n;
    }
    return buf;
}

buffer buffer_break_line(buffer buf) {
    auto& [lines, pos] = buf;
    auto& [x, y] = pos;
    auto line = *lines[y];
    x = std::min(line.size(), x);
    lines[y] = std::make_shared<buffer_line>(line.substr(x));
    lines.insert(lines.begin() + y,
                std::make_shared<buffer_line>(line.substr(0, x)));
    x = 0;
    y = y + 1;
    return buf;
}

buffer buffer_erase(buffer buf) {
    auto& [lines, pos] = buf;
    auto& [x, y] = pos;
    x = std::min(x, lines[y]->size());
    if (x > 0) {
        x = x - 1;
        if (lines[y].use_count() > 1) {
            lines[y] = std::make_shared<buffer_line>(*lines[y]);
        }
        lines[y]->erase(lines[y]->begin() + x);
    } else if (y > 0) {
        y = y - 1;
        x = lines[y]->size();
        if (lines[y].use_count() > 1) {
            lines[y] = std::make_shared<buffer_line>(*lines[y]);
        }
        lines[y]->insert(lines[y]->begin(), lines[y+1]->begin(),
                                            lines[y+1]->end());
        lines.erase(lines.begin() + y + 1);
    }
    return buf;
}

