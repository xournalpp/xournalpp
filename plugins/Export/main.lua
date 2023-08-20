-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Export to pdf", ["callback"] = "exportPdf", ["accelerator"] = "<Shift><Alt>p"});
  app.registerUi({["menu"] = "Export to svg", ["callback"] = "exportSvg", ["accelerator"] = "<Shift><Alt>s"});
  app.registerUi({["menu"] = "Export to png", ["callback"] = "exportPng", ["accelerator"] = "<Shift><Alt>n"});
  app.registerUi({["menu"] = "Start png auto-export", ["callback"] = "startAutoExport"})
  app.registerUi({["menu"] = "End png auto-export", ["callback"] = "endAutoExport"})
end

local DEFAULT_PATH = "/tmp/temp"  -- change this to get a different default path for xopp-files that have not been saved yet
local MAX_DELAY = 5 -- seconds

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
local lastExportTimes = {}
local lastUpdateTimes = {}

function updatePageTime(page)
  if not running then return end 
  time = os.time()
  lastUpdateTimes[page+1] = time; -- in Lua indices (page numbers) start with 1
  print("Updated time for page " .. page+1)
end

function updatePageTimesFrom(page) 
  if not running then return end
  local noPages = #app.getDocumentStructure()["pages"]
  for i=page, noPages-1 do
    updatePageTime(i)
  end
end

function updateAllPageTimes(page)
  updatePageTimesFrom(0)
end

function exportPages()
  if not running then return end
  print("exportPages called...")
  time = os.time()
  range = ""

  local noPages = #app.getDocumentStructure()["pages"]
  
  for i=1, math.min(#lastUpdateTimes, noPages) do
    if lastExportTimes[i]==nil or lastExportTimes[i]<lastUpdateTimes[i] then
      separator = (range=="") and "" or ", "
      range = range .. separator .. tostring(i)
    end
    lastExportTimes[i] = time
    
    if range ~="" then
      print("Exporting pages within range: ", range)
      local pageIndicator = (string.find(range, ",")==nil) and "-" .. range or ""
      local pngName = getStem() .. pageIndicator .. ".png"
      app.export({["outputFile"] = pngName, ["range"] = range})
    end
  end
  return running
end

local timeoutId

function startAutoExport()
  print("Automatic export of changed pages in png format will start. ")
  app.connectToSignal("pageChanged", "updatePageTime")
  app.connectToSignal("pageSizeChanged", "updatePageTime")
  app.connectToSignal("documentChanged", "updateAllPageTimes")
  app.connectToSignal("pageInserted", "updatePageTimesFrom")
  app.connectToSignal("pageDeleted", "updatePageTimesFrom")
  app.connectToSignal("undoRedoPageChanged", "updatePageTime")
  app.connectToSignal("quit", "onQuit")
  timeoutId = app.addTimeout(MAX_DELAY, "exportPages")
  running = true
end

function endAutoExport()
  print("Automatic export of changed pages in png format is ended. ")
  app.disconnectFromSignal("pageChanged")
  app.disconnectFromSignal("pageSizeChanged")
  app.disconnectFromSignal("documentChanged")
  app.disconnectFromSignal("pageInserted")
  app.disconnectFromSignal("pageDeleted")
  app.disconnectFromSignal("undoRedoPageChanged")
  app.disconnectFromSignal("quit")
  app.removeTimeout(timeoutId)
  running = false
end

function onQuit()
  if running then 
    endAutoExport()
    print("Bye!")
  end
end
