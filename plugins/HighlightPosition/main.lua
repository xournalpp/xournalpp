function initUi()
  app.registerUi({["menu"] = "Toggle Highlight Position", ["callback"] = "laser", ["accelerator"] = "<Alt>x"});
end

local togglestate = false;

function laser()
  if toggleState == true then
    app.uiAction({["action"] = "ACTION_HIGHLIGHT_POSITION", ["enabled"] = false});
  else
    app.uiAction({["action"] = "ACTION_HIGHLIGHT_POSITION", ["enabled"] = true});
  end
  toggleState = not toggleState
end
