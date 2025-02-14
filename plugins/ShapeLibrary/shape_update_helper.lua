local sep = package.config:sub(1, 1) -- path separator depends on OS
local sourcePath = debug.getinfo(1).source:match("@?(.*" .. sep .. ")")
local filePath = sourcePath .. "config.lua"

local _M = {} -- functions to export

local function getShapesData()
    local sandbox = {} -- No access to global variables in order to safely read the file
    setmetatable(sandbox, { __index = function() error("Forbidden function in file " .. filePath, 2) end })

    local f, err = loadfile(filePath, "t", sandbox)
    if not f or err then
        print("Error loading config file: " .. (err or ""))
        return
    end
    return f()
end

local function serializeTable(tbl)
    local result = ""
    for i, v in ipairs(tbl) do
        -- Start each category block
        result = result .. "[" .. i .. "] = {\n"
        result = result .. "    name = \"" .. v.name .. "\",\n"
        result = result .. "    shapes = {\n"

        -- Serialize the shapes within the category
        for j, shape in ipairs(v.shapes) do
            result = result .. "        [" .. j .. "] = {"
            result = result .. " name = \"" .. shape.name .. "\","
            result = result .. " shapeName = \"" .. shape.shapeName .. "\""
            result = result .. " },\n"
        end

        result = result .. "    },\n"
        result = result .. "},\n"
    end
    return result
end

local function writeConfig(shapesData)
    -- Wrap the serialized table in a return statement
    local serializedData = "return {\n" .. serializeTable(shapesData) .. "}\n"

    -- Step 4: Write the updated data back to the file
    local file = io.open(filePath, "w")
    if file then
        file:write(serializedData)
        file:close()
        print("File updated successfully!")
    else
        print("Error: Could not write to file!")
    end
end

function _M.addCategory(newCategoryName)
    if not newCategoryName then print("No category name to add!") return end

    -- Step 1: Load the shapes data from the config file
    local shapesData = getShapesData() or {}

    -- Step 2: Add the new category
    local newCategory = { name = newCategoryName, shapes = {} }
    table.insert(shapesData, newCategory)
    print("New category added: " .. newCategoryName)

    -- Step 3: Write the modified shapes data back into the config file
    writeConfig(shapesData)
end

-- Function to read, modify, and write back to the shapes file
function _M.updateShape(categoryName, shapeName, shapeFileName)
    if not categoryName then print("Error: No category name to update!") return end

    -- Step 1: Load the shapes data from the config file
    local shapesData = getShapesData() or {}

    -- Step 2: Modify the shapes data
    local categoryFound = false
    for _, category in ipairs(shapesData) do
        if category.name == categoryName then
            table.insert(category.shapes, { name = shapeName, shapeName = shapeFileName })
            print("Shape added: " .. shapeName .. " in category " .. category.name)
            categoryFound = true
            break
        end
    end
    if not categoryFound then print("Error: Category ".. categoryName .. " not found!") return end

    -- Step 3: Write the modified shapes data back into the config file
    writeConfig(shapesData)
end

return _M
