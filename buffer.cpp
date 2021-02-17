#include <algorithm>
#include "buffer.h"

buffer
buffer_move_start(const buffer& b) {
    auto& [lines, pos] = b;
    return {lines, {0, 0}};
}

buffer
buffer_move_end(const buffer& b) {
    auto& [lines, pos] = b;
    return {lines, {0, lines.size() - 1}};
}

buffer
buffer_move_start_of_line(const buffer& b) {
    auto& [lines, pos] = b;
    return {lines, {0, pos.y}};
}

buffer
buffer_move_end_of_line(const buffer& b) {
    auto& [lines, pos] = b;
    return {lines, {lines[pos.y]->size(), pos.y}};
}

buffer
buffer_move_left(buffer b, size_t n) {
    auto& [lines, pos] = b;
    size_t x = std::min(b.first[b.second.y]->size(), b.second.x);
    pos.x = n > x ? 0 : x - n;
    return b;
}

buffer
buffer_move_right(buffer b, size_t n) {
    auto& [lines, pos] = b;
    size_t x = std::min(lines[pos.y]->size(), pos.x);
    pos.x = std::min(x + n, lines[pos.y]->size());
    return b;
}

buffer
buffer_move_down(buffer b, size_t n) {
    auto& [lines, pos] = b;
    pos.y = std::min(pos.y + n, lines.size() - 1);
    return b;
}

buffer
buffer_move_up(buffer b, size_t n) {
    auto& [lines, pos] = b;
    pos.y = n > pos.y ? 0 : pos.y - n;
    return b;
}

buffer
buffer_erase_current_line(const buffer& b) {
    auto [lines, pos] = b;
    if (lines.size() == 1) {
        return {{std::make_shared<buffer_line>()}, pos};
    }
    lines.erase(lines.begin() + pos.y);
    return {lines, {pos.x, std::min(pos.y, lines.size() - 1)}};
}

buffer
buffer_insert(buffer b, buffer_char c, size_t n) {
    auto& [lines, pos] = b;
    size_t x = std::min(lines[pos.y]->size(), pos.x);
    lines[pos.y] = std::make_shared<buffer_line>(*lines[pos.y]);
    for (size_t i = 0; i < n; i++) {
        lines[pos.y]->insert(lines[pos.y]->begin() + x, c);
    }
    pos.x = x + n;
    return b;
}

buffer
buffer_break_line(buffer b) {
    auto& [lines, pos] = b;
    if (pos.x == 0) {
        lines.insert(lines.begin() + pos.y, std::make_shared<buffer_line>());
        pos.y++;
    } else {
        auto line = *lines[pos.y];
        size_t x = std::min(lines[pos.y]->size(), pos.x);
        lines.erase(lines.begin() + pos.y);
        lines.insert(lines.begin() + pos.y,
                        std::make_shared<buffer_line>(
                                    line.begin() + x,
                                    line.end()));
        lines.insert(lines.begin() + pos.y,
                        std::make_shared<buffer_line>(
                                    line.begin(),
                                    line.begin() + x));
        pos.y++;
    }
    return b;
}

buffer
buffer_erase(buffer b) {
    auto& [lines, pos] = b;
    auto [x, y] = std::pair(std::min(pos.x, lines[pos.y]->size()), pos.y);
    if (x > 0) {
        lines[y] = std::make_shared<buffer_line>(*lines[y]);
        lines[y]->erase(lines[y]->begin() + x - 1);
        x = x - 1;
    } else if (y > 0) {
        x = lines[y-1]->size();
        lines[y-1] = std::make_shared<buffer_line>(*lines[y-1]);
        lines[y-1]->insert(lines[y-1]->end(),
                           lines[y]->begin(),
                           lines[y]->end());
        lines.erase(lines.begin() + y);
        y = y - 1;
    }
    return b;
}

std::string
buffer_to_string(const buffer& b) {
    auto& [lines, pos] = b;
    std::string s;
    for (const auto& line : lines) {
        std::copy(line->begin(), line->end(), std::back_inserter(s));
        s.push_back('\n');
    }
    if (s.size() > 0) s.pop_back();
    return s;
}

buffer_line
buffer_get_line(const buffer& b, size_t y) {
    const auto& [lines, pos] = b;
    return *lines[y];
}

buffer_line
buffer_get_current_line(const buffer& b) {
    auto& [lines, pos] = b;
    return *lines[pos.y];
}
