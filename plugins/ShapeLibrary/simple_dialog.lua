local shapes_dict = require("config")
local insertion_helper = require("insertion_helper")

local _M = {} -- functions to export

-- All functions for "Option Selection" and provide the name of the shape to the "insertStroke" function

-- Showing Main dialog for selecting shape categories (gives the primary options for the Shape-Catagory)
function _M.showMainShapeDialog()
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
    app.openDialog("Select Shape", options, "ShapeDialogCallback")
end

-- Callback for the secondary shape dialog
function ShapeDialogCallback(result)
    local numShapesInCategory = #shapes_dict[currentCategory].shapes
    if result == numShapesInCategory + 1 then
        _M.showMainShapeDialog() -- When "Back" option is selected the secondary dialog closes and main dialog reopens
    elseif result >= 1 and result <= numShapesInCategory then
        local shapeName = shapes_dict[currentCategory].shapes[result].shapeName
        insertion_helper.insertStroke(shapeName) -- Shape name is provided to {function insertStroke(shape_name)} for inserting the strokes of a shape
    end
end

return _M
