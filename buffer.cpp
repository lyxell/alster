#include <algorithm>
#include "buffer.h"

buffer_t
buffer_move_left(const buffer_t& b, int steps) {
    buffer_t n(b);
    n.x = std::max(0, n.x - steps);
    return n;
}

buffer_t
buffer_move_right(const buffer_t& b, int steps) {
    buffer_t n(b);
    n.x = std::min(int(b.lines[n.y]->size()), n.x + steps);
    return n;
}

buffer_t
buffer_move_down(const buffer_t& b, int steps) {
    buffer_t n(b);
    n.y = std::min(int(b.lines.size()) - 1, n.y + steps);
    n.x = std::min(n.x, int(b.lines[n.y]->size()));
    return n;
}

buffer_t
buffer_move_up(const buffer_t& b, int steps) {
    buffer_t n(b);
    n.y = std::max(0, n.y - steps);
    n.x = std::min(n.x, int(b.lines[n.y]->size()));
    return n;
}

buffer_t
buffer_insert(const buffer_t& b, buffer_char_t c) {
    buffer_t n(b);
    n.lines[n.y] = std::make_shared<buffer_line_t>(*n.lines[n.y]);
    n.lines[n.y]->insert(n.lines[n.y]->begin() + n.x, c);
    n.x++;
    return n;
}

buffer_t
buffer_break_line(const buffer_t& b) {
    buffer_t n(b);
    buffer_line_t curr_line = *n.lines[n.y];
    n.lines.erase(n.lines.begin() + n.y);
    n.lines.insert(n.lines.begin() + n.y,
                    std::make_shared<buffer_line_t>(
                                curr_line.begin() + n.x,
                                curr_line.end()));
    n.lines.insert(n.lines.begin() + n.y,
                    std::make_shared<buffer_line_t>(
                                curr_line.begin(),
                                curr_line.begin() + n.x));
    n.x = 0;
    n.y++;
    return n;
}

buffer_t
buffer_erase(const buffer_t& b) {
    buffer_t n(b);
    if (n.x > 0) {
        n.lines[n.y] = std::make_shared<buffer_line_t>(*n.lines[n.y]);
        n.lines[n.y]->erase(n.lines[n.y]->begin() + n.x - 1);
        n.x--;
    } else if (n.y > 0) {
        n.x = n.lines[n.y-1]->size();
        n.lines[n.y-1]->insert(n.lines[n.y-1]->end(),
                               n.lines[n.y]->begin(),
                               n.lines[n.y]->end());
        n.lines.erase(n.lines.begin() + n.y);
        n.y--;
    }
    return n;
}

std::string
buffer_to_string(const buffer_t& b) {
    static std::string s;
    for (const auto& line : b.lines) {
        std::copy(line->begin(), line->end(), std::back_inserter(s));
        s.push_back('\n');
    }
    return s;
}

std::string
buffer_get_line(const buffer_t& b, size_t i) {
    return std::string(b.lines[i]->begin(), b.lines[i]->end());
}
