function initUi()
  app.registerUi({["menu"] = "Toggle Highlight Position", ["callback"] = "laser", ["accelerator"] = "<Alt>x"});
end

local toggleState = false;

function laser()
  toggleState = not toggleState
  app.changeActionState("position-highlighting", toggleState);
end
