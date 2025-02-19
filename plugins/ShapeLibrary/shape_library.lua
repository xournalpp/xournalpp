-- Register Toolbar actions and initialize UI stuff
function initUi()
    -- Getting the source folder Path (The plugin folder-path)

    app.registerUi({
        ["menu"] = "Insert Shapes (simple)",
        ["callback"] = "ShowSimpleDialog",
        ["toolbarId"] = "shapedialog",
        ["iconName"] = "shapes_symbolic"
    })
    app.registerUi({
        ["menu"] = "Extract Stroke Info (simple)",
        ["callback"] = "StoreStrokeInfo",
        ["toolbarId"] =
        "StrInfo",
        ["iconName"] = "extract-info-symbolic"
    })
    app.registerUi({
        ["menu"] = "Manage and Insert Shapes (lgi)",
        ["callback"] = "ShowLgiDialog",
    })
end

function StoreStrokeInfo()
        local stroke_io = require("stroke_io")
        local strokes = app.getStrokes("selection")
        stroke_io.storeStrokeInfoInFile(strokes, sourcePath .. "giveMeName_and_placeMe_in_shapesFolder.lua")
end

function ShowSimpleDialog()
    local simple_dialog = require("simple_dialog")
    simple_dialog.showMainShapeDialog()
end

function ShowLgiDialog()
    local lgi_dialog = require("lgi_dialog")
    lgi_dialog.showMainShapeDialog()
end
