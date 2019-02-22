
-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Grid with snapping On", ["callback"] = "toggleGridOn"});
  app.registerUi({["menu"] = "Grid with snapping Off", ["callback"] = "toggleGridOff"});
end

function toggleGridOn()
  app.uiActionSelected("GROUP_GRID_SNAPPING", "ACTION_GRID_SNAPPING");
  app.changeCurrentPageBackground("graph");
end

function toggleGridOff()
  app.uiActionSelected("GROUP_GRID_SNAPPING", "ACTION_NONE");
  app.changeCurrentPageBackground("plain");
end
