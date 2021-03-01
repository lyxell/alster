local MODE_NORMAL = 0
local MODE_INSERT = 1

local KEY_ESC = "\27"
local KEY_DEL = "\127"
local KEY_ENTER = "\r"
local KEY_TAB = "\t"

-- indentation
local INDENTATION = line.char(string.byte("    ", 1, 4))
getindentation = function(l)
    local i = 1
    local res = 0
    while l:sub(i, i + 3) == INDENTATION do
        res = res + 1
        i = i + #INDENTATION
    end
    return res
end
autoindent = function(l, prevline)
    local previndent = 0
    if prevline ~= nil then
        previndent = getindentation(prevline)
    end
    for i = 1, previndent, 1 do
        l = INDENTATION .. l
    end
    return l
end

-- bindings

bindings = {
    insert = {
        [KEY_ESC] = function()
            buffer.mode = MODE_NORMAL
        end,
        [KEY_ENTER] = function()
            local y = buffer.y
            local x = math.min(buffer.x, #buffer.lines[y] + 1)
            local above = buffer.lines[y]:sub(1, x-1)
            local below = buffer.lines[y]:sub(x)
            below = autoindent(below, above)
            buffer.lines[y] = above
            lines.insert(buffer.lines, y + 1, below)
            buffer.y = y + 1
            buffer.x = #INDENTATION * getindentation(buffer.lines[y+1]) + 1
        end,
        [KEY_DEL] = function()
            local y = buffer.y
            local x = math.min(buffer.x, #buffer.lines[y] + 1)
            local line = buffer.lines[y]
            if x > 1 then
                buffer.lines[y] = line:sub(1, x - 2) .. line:sub(x)
                buffer.x = x - 1
            elseif y > 1 then
                buffer.x = #buffer.lines[y - 1] + 1
                buffer.y = y - 1
                buffer.lines[y - 1] = buffer.lines[y - 1] .. buffer.lines[y]
                lines.remove(buffer.lines, y)
            end
        end,
        [KEY_TAB] = function()
            buffer.lines[buffer.y] = buffer.lines[buffer.y]:sub(1, buffer.x - 1)
                                  .. INDENTATION
                                  .. buffer.lines[buffer.y]:sub(buffer.x)
            buffer.x = buffer.x + 4
        end
    },
    normal = {
        ["h"] = function(n)
            buffer.x = math.max(buffer.x - 1, 1)
        end,
        ["j"] = function(n)
            buffer.y = math.min(buffer.y + 1, #buffer.lines)
        end,
        ["k"] = function(n)
            buffer.y = math.max(buffer.y - 1, 1)
        end,
        ["l"] = function(n)
            buffer.x = math.min(buffer.x + 1, #(buffer.lines[buffer.y]) + 1)
        end,
        ["d$"] = function()
            buffer.lines[buffer.y] = buffer.lines[buffer.y]:sub(1, buffer.x - 1)
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
            lines.insert(buffer.lines, buffer.y + 1, line.char())
            buffer.x = 1
            buffer.y = buffer.y + 1
            buffer.mode = MODE_INSERT
        end,
        ["O"] = function()
            lines.insert(buffer.lines, buffer.y, line.char())
            buffer.x = 1
            buffer.mode = MODE_INSERT
        end,
        ["q"] = function()
            buffer.exiting = true
        end,
        ["dd"] = function()
            lines.remove(buffer.lines, buffer.y)
        end,
        ["=="] = function(n)
            buffer.lines[buffer.y] = autoindent(buffer.lines[buffer.y],
                                                buffer.lines[buffer.y - 1])
        end,
    }
}

