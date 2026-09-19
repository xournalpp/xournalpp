-- Register Toolbar actions and initialize UI stuff
function initUi()
    app.registerUi {
        menu = "Manage and Insert Shapes",
        callback = "ShowLgiDialog",
        accelerator = "s",
        toolbarID = "SHAPE_LIBRARY",
        iconName = "shapes_symbolic.svg"
    }
end
function ShowLgiDialog()
    local lgi_dialog = require("lgi_dialog")
    lgi_dialog.showMainShapeDialog()
end
