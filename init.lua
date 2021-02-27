MODE_NORMAL = 0
MODE_INSERT = 1

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

config.bind("h",  function()
    buffer.x = math.max(buffer.x - 1, 1)
end)
config.bind("j",  function()
    buffer.y = math.min(buffer.y + 1, #buffer.lines - 1)
end)
config.bind("d$",  function()
    buffer.lines[buffer.y] = line.sub(buffer.lines[buffer.y], 1, buffer.x - 1)
end)
config.bind("k",  function()
    buffer.y = math.max(buffer.y - 1, 1)
end)
config.bind("l", function()
    buffer.x = math.min(buffer.x + 1, #(buffer.lines[buffer.y]) + 1)
end)
config.bind("gg", function()
    buffer.x = 1
    buffer.y = 1
end)
config.bind("0",  function()
    buffer.x = 1
end)
config.bind("G",  function()
    buffer.y = #buffer.lines - 1
end)
config.bind("$",  function()
    buffer.x = #(buffer.lines[buffer.y]) + 1
end)
config.bind("A",  function()
    buffer.mode = MODE_INSERT
    buffer.x = #(buffer.lines[buffer.y]) + 1
end)
config.bind("i",  function()
    buffer.mode = MODE_INSERT
end)
config.bind("I",  function()
    buffer.mode = MODE_INSERT
    buffer.x = 1
end)
config.bind("\r",  function()
    local curr_line = buffer.lines[buffer.y]
    lines.insert(buffer.lines, buffer.y + 1, line.sub(curr_line, buffer.x, #curr_line))
    buffer.lines[buffer.y] = line.sub(curr_line, 1, buffer.x - 1)
    buffer.y = buffer.y + 1
    buffer.x = 1
end)
config.bind("o",  function()
    lines.insert(buffer.lines, buffer.y + 1, line.create())
    buffer.x = 1
    buffer.y = buffer.y + 1
    buffer.mode = MODE_INSERT
end)
config.bind("O",  function()
    lines.insert(buffer.lines, buffer.y, line.create())
    buffer.x = 1
    buffer.mode = MODE_INSERT
end)
