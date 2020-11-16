-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Cycle through color list", ["callback"] = "cycle", ["accelerator"] = "<Alt>c"});
  -- if you want to have multiple color lists you must use the app.registerUi function multiple times
  -- with different callback functions and accelerators
end

-- Predefined colors copied from LoadHandlerHelper.cpp 
-- modify to your needs 
local colorList = { 
  {"black", 0x000000},  
  {"green", 0x008000},
  {"lightblue", 0x00c0ff}, 
  {"lightgreen", 0x00ff00}, 
  {"blue", 0x3333cc},      
  {"gray", 0x808080},   
  {"red", 0xff0000},        
  {"magenta", 0xff00ff},
  {"orange", 0xff8000}, 
  {"yellow", 0xffff00},    
  {"white", 0xffffff}
}

-- start with first color
local currentColor = 0 

function cycle()
  if (currentColor < #colorList) then
    currentColor = currentColor + 1
  else
    currentColor = 1
  end
  -- apply color to currently used tool and allow coloring of elements from selections
  app.changeToolColor({["color"] = colorList[currentColor][2], ["selection"] = true})
  -- use app.changeColor({["color"] = colorList[currentColor][2], ["tool"] = "pen""}) 
  -- instead if you only want to change pen color
  -- similarly if you want to change hilighter color or the color from another tool with color capabilities.
end
