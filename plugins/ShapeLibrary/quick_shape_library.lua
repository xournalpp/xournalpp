-- Register Toolbar actions and initialize UI stuff

local shapes_dict, sep, sourcePath

function initUi()
    -- Getting the source folder Path (The plugin folder-path)
    sep = package.config:sub(1, 1) -- path separator depends on OS
    sourcePath = debug.getinfo(1).source:match("@?(.*" .. sep .. ")")

    shapes_dict = require "config"
    app.registerUi({
        ["menu"] = "Insert Shapes",
        ["callback"] = "showMainShapeDialog",
        ["toolbarId"] = "shapedialog",
        ["iconName"] = "shapes_symbolic"
    })
    app.registerUi({
        ["menu"] = "Extract_Stroke_Info",
        ["callback"] = "store_stroke_info_in_file",
        ["toolbarId"] =
        "StrInfo",
        ["iconName"] = "extract-info-symbolic"
    })
end

-- Function to read and provide formatted stroke data from a shape-file
function read_strokes_from_file(filepath)
    if filepath == nil then return end
    local hasFile, content = pcall(dofile, filepath)
    if not hasFile then print("Error: " .. content) return end
    local strokesToAdd = {}

    for _, stroke in ipairs(content) do
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

-- Function to insert strokes for a shape (need to extract the shape name from the dictionary)
function insert_stroke(shape_name)
    local filepath = sourcePath .. "Shapes" .. sep .. shape_name .. ".lua"
    local strokes = read_strokes_from_file(filepath)
    if strokes and #strokes > 0 then
        app.addStrokes({ strokes = strokes , allowUndoRedoAction = "grouped" })
        app.refreshPage()
    end
end



-- All functions for "Option Selection" and provide the name of the shape to the "insert_stroke" function

-- Showing Main dialog for selecting shape categories (gives the primary options for the Shape-Catagory)
function showMainShapeDialog()
    local options = {}
    for i, category in ipairs(shapes_dict) do
        table.insert(options, category.name)
    end
    table.insert(options, "Cancel") -- Add Cancel option
    app.openDialog("Select Shape Category", options, "mainShapeDialogCallback")
end

-- Callback for the main shape dialog
local currentCategory = nil --At first its value is nil, it gets value after mainShapeDialogCallback is called
function mainShapeDialogCallback(result)
    if result == #shapes_dict + 1 then
        -- When "Cancel" option is clicked it closes the main dialog
        return
    elseif result >= 1 and result <= #shapes_dict then
        currentCategory = result -- gets currentCategory from here
        showShapeDialog(result)
    end
end

-- Showing secondary dialog for selecting shape (gives the secondary options for selecting shapes)
function showShapeDialog(categoryIndex)
    local category = shapes_dict[categoryIndex]

    local options = {}

    for i, shape in ipairs(category.shapes) do
        table.insert(options, shape.name)
    end

    table.insert(options, "Back") -- Add Back option

    -- Open the dialog with shape names and Back option
    app.openDialog("Select Shape", options, "shapeDialogCallback")
end

-- Callback for the secondary shape dialog
function shapeDialogCallback(result)
    local numShapesInCategory = #shapes_dict[currentCategory].shapes
    if result == numShapesInCategory + 1 then
        showMainShapeDialog() -- When "Back" option is selected the secondary dialog closes and main dialog reopens
    elseif result >= 1 and result <= numShapesInCategory then
        local shapeName = shapes_dict[currentCategory].shapes[result].shapeName
        insert_stroke(shapeName) -- Shape name is provided to {function insert_stroke(shape_name)} for inserting the strokes of a shape
    end
end

-- All codes for Extract stroke information (the file is saved in plugin folder, you have to rename it and place it in "shapes" folder, also add the name in shape dictionary)
-- Function to store stroke information in a file
function store_stroke_info_in_file()
    -- Get the selected strokes
    local strokes = app.getStrokes("selection")

    -- Open a file for writing in the folder path
    local file = assert(io.open(sourcePath .. "giveMeName_and_placeMe_in_shapesFolder.lua", "w"))
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
    file:write("   -- Return the strokesData table") -- End of strokesData table

    -- Close the file
    file:close()
end
