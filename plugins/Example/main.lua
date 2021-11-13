-- This is an example Xournal++ Plugin - copy this to get started

var_dump = require "var_dump"

-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  print("Hello from Example: Plugin initUi called\n");

  ref = app.registerUi({["menu"] = "Test123", ["callback"] = "exampleCallback", ["accelerator"] = "<Control>t"});
  print("Menu reference:");
  var_dump(ref);

  print("Example plugin registered\n");
end

-- Callback if the menu item is executed
function exampleCallback()
  result = app.msgbox("Test123", {[1] = "Yes", [2] = "No"});
  print("result = " .. result)
end
