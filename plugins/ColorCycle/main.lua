-- Register all Toolbar actions and initialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Cycle forward through color list", ["callback"] = "cycleForward", ["accelerator"] = "<Alt>c"})
  app.registerUi({["menu"] = "Cycle backward through color list", ["callback"] = "cycleBackward", ["accelerator"] = "<Shift><Alt>c"})
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

-- start with first color (0 = uninitialized)
local currentColor = 0 

local function applyCurrentColor()
  -- apply color to currently used tool and allow coloring of elements from selections
  app.changeToolColor({["color"] = colorList[currentColor][2], ["selection"] = true})
end

function cycleForward()
  if (currentColor < #colorList) then
    currentColor = currentColor + 1
  else
    currentColor = 1
  end
  applyCurrentColor()
end

-- Backward compatibility alias
cycle = cycleForward

function cycleBackward()
  if (currentColor > 1) then
    currentColor = currentColor - 1
  else
    currentColor = #colorList
  end
  applyCurrentColor()
end
