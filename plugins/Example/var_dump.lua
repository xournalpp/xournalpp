-- Source: https://gist.github.com/lunixbochs/5b0bb27861a396ab7a86#file-var_dump-lua-L1

local function string(o)
    return '"' .. tostring(o) .. '"'
end

local function recurse(o, indent)
    if indent == nil then indent = '' end
    local indent2 = indent .. '  '
    if type(o) == 'table' then
        local s = indent .. '{' .. '\n'
        local first = true
        for k,v in pairs(o) do
            if first == false then s = s .. ', \n' end
            local l = type(k) == 'number' and k or string(k)
            s = s .. indent2 .. '[' .. l .. '] = ' .. recurse(v, indent2)
            first = false
        end
        return s .. '\n' .. indent .. '}'
    else
        return string(o)
    end
end

local function var_dump(...)
    local args = {...}
    if #args > 1 then
        var_dump(args)
    else
        print(recurse(args[1]))
    end
end
return var_dump
