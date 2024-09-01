local m = {
    NORMAL = 0,
    FORCE = 1,
}

-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  app.registerUi{menu="Add one empty pages after all pdf page once", callback="AddEmpty", mode=m.NORMAL}
  app.registerUi{menu="Add empty pages after every page", callback="AddEmpty", mode=m.FORCE}
end

function AddEmpty(mode)
    local struct = app.getDocumentStructure()
    for p=#struct.pages,1,-1 do
        local page = struct.pages[p]
        local pred = true
        if mode ~= m.FORCE then
            -- page p is a pdf page
            pred = page.pdfBackgroundPageNo ~= 0
            if pred then
                -- check if page p+1 is also a pdf page (makes the operation idempotent)
                local n = struct.pages[p+1]
                -- no p is the last page -> also insert an empty one
                if n == nil then
                    pred = true
                else
                    pred = n.pdfBackgroundPageNo ~= 0
                end
            end
        end
        if pred or mode == m.FORCE then
            app.setCurrentPage(p)
            app.uiAction{action="ACTION_NEW_PAGE_AFTER"}
        end
    end
end
