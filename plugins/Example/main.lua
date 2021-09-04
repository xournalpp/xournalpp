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

  toolInfo = app.getToolInfo("active")
  print("active tool: " .. toolInfo["type"])
  print("active size (name): " .. toolInfo["size"]["name"])
  print("active size (value): " .. toolInfo["size"]["value"])
  print("active color: " .. string.format("0x%06x", toolInfo["color"]))
  print("active fill opacity: " .. tostring(toolInfo["fillOpacity"]))
  print("active drawing type: " .. toolInfo["drawingType"])
  print("active line style: " .. toolInfo["lineStyle"])

  penInfo = app.getToolInfo("pen")
  print("pen size (name): " .. penInfo["size"]["name"])
  print("pen size (value): " .. penInfo["size"]["value"])
  print("pen filled: " .. tostring(penInfo["filled"]))
  print("pen fill opacity: " .. tostring(penInfo["fillOpacity"]))
  print("pen color: " .. string.format("0x%06x", penInfo["color"]))
  print("pen drawing type: " .. penInfo["drawingType"])
  print("pen line style: " .. penInfo["lineStyle"])

  highlighterInfo = app.getToolInfo("highlighter")
  print("highlighter size (name): " .. highlighterInfo["size"]["name"])
  print("highlighter size (value): " .. highlighterInfo["size"]["value"])
  print("highlighter filled: " .. tostring(highlighterInfo["filled"]))
  print("highlighter fill opacity: " .. tostring(highlighterInfo["fillOpacity"]))
  print("highlighter color: " .. string.format("0x%06x", highlighterInfo["color"]))
  print("highlighter drawing type: " .. highlighterInfo["drawingType"])

  eraserInfo = app.getToolInfo("eraser")
  print("eraser type: " .. eraserInfo["type"])
  print("eraser size (name): " .. tostring(eraserInfo["size"]["name"]))
  print("eraser size (value): " .. tostring(eraserInfo["size"]["value"]))

  textInfo = app.getToolInfo("text")
  print("text font (name): " .. textInfo["font"]["name"])
  print("text font (size): " .. tostring(textInfo["font"]["size"]))
  print("text color: " .. string.format("0x%06x", textInfo["color"]))
end
