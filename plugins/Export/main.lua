-- Register all Toolbar actions and initialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Export to pdf", ["callback"] = "exportPdf", ["accelerator"] = "<Shift><Alt>p"});
  app.registerUi({["menu"] = "Export to svg", ["callback"] = "exportSvg", ["accelerator"] = "<Shift><Alt>s"});
  app.registerUi({["menu"] = "Export to png", ["callback"] = "exportPng", ["accelerator"] = "<Shift><Alt>n"});
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
