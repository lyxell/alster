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
buffer_move_left(const buffer& b, size_t n) {
    auto& [lines, pos] = b;
    size_t x = std::min(lines[pos.y]->size(), pos.x);
    return {lines, {n > x ? 0 : x - n, pos.y}};
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
buffer_move_right(const buffer& b, size_t n) {
    auto& [lines, pos] = b;
    size_t x = std::min(lines[pos.y]->size(), pos.x);
    return {lines, {std::min(x + n, lines[pos.y]->size()), pos.y}};
}

buffer
buffer_move_down(const buffer& b, size_t n) {
    auto& [lines, pos] = b;
    return {lines, {pos.x, std::min(pos.y + n, lines.size() - 1)}};
}

buffer
buffer_move_up(const buffer& b, size_t n) {
    auto& [lines, pos] = b;
    return {lines, {pos.x, n > pos.y ? 0 : pos.y - n}};
}

buffer
buffer_insert(const buffer& b, buffer_char c) {
    auto [lines, pos] = b;
    size_t x = std::min(lines[pos.y]->size(), pos.x);
    lines[pos.y] = std::make_shared<buffer_line>(*lines[pos.y]);
    lines[pos.y]->insert(lines[pos.y]->begin() + x, c);
    return {lines, {x + 1, pos.y}};
}

buffer
buffer_break_line(const buffer& b) {
    auto [lines, pos] = b;
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
    return {lines, {0, pos.y + 1}};
}

buffer
buffer_erase(const buffer& b) {
    auto [lines, pos] = b;
    auto [x, y] = std::pair(std::min(pos.x, lines[pos.y]->size()), pos.y);
    if (x > 0) {
        lines[y] = std::make_shared<buffer_line>(*lines[y]);
        lines[y]->erase(lines[y]->begin() + x - 1);
        x--;
    } else if (y > 0) {
        x = lines[y-1]->size();
        lines[y-1]->insert(lines[y-1]->end(),
                               lines[y]->begin(),
                               lines[y]->end());
        lines.erase(lines.begin() + y);
        y--;
    }
    return {lines, {x, y}};
}

std::string
buffer_to_string(const buffer& b) {
    auto& [lines, pos] = b;
    std::string s;
    for (const auto& line : lines) {
        std::copy(line->begin(), line->end(), std::back_inserter(s));
        s.push_back('\n');
    }
    // no trailing newline
    if (s.size() > 0) s.pop_back();
    return s;
}

buffer_line
buffer_get_line(const buffer& b, size_t i) {
    auto& [lines, pos] = b;
    return *lines[i];
}

buffer_line
buffer_get_current_line(const buffer& b) {
    auto& [lines, pos] = b;
    return *lines[pos.y];
}
