
-- Register all Toolbar actions and initialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Toggle Grid Paper", ["callback"] = "toggleGridPaper"});
end

local toggleState = false

function toggleGridPaper()
  if toggleState == true then
    app.changeActionState("grid-snapping", true);
    app.changeCurrentPageBackground("graph");
  else
    app.changeActionState("grid-snapping", false);
    app.changeCurrentPageBackground("plain");
  end
  toggleState = not toggleState
end
