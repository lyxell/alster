history = {
    undodata = {},
    redodata = {},
    redo = function(history)
    end,
    undo = function(history)
        assert(#history.undodata > 0)
        local value = history.undodata[#history.undodata]
        table.remove(history.undodata)
        return value
    end,
    save = function(history, value)
        table.insert(history.undodata, value)
    end,
    empty = function(history)
        return #history.undodata == 0
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
            return {y = math.min(state.y + 1, #state.buffer)}
        end,
        ["k"] = function(state)
            return {y = math.max(state.y - 1, 1)}
        end,
        ["l"] = function(state)
            return {x = state.x + 1}
        end,
        ["dd"] = function(state)
            local b, y = state.buffer, state.y
            history:save(b)
            return {
                buffer = b:sub(1, y - 1) .. b:sub(y + 1)
            }
        end,
        ["i"] = function(state)
            return {
                mode = "insert"
            }
        end,
        ["u"] = function(state)
            if history:empty() then
                return {
                    status = "History empty"
                }
            end
            return {
                buffer = history:undo()
            }
        end
    }
}
