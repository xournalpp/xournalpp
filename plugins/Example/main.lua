-- This is an example Xournal++ Plugin - copy this to get started

var_dump = require "var_dump"

-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  print("Hello from Example: Plugin initUi called\n");

  ref = app.registerUi({["menu"] = "Test123", ["callback"] = "exampleCallback", ["accelerator"] = "<Control>t",
                        ["toolbarId"] = "ExamplePlugin", ["iconName"] = "mail-attachment"});
  print("Menu reference:");
  var_dump(ref);

  print("Example plugin registered\n");
end

-- Callback if the menu item is executed
function exampleCallback()
  app.openDialog("Test123", {"Yes", "No"}, "dialogCallback");
end

-- Callback for when the user click one of the dialog's button
function dialogCallback(result)
  print("result = " .. result)
end
