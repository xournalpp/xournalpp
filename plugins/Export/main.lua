-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Export to pdf", ["callback"] = "exportPdf", ["accelerator"] = "<Shift><Alt>p"});
  app.registerUi({["menu"] = "Export to svg", ["callback"] = "exportSvg", ["accelerator"] = "<Shift><Alt>s"});
  app.registerUi({["menu"] = "Export to png", ["callback"] = "exportPng", ["accelerator"] = "<Shift><Alt>n"});
  app.registerUi({["menu"] = "Start auto-exporting", ["callback"] = "startAutoExport"})
  app.registerUi({["menu"] = "End auto-exporting", ["callback"] = "endAutoExport"})
end

local DEFAULT_PATH = "/tmp/temp"  -- change this to get a different default path for xopp-files that have not been saved yet

function exportPdf()
  local pdfName = getStem() .. "_export.pdf"
  app.export({["outputFile"] = pdfName})
  -- use the "range", "background" and "progressiveMode" fields for more customization
end

function exportSvg()  
  local svgName = getStem() .. "_export.svg"
  app.export({["outputFile"] = svgName})
  -- use the "range", "background" for more customization
end

function exportPng()
  local pngName = getStem() .. "_export.png"
  app.export({["outputFile"] = pngName})
  -- use the "range", "background" and "pngDpi", "pngWidth" or "pngHeight" fields for more customization
end

function getStem()
  local xoppName = app.getDocumentStructure()["xoppFilename"]
  if (xoppName ~= "" and xoppName ~= nil) then 
    return xoppName:match("(.+)%..+$")
  else
    print("Exporting to default path. Consider saving the xopp-file before exporting! ")
    return DEFAULT_PATH
  end
end

local running = false 
local lgi = require 'lgi'
local GLib = lgi.GLib
local main_loop = GLib.MainLoop()
local lastExportTimes = {}

function startAutoExport()
  print("Automatic export of changed pages in png format will start. ")
  running = true

  function callback()
    if running then
      local updateTimes = app.getChangeTimes()
      print("Last document change time: " .. updateTimes["document"])
      pageTimes = updateTimes["pages"]
      range = ""
      for i=1, #pageTimes do
        if lastExportTimes[i]~= nil and lastExportTimes[i]<pageTimes[i] then
          separator = (range=="") and "" or ", "
          range = range .. separator .. tostring(i)
        end
        lastExportTimes[i] = pageTimes[i]
      end
      if range ~="" then
        print("Exporting pages within range: ", range)
        local pageIndicator = (string.find(range, ",")==nil) and "-" .. range or ""
        local pngName = getStem() .. pageIndicator .. ".png"
        app.export({["outputFile"] = pngName, ["range"] = range})
      end
    end    
    return running
  end

  local milliseconds = 500
  GLib.timeout_add(GLib.PRIORITY_DEFAULT, milliseconds, callback)
  main_loop:run()
end

function endAutoExport()
  print("Automatic export of changed pages in png format is ended. ")
  running = false
  main_loop:quit()
end