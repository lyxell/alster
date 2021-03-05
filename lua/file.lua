local api = {}

api.read = function(filename)
    local file = io.open(filename, "rb")
    if not file then
        return {}
    end
    local lines = {}
    for line in io.lines(filename) do
        table.insert(lines, line)
    end
    file:close()
    return lines
end

api.write = function(filename, lines)
    local file = io.open(filename, "wb")
    for _, line in ipairs(lines) do
        file:write(line, "\n")
    end
    file:close()
end

return api
