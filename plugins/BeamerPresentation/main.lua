local m = {
    dec = 1,
    inc = 2,
}
-- Register all Toolbar actions and intialize all UI stuff
function initUi()
    app.registerUi({menu = "beamer +1", callback = "Beamer", mode=m.inc, accelerator="<Alt>y"});
    app.registerUi({menu = "beamer -1", callback = "Beamer", mode=m.dec, accelerator="<Alt>x"});
    app.registerUi({menu = "beamer clean", callback = "BeamerClean"});
end

function Beamer(mode)
    local struct = app.getDocumentStructure()
    local diff = (mode == m.inc) and 1 or -1


    local pCurr = app.getPageLabel(struct.pages[struct.currentPage].pdfBackgroundPageNo)

    local pNext = app.getPageLabel(struct.pages[struct.currentPage].pdfBackgroundPageNo+diff)

    if pCurr and pNext and pCurr == pNext then
        app.changeBackgroundPdfPageNr(diff, true)
        app.refreshPage()
    else
        app.scrollToPage(diff, true)
    end
end

function BeamerClean()
    local struct = app.getDocumentStructure()
    for p=#struct.pages,2,-1 do
        local page = struct.pages[p].pdfBackgroundPageNo
        local prev = struct.pages[p-1].pdfBackgroundPageNo
        if app.getPageLabel(page) == app.getPageLabel(prev) then
            app.setCurrentPage(p)
            app.activateAction("delete-page")
        end
    end
end
