-- Register all Toolbar actions and initialize all UI stuff
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
    app.openDialog("No next page. ", {"OK"}, "", true)
    return
  end

  local bgNextPage = docStructure["pages"][nextPage]["pageTypeFormat"]
  if bgNextPage ~= ":pdf" then
    app.openDialog("Next page has no pdf background. ", {"OK"}, "", true)
    return
  end

  local isAnnotated = docStructure["pages"][nextPage]["isAnnotated"]
  if isAnnotated then
    app.openDialog("Next page contains annotations that will be lost, when proceeding. ", {"Cancel", "Proceed"}, "proceedIf")
    return
  end

  proceed()
end

function proceedIf(result)
  if result == 2 then
    proceed()
  end
end

function proceed()
  -- Copy the page, change its background to the background of the next pdf page and delete the old page without cloned layers
  local docStructure = app.getDocumentStructure()
  local currentPage = docStructure["currentPage"]
  local nextPage = currentPage + 1
  local numPages = #docStructure["pages"]

  local nextPdfPage = docStructure["pages"][nextPage]["pdfBackgroundPageNo"]

  local sidebarPage = app.getSidebarPageNo()
  app.setSidebarPageNo(2)
  app.sidebarAction("COPY");
  app.setSidebarPageNo(sidebarPage)

  app.changeBackgroundPdfPageNr(nextPdfPage, false);
  app.refreshPage()
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
