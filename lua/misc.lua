local api = {}

api.mergestates = function(oldstate, newstate)
    for k, v in pairs(newstate) do
        oldstate[k] = v
    end
end

return api
