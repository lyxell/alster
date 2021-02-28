MODE_NORMAL = 0
MODE_INSERT = 1

KEY_ESC = "\27"

lines = {}

buffer = {
    x = 1,
    y = 1,
    lines = {},
    mode = MODE_NORMAL
}

config = {
    bindings = {},
    bind = function(str, fn)
        config.bindings[str] = fn
    end
}

bindings = {
    insert = {
        [KEY_ESC] = function()
            buffer.mode = MODE_NORMAL
        end,
        ["\r"] = function()
            local l = buffer.lines[buffer.y]
            local left, right = line.sub(l, 1, buffer.x - 1),
                                line.sub(l, buffer.x)
            lines.insert(buffer.lines, buffer.y + 1, right)
            buffer.lines[buffer.y] = left
            buffer.y = buffer.y + 1
            buffer.x = 1
        end
    },
    normal = {
        ["h"] = function()
            buffer.x = math.max(buffer.x - 1, 1)
        end,
        ["j"] = function()
            buffer.y = math.min(buffer.y + 1, #buffer.lines)
        end,
        ["k"] = function()
            buffer.y = math.max(buffer.y - 1, 1)
        end,
        ["l"] = function()
            buffer.x = math.min(buffer.x + 1, #(buffer.lines[buffer.y]) + 1)
        end,
        ["d$"] = function()
            buffer.lines[buffer.y] =
                line.sub(buffer.lines[buffer.y], 1, buffer.x - 1)
        end,
        ["gg"] = function()
            buffer.x = 1
            buffer.y = 1
        end,
        ["0"] = function()
            buffer.x = 1
        end,
        ["G"] = function()
            buffer.y = #buffer.lines
        end,
        ["$"] = function()
            buffer.x = #(buffer.lines[buffer.y]) + 1
        end,
        ["A"] = function()
            buffer.mode = MODE_INSERT
            buffer.x = #(buffer.lines[buffer.y]) + 1
        end,
        ["i"] = function()
            buffer.mode = MODE_INSERT
        end,
        ["I"] = function()
            buffer.mode = MODE_INSERT
            buffer.x = 1
        end,
        ["o"] = function()
            lines.insert(buffer.lines, buffer.y + 1, line.create())
            buffer.x = 1
            buffer.y = buffer.y + 1
            buffer.mode = MODE_INSERT
        end,
        ["O"] = function()
            lines.insert(buffer.lines, buffer.y, line.create())
            buffer.x = 1
            buffer.mode = MODE_INSERT
        end,
        ["q"] = function()
            buffer.exiting = true
        end,
        ["dd"] = function()
            lines.remove(buffer.lines, buffer.y)
        end
    }
}

