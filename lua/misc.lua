local pt = require 'piecetable'

local api = {}

api.mergestates = function(oldstate, newstate)
    for k, v in pairs(newstate) do
        oldstate[k] = v
    end
end

api.initialstate = {
    buffer = pt.topiecetable({}),
    x = 1,
    y = 1,
    exiting = false,
    mode = 0,
    cmd = ""
}

return api
