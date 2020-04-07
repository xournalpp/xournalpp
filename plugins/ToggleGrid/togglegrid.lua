
-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Toggle Grid Paper", ["callback"] = "toggleGridPaper"});
end

local toggleState = false

function toggleGridPaper()
  if toggleState == true then
    app.uiActionSelected("GROUP_GRID_SNAPPING", "ACTION_GRID_SNAPPING");
    app.changeCurrentPageBackground("graph");
  else
    app.uiActionSelected("GROUP_GRID_SNAPPING", "ACTION_NONE");
    app.changeCurrentPageBackground("plain");
  end
  toggleState = not toggleState
end