MODE_NORMAL = 0
MODE_INSERT = 1

buffer = {
    x = 0,
    y = 0,
    mode = MODE_NORMAL,
    num_lines = 1,
    move_down = function()
        buffer.y = math.min(buffer.y + 1, buffer.num_lines - 1)
    end,
    move_up = function()
        buffer.y = math.max(buffer.y - 1, 0)
    end,
    move_left = function()
        buffer.x = math.max(buffer.x - 1, 0)
    end,
    move_right = function()
        buffer.x = buffer.x + 1
    end,
    move_start = function()
        buffer.x = 0
        buffer.y = 0
    end,
    move_end = function()
        buffer.y = buffer.num_lines - 1
    end,
    move_start_of_line = function()
        buffer.x = 0
    end
}

config = {
    bindings = {},
    bind = function(str, fn)
        config.bindings[str] = fn
    end,
}

config.bind("h",  function() buffer.move_left() end)
config.bind("j",  function() buffer.move_down() end)
config.bind("k",  function() buffer.move_up() end)
config.bind("l",  function() buffer.move_right() end)
config.bind("gg", function() buffer.move_start() end)
config.bind("0",  function() buffer.move_start_of_line() end)
config.bind("G",  function() buffer.move_end() end)
config.bind("i",  function() buffer.mode = MODE_INSERT end)
config.bind("I",  function()
    buffer.mode = MODE_INSERT
    buffer.move_start_of_line()
end)

