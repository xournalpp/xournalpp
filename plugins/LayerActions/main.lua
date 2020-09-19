-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Clone non-background layers to next page", ["callback"] = "clone", ["accelerator"]="<Control><Shift>c"});
  app.registerUi({["menu"] = "Hide all layers except first layers and backgrounds", ["callback"] = "hide"});
  app.registerUi({["menu"] = "Add new top layer on each page", ["callback"] = "add"});
end

function clone()
  local docStructure = app.getDocumentStructure()
  local currentPage = docStructure["currentPage"]
  local nextPage = currentPage + 1
  local numPages = #docStructure["pages"]

  -- Make sure there is a next page, it has pdf background and no annotations or the annotations can be overwritten
  if currentPage == numPages then
    app.msgbox("No next page. ", {[1] = "OK"})
    return
  end
  
  local bgNextPage = docStructure["pages"][nextPage]["pageTypeFormat"]
  if bgNextPage ~= ":pdf" then
    app.msgbox("Next page has no pdf background. ", {[1] = "OK"})
    return
  end

  local nextPdfPage = docStructure["pages"][nextPage]["pdfBackgroundPageNo"]
  print(nextPdfPage)

  local isAnnotated = docStructure["pages"][nextPage]["isAnnotated"]
  if isAnnotated then
    local res = app.msgbox("Next page contains annotations that will be lost, when proceeding. ", {[1]="Cancel", [2]="Proceed"})
    if res == 1 then
      return
    end
  end   

  -- Copy the page, change its background to the background of the next pdf page and delete the old page without cloned layers
  app.sidebarAction("COPY");
  app.changeBackgroundPdfPageNr(nextPdfPage, false);
  app.uiAction({["action"]="ACTION_GOTO_NEXT"})
  app.uiAction({["action"]="ACTION_DELETE_PAGE"})
  if currentPage < numPages -1 then
    app.uiAction({["action"]="ACTION_GOTO_BACK"})
  end
end

function hide()
  local docStructure = app.getDocumentStructure()
  local page = docStructure["currentPage"]
  local numPages = #docStructure["pages"]

  for i=1, numPages do
    app.setCurrentPage(i)
    app.setCurrentLayer(1, true)  -- makes background layer and layer 1 visible and all other layers invisible
  end
  
  app.setCurrentPage(page)
end

function add()
  local docStructure = app.getDocumentStructure()
  local numPages = #docStructure["pages"]
  local page = docStructure["currentPage"]
  
  for i=1, numPages do
    app.setCurrentPage(i)
    app.layerAction("ACTION_GOTO_TOP_LAYER")
    app.layerAction("ACTION_NEW_LAYER")  
  end
  
  app.setCurrentPage(page)
end
