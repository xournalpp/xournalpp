-- The QuickScreenshot plugin:
--
-- Allows the user to save a region to disk as png image.
-- To be used so that users can save various diagrams, etc
-- to their directory of choice; useful when creating a lot
-- of diagrams for presentation.
--
-- TODO:
--  o Support for Windows and macOS is missing. To add support
--    you must edit the functions 'existsUtility' and 'go' to
--    use the right system programs.
--

-- Returns 'true' if the system utility exists;
-- returns 'false' otherwise. This is a dangerous function,
-- since the argument to popen is not sanitized.
-- The programmer must ensure that the argument to
-- existsUtility() is sanitized (not malicious).
function existsUtility(utility)
  if operatingSystem == "Windows" then
    -- Windows not supported yet.
  elseif operatingSystem == "Darwin" then
    -- macOS not supported yet.
  else
    local f = assert(io.popen("command -v " .. utility))
    local s = assert(f:read('*a'))
    f:close()
    return s ~= ""
  end
end

-- Return one of "Windows", "Linux", "Darwin", "FreeBSD",
-- "OpenBSD" or "NetBSD", etc.
function findOS()
  if package.config:sub(1,1) == '\\' then
    return "Windows"
  else
    local f = assert(io.popen("uname -s", 'r'))
    local s = assert(f:read('*a'))
    f:close()
    return s
  end
end

-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  ref = app.registerUi({["menu"] = "QuickScreenshot", ["callback"] = "go", ["accelerator"] = "<Shift><Alt>t"});
  operatingSystem = findOS() -- What's the operating system?
end

-- This is the callback that gets executed when the user
-- activates the plugin via the menu or hotkey.
--
-- This function executes a screenshot utility to capture
-- the region the user selects. Then, it calls 'app.saveAs'
-- to create a "Save As" dialog for the user to choose the
-- filename where the image should reside.
function go()
  local windowsUtilities = {} -- list of windows programs here
  local macUtilities = {} -- list of macOS programs here
  local elseUtilities = -- list of programs for other systems
                        -- maim and xclip are not listed, because they
                        -- are the default
    { "scrot --overwrite --select --silent "
-- gnome-screenshot behaves oddly (pollutes the window)
-- the line is added here to be indicative of what
-- adding another utility looks like.
--
--  , "gnome-screenshot --area --file="
    }

  -- Different operating systems have different
  -- screenshot utilities, so we check the OS here
  if operatingSystem == "Windows" then
    app.msgbox("Windows not supported yet.", {[1] = "OK"})
  elseif operatingSystem == "Darwin" then
    app.msgbox("macOS not supported yet.", {[1] = "OK"})
  else
    -- Skip all the detection of tools in case maim and xclip are
	-- installed. Then a screenshot is made with maim and saved to the
	-- clipboard, skipping all other tools
    if existsUtility("maim") and existsUtility("xclip") then
      os.execute("maim -s | xclip -selection clipboard -t image/png")
      return
    end
    -- This becomes true if at least one screenshot
    -- utility is available on the system
    local foundUtility = false
    -- Check utility availability and use it
    for i,command in ipairs(elseUtilities) do
      utilityName = command:match("%S+")
      if existsUtility(utilityName) then
        local tmpFilename = os.tmpname() .. ".png" 
        -- The file extension is added in order to avoid the giblib error: no image grabbed
        -- see https://stackoverflow.com/questions/26326664/scrot-giblib-error-saving-to-file-failed
        local runCommand = assert(io.popen(command .. tmpFilename .. " &"))
        runCommand:read('*all')
        runCommand:close()

        -- Launch the "Save As" dialog
        local filename = app.saveAs("Untitled.png")
        if not filename then
          os.remove(tmpFilename)
          return
        end

	local res, msg = app.glib_rename(tmpFilename, filename)
        if not res then print(msg) end
        foundUtility = true
        break
      end
    end
    -- No utility available on the system, inform the user
    if not foundUtility then
      print("Missing screenshot utility.")
      print("Please install one of:")
      for i,command in ipairs(elseUtilities) do
        print(command:match("%S+"))
      end
      return
    end
  end
end
