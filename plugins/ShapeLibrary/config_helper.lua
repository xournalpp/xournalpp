local sep = package.config:sub(1, 1) -- path separator depends on OS
local sourcePath = debug.getinfo(1).source:match("@?(.*" .. sep .. ")")
local filePath = sourcePath .. "config.lua"

local _M = {} -- functions to export

function _M.getShapesData()
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
        print("Config file updated successfully!")
    else
        print("Error: Could not write to file!")
    end
end

function _M.addCategory(newCategoryName)
    if not newCategoryName then print("No category name to add!") return end

    -- Step 1: Load the shapes data from the config file
    local shapesData = _M.getShapesData() or {}

    -- Step 2: Add the new category
    local newCategory = { name = newCategoryName, shapes = {} }
    table.insert(shapesData, newCategory)
    print("New category added: " .. newCategoryName)

    -- Step 3: Write the modified shapes data back into the config file
    writeConfig(shapesData)
end

function _M.removeCategory(categoryName)
    if not categoryName then print("No category name to remove!") return end

    -- Step 1: Load the shapes data from the config file
    local shapesData = _M.getShapesData() or {}

    -- Step 2: Add the new category
    for i, category in ipairs(shapesData) do
        if category.name == categoryName then
            for _, shape in ipairs(category.shapes) do
                local filename = shape.shapeName .. ".lua"
                print("Removing shape: " .. filename)
            end
            table.remove(shapesData, i)
            break
        end
    end

    -- Step 3: Write the modified shapes data back into the config file
    writeConfig(shapesData)
end

-- Function to read, modify, and write back to the shapes file
function _M.renameCategory(oldCategoryName, newCategoryName)
    if not oldCategoryName then print("Error: No category to rename from!") return end
    if not newCategoryName then print("Error: No category to rename to!") return end

    -- Step 1: Load the shapes data from the config file
    local shapesData = _M.getShapesData() or {}

    -- Step 2: Modify the shapes data
    local categoryFound = false
    for _, category in ipairs(shapesData) do
        if category.name == oldCategoryName then
            categoryFound = true
            print("Category " .. oldCategoryName .. " found")
            category.name = newCategoryName
            break
        end
    end
    if not categoryFound then print("Error: Category ".. oldCategoryName .. " not found!") return end

    -- Step 3: Write the modified shapes data back into the config file
    writeConfig(shapesData)
end

function _M.addShape(categoryName, shapeName, fileName)
    if not categoryName then print("No category name to which to add!") return end
    if not shapeName then print("No shape name to add!") return end

    if not fileName then print("No file name to add!") return end

        -- Step 1: Load the shapes data from the config file
        local shapesData = _M.getShapesData() or {}

        -- Step 2: Add the shape
        local found = false
        for _, category in ipairs(shapesData) do
            if category.name == categoryName then
                found = true
                table.insert(category.shapes, { name = shapeName, shapeName = fileName })
                print("Adding shape: " .. shapeName)
                break
            end
            if found then break end
        end

        if not found then print("error: Did not find category " .. categoryName) end

        -- Step 3: Write the modified shapes data back into the config file
        writeConfig(shapesData)

end

function _M.removeShape(categoryName, shapeName)
    if not categoryName then print("No category name from which to remove!") return end
    if not shapeName then print("No shape name to remove!") return end

    -- Step 1: Load the shapes data from the config file
    local shapesData = _M.getShapesData() or {}

    -- Step 2: Add the new category
    local found = false
    for _, category in ipairs(shapesData) do
        if category.name == categoryName then
            for j, shape in ipairs(category.shapes) do
                if shape.name == shapeName then
                    table.remove(category.shapes, j)
                    print("Removing shape: " .. shapeName)
                    found = true
                    break
                end
            end
        end
        if found then break end
    end

    if not found then print("error: Did not find shape " .. shapeName .. " in category " .. categoryName) end

    -- Step 3: Write the modified shapes data back into the config file
    writeConfig(shapesData)
end

-- Function to read, modify, and write back to the shapes file
function _M.renameShape(categoryName, oldName, newName, oldShapeName, newShapeName)
    if not categoryName then print("Error: No category name to update!") return end

    -- Step 1: Load the shapes data from the config file
    local shapesData = _M.getShapesData() or {}

    -- Step 2: Modify the shapes data
    local categoryFound, nameFound = false, false
    for _, category in ipairs(shapesData) do
        if category.name == categoryName then
            categoryFound = true
            print("Category " .. categoryName .. " found")
            for j, shape in ipairs(category.shapes) do
                if shape.name == oldName and shape.shapeName == oldShapeName then
                    category.shapes[j] = { name = newName, shapeName = newShapeName }
                    print("Renamed shape " .. oldName .. " in category " .. category.name .. " to " .. newName)
                    nameFound = true
                    break
                end
                if nameFound then break end
            end
            if categoryFound then break end
        end
    end
    if not nameFound then print("Error: Shape ".. oldName .. " not found!") return end

    -- Step 3: Write the modified shapes data back into the config file
    writeConfig(shapesData)
end

return _M
