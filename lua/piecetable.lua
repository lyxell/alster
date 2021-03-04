local mt = {
    __concat = function(lhs, rhs)
        result = topiecetable({})
        if not lhs.ptrs then
            lhs = topiecetable(lhs)
        end
        if not rhs.ptrs then
            rhs = topiecetable(rhs)
        end
        for i, v in ipairs(lhs.ptrs) do
            table.insert(result.ptrs, v)
        end
        for i, v in ipairs(rhs.ptrs) do
            table.insert(result.ptrs, v)
        end
        return result
    end,
    __tostring = function(pt)
        data = {}
        for i, v in ipairs(pt.ptrs) do
            table.insert(data, table.concat(v.data, ", ", v.offset,
                                            v.offset + v.length - 1))
        end
        return "{" .. table.concat(data, ", ") .. "}"
    end,
    __len = function(pt)
        return #flattenpiecetable(pt)
    end
}

local api = {
    get = function(pt, i)
        return (flattenpiecetable(pt))[i]
    end,
    sub = function(pt, i, j)
        local res = topiecetable({})
        local skip = i - 1
        local take = math.huge
        if not (j == nil) then
            take = j - i + 1
        end
        for i, v in ipairs(pt.ptrs) do
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
        return #flattenpiecetable(pt)
    end,
    join = function(pt)
        return table.concat(flattenpiecetable(pt))
    end
}

topiecetable = function(t)
    local pt = {ptrs = {}}
    if t ~= nil and #t > 0 then
        table.insert(pt.ptrs, { data = t, offset = 1, length = #t })
    end
    setmetatable(pt, mt)
    pt.sub = api.sub
    pt.flatten = api.flatten
    pt.get = api.get
    pt.len = api.len
    pt.join = api.join
    return pt
end

flattenpiecetable = function(pt)
    t = {}
    for i, v in ipairs(pt.ptrs) do
        for j = 1, v.length do
            table.insert(t, v.data[v.offset + j - 1])
        end
    end
    return t
end
