function initUi()
  app.registerUi({["menu"] = "Toggle Highlight Position", ["callback"] = "laser", ["accelerator"] = "<Alt>x"});
end

function laser()
  app.uiAction({["action"] = "ACTION_HIGHLIGHT_POSITION"})
end
