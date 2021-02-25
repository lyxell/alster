buffer = {
    x = 0,
    y = 0,
    move_down  = function() buffer.y = buffer.y + 1 end,
    move_up    = function() buffer.y = buffer.y - 1 end,
    move_left  = function() buffer.x = buffer.x - 1 end,
    move_right = function() buffer.x = buffer.x + 1 end
}

config = {
    bindings = {},
    bind = function(str, fn)
        config.bindings[str] = fn
    end,
}

config.bind("h", function() buffer.move_left() end)
config.bind("j", function() buffer.move_down() end)
config.bind("k", function() buffer.move_up() end)
config.bind("l", function() buffer.move_right() end)
