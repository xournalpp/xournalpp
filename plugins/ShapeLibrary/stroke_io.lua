local _M = {} -- functions to export

-- Function to read and provide formatted stroke data from a shape-file
function _M.readStrokesFromFile(filepath)
    if filepath == nil then return end
    local sandbox = {} -- No access to global variables in order to safely read the file
    setmetatable(sandbox, {__index = function() error("Forbidden function in file " .. filepath, 2) end})

    local f, err = loadfile(filepath, "t", sandbox)
    if not f or err then print("Error loading the strokes from " .. filepath .. ": " .. (err or "")) return end
    local strokesToAdd = {}

    for _, stroke in ipairs(f()) do
        if type(stroke) == "table" and stroke.x and stroke.y then
            local newStroke = {
                x = stroke.x,
                y = stroke.y,
                pressure = stroke.pressure,
                tool = stroke.tool or "pen",
                color = stroke.color or 0,
                width = stroke.width or 1,
                fill = stroke.fill or 0,
                lineStyle = stroke.lineStyle or "plain"
            }
            table.insert(strokesToAdd, newStroke)
        end
    end

    return strokesToAdd -- formatted strokes data for adding
end

-- Function to store stroke information in a file
function _M.storeStrokeInfoInFile(strokes, filepath)

    -- Open a file for writing in the folder path
    local file = assert(io.open(filepath, "w"))
    -- Start writing the Lua table format
    file:write("local strokesData = {\n")
    -- Iterate over each stroke and collect information
    for i, stroke in ipairs(strokes) do
        file:write(string.format("  [%d] = {\n", i))
        file:write("    x = { ")
        -- Write x coordinates
        for j = 1, #stroke.x do
            file:write(stroke.x[j])
            if j < #stroke.x then
                file:write(", ")
            end
        end
        file:write(" },\n")
        file:write("    y = { ")
        -- Write y coordinates
        for j = 1, #stroke.y do
            file:write(stroke.y[j])
            if j < #stroke.y then
                file:write(", ")
            end
        end
        file:write(" },\n")

        -- Write pressure values if present
        if stroke.pressure then
            file:write("    pressure = { ")
            for j = 1, #stroke.pressure do
                file:write(stroke.pressure[j])
                if j < #stroke.pressure then
                    file:write(", ")
                end
            end
            file:write(" },\n")
        else
            file:write("    pressure = {},\n")
        end

        -- Write stroke options
        file:write(string.format("    tool = \"%s\",\n", stroke.tool or "N/A"))
        file:write(string.format("    color = %s,\n", stroke.color or "N/A"))
        file:write(string.format("    width = %.2f,\n", stroke.width or 0.0))
        file:write(string.format("    fill = %d,\n", stroke.fill or 0))
        file:write(string.format("    lineStyle = \"%s\",\n", stroke.lineStyle or "N/A"))
        file:write("  },\n") -- End of stroke table
    end
    file:write("}\n")
    file:write("return strokesData")
    file:write("   -- Return the strokesData table\n") -- End of strokesData table

    -- Close the file
    file:close()
    print("Wrote shape successfully to " .. filepath)
end

return _M
