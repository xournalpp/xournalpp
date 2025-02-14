-- Register Toolbar actions and initialize UI stuff
function initUi()
    -- Getting the source folder Path (The plugin folder-path)

    app.registerUi({
        ["menu"] = "Insert Shapes (simple)",
        ["callback"] = "Show_simple_dialog",
        ["toolbarId"] = "shapedialog",
        ["iconName"] = "shapes_symbolic"
    })
    app.registerUi({
        ["menu"] = "Extract_Stroke_Info (simple)",
        ["callback"] = "Store_stroke_info",
        ["toolbarId"] =
        "StrInfo",
        ["iconName"] = "extract-info-symbolic"
    })
    app.registerUi({
        ["menu"] = "Manage and Insert Shapes (lgi)",
        ["callback"] = "Show_lgi_dialog",
    })
end

function Store_stroke_info()
        local stroke_io = require "stroke_info"
        local strokes = app.getStrokes("selection")
        stroke_io.store_strokes_info_in_file(strokes)
end

function Show_simple_dialog()
    local simple_dialog = require "simple_dialog"
    simple_dialog.showMainShapeDialog()
end

function Show_lgi_dialog()
    local lgi_dialog = require "lgi_dialog"
    lgi_dialog.showMainShapeDialog()
end
