local api = {}

local table_assign = function(left, right)
    for k, v in pairs(right) do
        left[k] = v
    end
end

local functions = {
    get = function(pt, i)
        return (api.flattenpiecetable(pt))[i]
    end,
    sub = function(pt, i, j)
        local res = api.topiecetable({})
        local skip = i - 1
        local take = math.huge
        if not (j == nil) then
            take = j - i + 1
        end
        for _, v in ipairs(pt.ptrs) do
            if take < 1 then
                return res
            elseif v.length > skip then
                table.insert(res.ptrs, {
                    data = v.data,
                    offset = v.offset + skip,
                    length = math.min(v.length - skip, take)
                })
                take = take - math.min(v.length - skip, take)
                skip = 0
            else
                skip = skip - v.length
            end
        end
        return res
    end,
    len = function(pt)
        return #api.flattenpiecetable(pt)
    end,
    join = function(pt)
        return table.concat(api.flattenpiecetable(pt))
    end
}

local mt = {
    __concat = function(lhs, rhs)
        local result = api.topiecetable({})
        if not lhs.ptrs then
            lhs = api.topiecetable(lhs)
        end
        if not rhs.ptrs then
            rhs = api.topiecetable(rhs)
        end
        for _, v in ipairs(lhs.ptrs) do
            table.insert(result.ptrs, v)
        end
        for _, v in ipairs(rhs.ptrs) do
            table.insert(result.ptrs, v)
        end
        return result
    end,
    __tostring = function(pt)
        local data = {}
        for _, v in ipairs(pt.ptrs) do
            table.insert(data, table.concat(v.data, ", ", v.offset,
                                            v.offset + v.length - 1))
        end
        return "{" .. table.concat(data, ", ") .. "}"
    end,
    __len = function(pt)
        return #api.flattenpiecetable(pt)
    end
}

api.topiecetable = function(t)
    local pt = {ptrs = {}}
    if t ~= nil and #t > 0 then
        table.insert(pt.ptrs, { data = t, offset = 1, length = #t })
    end
    setmetatable(pt, mt)
    table_assign(pt, functions)
    return pt
end

api.flattenpiecetable = function(pt)
    local t = {}
    for _, v in ipairs(pt.ptrs) do
        for j = 1, v.length do
            table.insert(t, v.data[v.offset + j - 1])
        end
    end
    return t
end

return api
