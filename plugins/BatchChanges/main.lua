local m = {
    FORCE = 1,
}

-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  app.registerUi{menu="Add one empty pages after all pdf page once", callback="AddEmpty", mode=m.FORCE}
  app.registerUi{menu="Add empty pages after every page", callback="AddEmpty", mode=m.FORCE}

  -- app.registerUi{menu="Background for editing", callback="SetBackgroundAll", mode=0}
  -- app.registerUi{menu="Background for printing", callback="SetBackgroundAll", mode=1}
end

function AddEmpty(mode)
    local struct = app.getDocumentStructure()
    for p=#struct.pages,1,-1 do
        local page = struct.pages[p]
        local pred
        if not mode == m.FORCE then
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

-- function SetBackgroundAll(mode)
--     local struct = app.getDocumentStructure()
--     for p in ipairs(struct.pages) do
--         app.setCurrentPage(p)
--         local color = 0
--         if mode == 0 then
--             color = 0x0
--         else
--             color = 0x1
--         end
--         app.changeCurrentPageBackground{color=color}
--     end
-- end
