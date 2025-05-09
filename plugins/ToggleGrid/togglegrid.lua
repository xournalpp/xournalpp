
-- Register all Toolbar actions and initialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Toggle Grid Paper", ["callback"] = "toggleGridPaper"});
end

local toggleState = false

function toggleGridPaper()
  if toggleState == true then
    app.uiAction({["action"] = "ACTION_GRID_SNAPPING", ["enabled"] = true});
    app.changeCurrentPageBackground("graph");
  else
    app.uiAction({["action"] = "ACTION_GRID_SNAPPING", ["enabled"] = false});
    app.changeCurrentPageBackground("plain");
  end
  toggleState = not toggleState
end
