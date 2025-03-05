local sep = package.config:sub(1, 1) -- path separator depends on OS
local sourcePath = debug.getinfo(1).source:match("@?(.*" .. sep .. ")")
local systemFilePath = sourcePath .. "config.lua"
local userFilePath = app.getFolder("config") .. sep .. "config.lua"

local _M = {} -- functions to export


local function writeConfig(shapesData)

    -- Write the updated data back to the file
    local file = io.open(userFilePath, "w")
    if not file then print("Error: Could not write to file!") return end

    file:write("return {\n")
    for i, v in ipairs(shapesData) do
        -- Start each category block
        file:write("[" .. i .. "] = {\n")
        file:write("    name = \"" .. v.name .. "\",\n")
        file:write("    shapes = {\n")

        -- Serialize the shapes within the category
        for j, shape in ipairs(v.shapes) do
            file:write("        [" .. j .. "] = {")
            file:write(" name = \"" .. shape.name .. "\",")
            file:write(" filename = \"" .. shape.filename .. "\"")
            file:write(" },\n")
        end

        file:write("    },\n")
        file:write("},\n")
    end

    file:write("}\n")
    file:close()
    print("Config file updated successfully!")
end

local function ensureUserConfigExists()
    local f = io.open(userFilePath, "r")
    if f == nil then -- file cannot be opened
        print("Create fresh user config file in " .. userFilePath)
        writeConfig({})
    end
end

function _M.getShapesData(systemMode) -- systemMode defaults to nil, i.e. user mode
    local sandbox = {} -- No access to global variables in order to safely read the file
    local filePath = systemMode and systemFilePath or userFilePath
    if not systemMode then
        ensureUserConfigExists()
    end
    setmetatable(sandbox, { __index = function() error("Forbidden function in file " .. filePath, 2) end })


    local f, err = loadfile(filePath, "t", sandbox)
    if not f or err then
        print("Error loading config file: " .. (err or ""))
        return
    end
    return f()
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

    -- Step 2: Remove the category
    local categoryFound = false
    for i, category in ipairs(shapesData) do
        if category.name == categoryName then
            categoryFound = true
            table.remove(shapesData, i)
            print("Removing category: " .. categoryName)
            break
        end
    end

    if not categoryFound then print("Error: Category ".. categoryName .. " not found!") return end

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
            print("Renaming category " .. oldCategoryName .. " to " .. newCategoryName)
            category.name = newCategoryName
            break
        end
    end
    if not categoryFound then print("Error: Category ".. oldCategoryName .. " not found!") return end

    -- Step 3: Write the modified shapes data back into the config file
    writeConfig(shapesData)
end

function _M.addShape(categoryName, shapeName, filename)
    if not categoryName then print("No category name to which to add!") return end
    if not shapeName then print("No shape name to add!") return end

    if not filename then print("No file name to add!") return end

        -- Step 1: Load the shapes data from the config file
        local shapesData = _M.getShapesData() or {}

        -- Step 2: Add the shape
        local found = false
        for _, category in ipairs(shapesData) do
            if category.name == categoryName then
                found = true
                table.insert(category.shapes, { name = shapeName, filename = filename })
                print("Adding shape: " .. shapeName)
                break
            end
            if found then break end
        end

        if not found then print("Error: Did not find category " .. categoryName) return end

        -- Step 3: Write the modified shapes data back into the config file
        writeConfig(shapesData)

end

function _M.removeShape(categoryName, shapeName)
    if not categoryName then print("No category name from which to remove!") return end
    if not shapeName then print("No shape name to remove!") return end

    -- Step 1: Load the shapes data from the config file
    local shapesData = _M.getShapesData() or {}

    -- Step 2: Remove shape
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

    if not found then print("Error: Did not find shape " .. shapeName .. " in category " .. categoryName) return end

    -- Step 3: Write the modified shapes data back into the config file
    writeConfig(shapesData)
end

-- Function to read, modify, and write back to the shapes file
function _M.renameShape(categoryName, oldName, newName, oldFilename, newFilename)
    if not categoryName then print("Error: No category name to update!") return end

    -- Step 1: Load the shapes data from the config file
    local shapesData = _M.getShapesData() or {}

    -- Step 2: Modify the shapes data
    local categoryFound, nameFound = false, false
    for _, category in ipairs(shapesData) do
        if category.name == categoryName then
            categoryFound = true
            for j, shape in ipairs(category.shapes) do
                if shape.name == oldName and shape.filename == oldFilename then
                    category.shapes[j] = { name = newName, filename = newFilename }
                    print("Renaming shape " .. oldName .. " in category " .. category.name .. " to " .. newName)
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
