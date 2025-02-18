-- This is an Xournal++ Plugin - copy this to get started

local var_action_index = 1

-- The PEN, HIGHLIGHTER, and HAND tools are predefined, but you can replace them with the ones you need.
local var_action_array = {"ACTION_TOOL_PEN", "ACTION_TOOL_HIGHLIGHTER", "ACTION_TOOL_HAND"}

-- Register all Toolbar actions and intialize all UI stuff
function initUi()

  -- I use [ Shift + Control + S ], but you can replace it with whatever you need.
  ref = app.registerUi({["menu"] = "Switch Pen Mode", ["callback"] = "switchPenMode", ["accelerator"] = "<Shift><Control>s"});

end

-- Callback if the menu item is executed
-- Currently, the tool index can only be memorized within the plugin, and it is not yet possible to perceive the program state.
function switchPenMode() 
  app.uiAction({["action"] = var_action_array[var_action_index] })

  var_action_index = var_action_index + 1

  if var_action_index > #var_action_array then
    var_action_index = 1
  end
  
end

