#include <algorithm>
#include "buffer.h"

buffer buffer_move_start(buffer_lines lines, buffer_position pos) {
    return {
        std::move(lines),
        {0, 0}
    };
}

buffer buffer_move_end(buffer_lines lines, buffer_position pos) {
    return {
        std::move(lines),
        {pos.x, lines.size() - 1}
    };
}

buffer buffer_move_start_of_line(buffer_lines lines, buffer_position pos) {
    return {
        std::move(lines),
        {0, pos.y}
    };
}

buffer buffer_move_end_of_line(buffer_lines lines, buffer_position pos) {
    return {
        std::move(lines),
        {lines[pos.y]->size(), pos.y}
    };
}

buffer buffer_move_left(buffer_lines lines, buffer_position pos, size_t n) {
    auto x = std::min(lines[pos.y]->size(), pos.x);
    return {
        std::move(lines),
        {n > x ? 0 : x - n, pos.y}
    };
}

buffer buffer_move_right(buffer_lines lines, buffer_position pos, size_t n) {
    return {
        std::move(lines),
        {pos.x + n, pos.y}
    };
}

buffer buffer_move_down(buffer_lines lines, buffer_position pos, size_t n) {
    return {
        std::move(lines),
        {pos.x, std::min(pos.y + n, lines.size() - 1)}
    };
}

buffer buffer_move_up(buffer_lines lines, buffer_position pos, size_t n) {
    return {
        std::move(lines),
        {pos.x, n > pos.y ? 0 : pos.y - n}
    };
}

buffer buffer_erase_current_line(buffer_lines lines, buffer_position pos) {
    if (lines.size() == 1) {
        return {{std::make_shared<buffer_line>()}, pos};
    }
    lines.erase(lines.begin() + pos.y);
    return {
        std::move(lines),
        {pos.x, std::min(pos.y, lines.size() - 1)}
    };
}

buffer buffer_insert(buffer_lines lines, buffer_position pos,
                     buffer_char c, size_t n) {
    auto x = std::min(lines[pos.y]->size(), pos.x);
    lines[pos.y] = std::make_shared<buffer_line>(*lines[pos.y]);
    for (size_t i = 0; i < n; i++) {
        lines[pos.y]->insert(lines[pos.y]->begin() + x, c);
    }
    return {
        std::move(lines),
        {pos.x + n, pos.y}
    };
}

buffer buffer_break_line(buffer_lines lines, buffer_position pos) {
    auto line = *lines[pos.y];
    auto x = std::min(lines[pos.y]->size(), pos.x);
    lines.erase(lines.begin() + pos.y);
    lines.insert(lines.begin() + pos.y,
                    std::make_shared<buffer_line>(
                                line.begin() + x,
                                line.end()));
    lines.insert(lines.begin() + pos.y,
                    std::make_shared<buffer_line>(
                                line.begin(),
                                line.begin() + x));
    return {
        std::move(lines),
        {0, pos.y + 1}
    };
}

buffer buffer_erase(buffer_lines lines, buffer_position pos) {
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
    return {
        std::move(lines),
        pos
    };
}

