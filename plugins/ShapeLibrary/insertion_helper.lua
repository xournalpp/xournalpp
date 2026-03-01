local sep = package.config:sub(1, 1) -- path separator depends on OS
local sourcePath = debug.getinfo(1).source:match("@?(.*" .. sep .. ")")
local stroke_io = require("stroke_io")

local _M = {}
-- Function to insert strokes for a shape (need to extract the shape name from the dictionary)
function _M.insertStroke(filename, systemMode)
    if not app.addToSelection then print("Missing addToSelection API, consider upgrading Xournal++") return end

    local filepath = systemMode and sourcePath .. "Shapes" .. sep .. filename or app.getFolder("data") .. sep .. filename
    local strokes = stroke_io.readStrokesFromFile(filepath)

    if not strokes or #strokes == 0 then print("No strokes to insert!") return end

    local refs = app.addStrokes({ strokes = strokes , allowUndoRedoAction = "grouped" })
    app.addToSelection(refs)
    app.refreshPage()
end

return _M
