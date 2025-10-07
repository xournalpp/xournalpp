local m = {
    NORMAL = 0,
    FORCE = 1,
}

-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  app.registerUi{menu="Add one empty pages after every pdf page once", callback="AddEmpty", mode=m.NORMAL}
  app.registerUi{menu="Add empty pages after every page", callback="AddEmpty", mode=m.FORCE}
end

function AddEmpty(mode)
    local struct = app.getDocumentStructure()
    for p=#struct.pages,1,-1 do
        local page = struct.pages[p]
        local pred = true
        if mode ~= m.FORCE then
            local nextPage = struct.pages[p+1]
            -- insert a new empty page if p is a pdf-page and succeeding page either does not exist or is also a pdf-page
            pred = page.pdfBackgroundPageNo ~= 0 and (nextPage == nil or nextPage.pdfBackgroundPageNo ~= 0)
        end
        -- if mode is force => pred is set to true
        if pred then
            app.setCurrentPage(p)
            app.activateAction("new-page-after")
        end
    end
end
