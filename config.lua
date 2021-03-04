MODE_NORMAL = 0
MODE_INSERT = 1

KEY_BACKSPACE = "\127"
KEY_ESCAPE = "\27"

history = {
    undodata = {},
    redodata = {},
    redo = function(history, ...)
        assert(#history.redodata > 0)
        local value = history.redodata[#history.redodata]
        table.insert(history.undodata, arg)
        table.remove(history.redodata)
        return unpack(value)
    end,
    undo = function(history, ...)
        assert(#history.undodata > 0)
        local value = history.undodata[#history.undodata]
        table.insert(history.redodata, arg)
        table.remove(history.undodata)
        return unpack(value)
    end,
    save = function(history, ...)
        table.insert(history.undodata, arg)
    end,
    canundo = function(history)
        return #history.undodata > 0
    end,
    canredo = function(history)
        return #history.redodata > 0
    end
}

events = {
    insert = function(state, insertion)
        local b, x, y = state.buffer, state.x, state.y
        local line = b:get(y)
        return {
            buffer = b:sub(1, y - 1)
                  .. {line:sub(1, x - 1) .. insertion .. line:sub(x)}
                  .. b:sub(y + 1),
            x = x + #insertion
        }
    end
}

bindings = {
    normal = {
        ["q"] = function(state)
            return {exiting = true}
        end,
        ["h"] = function(state)
            return {x = math.max(state.x - 1, 1)}
        end,
        ["j"] = function(state)
            return {y = math.min(state.y + 1, state.buffer:len())}
        end,
        ["k"] = function(state)
            return {y = math.max(state.y - 1, 1)}
        end,
        ["l"] = function(state)
            return {x = math.min(#state.buffer:get(state.y) + 1, state.x + 1)}
        end,
        ["A"] = function(state)
            local b, x, y = state.buffer, state.x, state.y
            history:save(b, x, y)
            return {
                x = #state.buffer:get(state.y) + 1,
                mode = MODE_INSERT
            }
        end,
        ["gg"] = function(state)
            return {x = 1, y = 1}
        end,
        ["G"] = function(state)
            return {x = 1, y = state.buffer:len()}
        end,
        ["0"] = function(state)
            return {x = 1}
        end,
        ["$"] = function(state)
            return {x = #state.buffer:get(state.y) + 1}
        end,
        ["o"] = function(state)
            local b, x, y = state.buffer, state.x, state.y
            history:save(b, x, y)
            return {
                buffer = b:sub(1, y) .. {""} .. b:sub(y + 1),
                x = 1,
                y = y + 1,
                mode = MODE_INSERT
            }
        end,
        ["dd"] = function(state)
            local b, y = state.buffer, state.y
            history:save(b, x, y)
            return {
                buffer = b:sub(1, y - 1) .. b:sub(y + 1),
                y = math.min(y, b:len() - 1)
            }
        end,
        ["i"] = function(state)
            history:save(state.buffer, state.x, state.y)
            return {
                mode = MODE_INSERT
            }
        end,
        ["u"] = function(state)
            if not history:canundo() then
                return {
                    status = "History empty"
                }
            end
            b, x, y = history:undo(state.buffer, state.x, state.y)
            return {
                buffer = b,
                x = x,
                y = y
            }
        end,
        ["r"] = function(state)
            if not history:canredo() then
                return {
                    status = "Future empty"
                }
            end
            b, x, y = history:redo(state.buffer, state.x, state.y)
            return {
                buffer = b,
                x = x,
                y = y
            }
        end
    },
    insert = {
        ["\r"] = function(state)
            local b, x, y = state.buffer, state.x, state.y
            local l = b:get(y)
            return {
                buffer = b:sub(1, y - 1) ..
                         {l:sub(1, x - 1), l:sub(x)} ..
                         b:sub(y + 1),
                x = 1,
                y = y + 1
            }
        end,
        [KEY_ESCAPE] = function(state)
            return {
                mode = MODE_NORMAL
            }
        end,
        [KEY_BACKSPACE] = function(state)
            local b, x, y = state.buffer, state.x, state.y
            if x > 1 then
                return {
                    buffer = b:sub(1, y - 1)
                          .. {b:get(y):sub(1, x - 2) .. b:get(y):sub(x)}
                          .. b:sub(y + 1),
                    x = x - 1
                }
            elseif y > 1 then
                return {
                    y = y - 1,
                    x = #b:get(y - 1) + 1,
                    buffer = b:sub(1, y - 2)
                          .. {b:sub(y - 1, y):join()}
                          .. b:sub(y + 1)
                }
            end
            return {}
        end
    }
}