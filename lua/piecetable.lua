local mt = {
    __concat = function(lhs, rhs)
        result = topiecetable({})
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
    end
}

local api = {
    sub = function(pt, i)
        local res = topiecetable({})
        shouldskip = i - 1
        for i, v in ipairs(pt.ptrs) do
            if v.length > shouldskip then
                table.insert(res.ptrs, {
                    data = v.data,
                    offset = v.offset + shouldskip,
                    length = v.length - shouldskip
                })
                shouldskip = 0
            else
                shouldskip = shouldskip - v.length
            end
        end
        return res
    end
}

topiecetable = function(t)
    local pt = {ptrs = {}}
    if t ~= nil and #t > 0 then
        table.insert(pt.ptrs, { data = t, offset = 1, length = #t })
    end
    setmetatable(pt, mt)
    pt.sub = api.sub
    return pt
end
