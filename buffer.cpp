#include <algorithm>
#include "buffer.h"

buffer buffer_move_start(buffer b) {
    auto& [lines, pos] = b;
    pos.x = 0;
    pos.y = 0;
    return b;
}

buffer buffer_move_end(buffer b) {
    auto& [lines, pos] = b;
    pos.y = lines.size() - 1;
    return b;
}

buffer buffer_move_start_of_line(buffer b) {
    auto& [lines, pos] = b;
    pos.x = 0;
    return b;
}

buffer buffer_move_end_of_line(buffer b) {
    auto& [lines, pos] = b;
    pos.x = lines[pos.y]->size();
    return b;
}

buffer buffer_move_left(buffer b, size_t n) {
    auto& [lines, pos] = b;
    auto x = std::min(b.first[b.second.y]->size(), b.second.x);
    pos.x = n > x ? 0 : x - n;
    return b;
}

buffer buffer_move_right(buffer b, size_t n) {
    auto& [lines, pos] = b;
    pos.x += n;
    return b;
}

buffer buffer_move_down(buffer b, size_t n) {
    auto& [lines, pos] = b;
    pos.y = std::min(pos.y + n, lines.size() - 1);
    return b;
}

buffer buffer_move_up(buffer b, size_t n) {
    auto& [lines, pos] = b;
    pos.y = n > pos.y ? 0 : pos.y - n;
    return b;
}

buffer buffer_erase_current_line(buffer b) {
    auto& [lines, pos] = b;
    if (lines.size() == 1) {
        return {{std::make_shared<buffer_line>()}, pos};
    }
    lines.erase(lines.begin() + pos.y);
    pos.y = std::min(pos.y, lines.size() - 1);
    return b;
}

buffer buffer_insert(buffer b, buffer_char c, size_t n) {
    auto& [lines, pos] = b;
    size_t x = std::min(lines[pos.y]->size(), pos.x);
    lines[pos.y] = std::make_shared<buffer_line>(*lines[pos.y]);
    for (size_t i = 0; i < n; i++) {
        lines[pos.y]->insert(lines[pos.y]->begin() + x, c);
    }
    pos.x = x + n;
    return b;
}

buffer buffer_break_line(buffer b) {
    auto& [lines, pos] = b;
    if (pos.x == 0) {
        lines.insert(lines.begin() + pos.y, std::make_shared<buffer_line>());
        pos.y++;
    } else {
        auto line = *lines[pos.y];
        pos.x = std::min(lines[pos.y]->size(), pos.x);
        lines.erase(lines.begin() + pos.y);
        lines.insert(lines.begin() + pos.y,
                        std::make_shared<buffer_line>(
                                    line.begin() + pos.x,
                                    line.end()));
        lines.insert(lines.begin() + pos.y,
                        std::make_shared<buffer_line>(
                                    line.begin(),
                                    line.begin() + pos.x));
        pos.y++;
        pos.x = 0;
    }
    return b;
}

buffer buffer_erase(buffer b) {
    auto& [lines, pos] = b;
    pos.x = std::min(pos.x, lines[pos.y]->size());
    if (pos.x > 0) {
        lines[pos.y] = std::make_shared<buffer_line>(*lines[pos.y]);
        lines[pos.y]->erase(lines[pos.y]->begin() + pos.x - 1);
        pos.x = pos.x - 1;
    } else if (pos.y > 0) {
        pos.x = lines[pos.y-1]->size();
        lines[pos.y-1] = std::make_shared<buffer_line>(*lines[pos.y-1]);
        lines[pos.y-1]->insert(lines[pos.y-1]->end(),
                           lines[pos.y]->begin(),
                           lines[pos.y]->end());
        lines.erase(lines.begin() + pos.y);
        pos.y = pos.y - 1;
    }
    return b;
}

buffer_line buffer_get_line(const buffer& b, size_t y) {
    const auto& [lines, pos] = b;
    return *lines[y];
}

buffer_line buffer_get_current_line(const buffer& b) {
    auto& [lines, pos] = b;
    return *lines[pos.y];
}

