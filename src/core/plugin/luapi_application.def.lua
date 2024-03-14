---@meta

--*** app ***
app = {}

---Simple dialog window -- DEPRECATED
---@param message string
---@param options table<integer, string> integer to button texts
---@return integer integer-key of the selected button
function app.msgbox(message, options) end

---Simple dialog window
---@param message string
---@param options table<integer, string> integer to button texts
---@param cb string name of the callback function(button:integer) to call on user interaction
---@param error boolean is this message an error (optional)
function app.openDialog(message, options, cb, error) end

---Renames file 'from' to file 'to' in the file system.
---@param from string 
---@param to string
---@return number|nil Returns 1 on success, and (nil, message) on failure.
---@return string
function app.glib_rename(from, to) end

---"Save as" dialog
---@param filename string suggestion for a filename, defaults to "untitlted"
---@return string path of the selected location
function app.saveAs(filename) end

---Allow to register menupoints/toolbar icons, this needs to be called from initUi
---@param opts {menu: string, callback: string, toolbarID: string, mode:integer, accelerator:string} options (`mode`, `toolbarID` and `accelerator` are optional)
---@return {menuId:integer}
function app.registerUi(opts) end

---Execute an UI action (usually internally called from Toolbar / Menu)
---@param opts {action: string, group: string, enabled:boolean} options (`enabled` is `true` by default, `group` is only for debugging)
function app.uiAction(opts) end

---Execute an action from the sidebar menu
---@param action string the desired action
function app.sidebarAction(action) end

---Execute an action from layer controller
---@param action string the desired action
function app.layerAction(action) end

---Change color of a specified tool or of the current tool 
---@param opts {color: integer, tool: string, selection: boolean} options (`selction=true` -> new color applies to active selection as well, `tool=nil` (unset) -> currently active tool is being used)
function app.changeToolColor(opts) end

---Get all colors of the currently active color palette
---@return {color: integer, name: string} options
function app.getColorPalette() end

---Change page background of current page
---@param background string backgroundType (e.g. "graph")
function app.changeCurrentPageBackground(background) end

---Select Background Pdf Page for Current Page
---@param pageNr integer page-nunmber
---@param relative boolean should `pageNr` be interpreted relative?
function app.changeBackgroundPdfPageNr(pageNr, relative) end

---Returns a table encoding all info on the chosen tool
---@param tool string tool to get the information for
---@return {} info varies a lot depending on the tool
function app.getToolInfo(tool) end

---Get the index of the currently active sidebar-page
---@return integer pageNr pageNr of the sidebar page
function app.getSidebarPageNo() end

---Set the index of the currently active sidebar-page
---@param pageNr integer pageNr of the sidebar page
function app.setSidebarPageNo(pageNr) end

---Returns a table encoding the document structure in a Lua table. Filename strings are empty if unset, pdfBackgroundPageNo is 0 if no pdfBackground on that page
---@return {pages:{pageWidth:number, pageHeight:number, isAnnotated:boolean, pageTypeFormat:string, pageTypeConfig:string, backgroundColor:integer, pdfBackgroundPageNo:integer, layers:{isVisible:boolean, isAnnotated:boolean}[], currentLayer:integer}[], currentPage:integer, pdfBackgroundFilename:string, xoppFilename:string}
function app.getDocumentStructure() end

---Scrolls to the page specified relatively or absolutely (by default)
---@param page integer (automatically clamped)
---@param relative boolean
function app.scrollToPage(page, relative) end

---Scrolls to the position on the selected page specified relatively (by default) or absolutely
---@param x number
---@param y number
---@param relative boolean
function app.scrollToPos(x,y, relative) end

---Sets the current page as indicated (without scrolling)
---@param pageNr integer (automatically clamped)
function app.setCurrentPage(pageNr) end

---Sets the width and height of the current page in pt = 1/72 inch either relatively or absolutely (by default)
---@param width number
---@param height number
---@param relative boolean
function app.setPageSize(width, height, relative) end

---Sets the current layer of the current page as indicated and updates visibility if specified (by default it does not)
---@param layerNr integer number of the (non-background) layer
---@param change_visibility boolean true -> makes all layers up to the selected one visible, all others invisible
function app.setCurrentLayer(layerNr, change_visibility) end

---Sets the visibility of the current layer
---@param visible boolean
function app.setLayerVisibility(visible) end

---Sets currently selected layer's name
---@param name string
function app.setCurrentLayerName(name) end

---Sets background name
---@param name string
function app.setBackgroundName(name) end

---Scales all text elements of the current layer by the given scale factor
---@param factor number
function app.scaleTextElements(factor) end

---Gets the display DPI
function app.getDisplayDpi() end

---Exports the current document as a pdf or as a svg or png image.
--- range might be something like "2-5; 7" or "3-"
---progressiveMode means that each layer gets an own page in the pdf
---@param opts {outputFile:string, range:string, background:string, progressiveMode: boolean}
function app.export(opts) end

---Given a table of sets of points, draws a batch of strokes on the canvas
---@param opts {strokes:{x:number[], y:number[], pressure:number[], tool:string, width:number, color:integer, fill:number, linestyle:string}[], allowUndoRedoAction:string}
function app.addStrokes(opts) end

---Given a table containing a series of splines, draws a batch of strokes on the canvas
--- coordinates always needs to contain eight coordinates for one point of the spline including handles (startX, startY, ctrl1X, ctrl1Y, ctrl2X, ctrl2Y, endX, endY)
---@param opts {splines:{coordinates:number[], tool:string, width:number, color:integer, fill:number, linestyle:string}[], allowUndoRedoAction:string}
function app.addSplines(opts) end

---Adds images from the provided paths or the provided image data on the current page on the current layer.
---@param opts {images:{path:string, data:string, x:number, y:number, maxHeight:number, maxWidth:number, aspectRatio:boolean}[], allowUndoRedoAction:string}
function app.addImages(opts) end

---Adds textboxes as specified to the current layer
---@param opts {texts:{text:string, font:{name:string, size:number}, color:integer, x:number, y:number}[], allowUndoRedoAction:string}
function app.addTexts(opts) end

---"Open File" dialog
---@param types string[] array of the different allowed extensions e.g. {'\*.bmp', '\*.png'}
---@returns string path of the selected location
function app.getFilePath(types) end

---Notifies program of any updates to the working document caused by the API
function app.refreshPage() end

---Puts a Lua Table of the Strokes (from the selection tool / selected layer) onto the stack.
---Is inverse to app.addStrokes
---@param type string "selection" or "layer"
---@return {x:number[], y:number[], pressure:number[], tool:string, width:number, color:integer, fill:number, linestyle:string}[] stokes
function app.getStrokes(type) end

---Puts a Lua Table of the Images (from the selection tool / selected layer) onto the stack.
---Is inverse to app.addImages
---@param type string "selection" or "layer"
---@return {path:string, data:string, x:number, y:number, maxHeight:number, maxWidth:number, aspectRatio:boolean}[] images
function app.getImages(type) end

---Returns a list of lua table of the texts (from current selection / current layer).
---Is mostly inverse to app.addTexts
---@param type string "selection" or "layer"
---@return {text:string, font:{name:string, size:number}, color:integer, x:number, y:number}[] texts
function app.getTexts(type) end

---Open a file and by default asks the user what to do with the old document.
---@param path string
---@param pageNr integer scroll to this page (defaults to 1)
---@param oldDocument boolean don't ask weather to save the old document?
---@return boolean success
function app.openFile(path, pageNr, oldDocument) end
