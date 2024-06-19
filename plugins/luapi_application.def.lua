---@meta
app = {}
--- Renames file 'from' to file 'to' in the file system.
--- Overwrites 'to' if it already exists.
--- 
--- @param from string
--- @param to string
--- @return number|nil Returns 1 on success, and (nil, message) on failure.
--- @return string
--- 
--- Example:
---   assert(app.glib_rename("path/to/foo", "other/bar"))
--- 
--- Preferred to os.rename() because it works across
--- partitions. Uses glib's rename function.
--- 
--- Returns 1 on success, and (nil, message) on failure.
function app.glib_rename(from, to) end

--- Create a 'Save As' dialog and once the user has chosen the filepath of the location to save
--- calls the specified callback function to which it passes the filepath or the empty string, if the
--- operation was cancelled.
--- 
--- @param cb string name of the callback function(path:string) to call when a file has been chosen
--- @param filename string suggestion for a filename, defaults to "Untitled"
--- 
--- Examples:
---   app.fileDialogSave("cb") -- defaults to suggestion "Untitled" in current working directory
---   app.fileDialogSave("cb", "foo") -- suggests "foo" as filename in current working directory
---   app.fileDialogSave("cb", "/path/to/folder/bar.png") -- suggestes the given absolute path
function app.fileDialogSave(cb, filename) end

--- Create an 'Open File' dialog and when the user has chosen a filepath
--- call a callback function whose sole argument is the filepath.
--- 
--- @param cb string name of the callback function(path:string) to call when a file was chosen
--- @param types string[] array of the different allowed extensions e.g. {'\*.bmp', '\*.png'}
--- 
--- Examples:
---   app.fileDialogOpen("cb", {})
---   app.fileDialogOpen("cb", {'*.bmp', '*.png'})
function app.fileDialogOpen(cb, types) end

--- Open a dialog with arbitrary text and buttons (with callbacks)
--- 
--- @param message string
--- @param options table<integer, string> integer to button texts
--- @param cb string name of the callback function(button:integer) to call on user interaction
--- @param error boolean is this message an error (optional)
--- 
--- Example 1: app.openDialog("Test123", {[1] = "Yes", [2] = "No"}, "cb", false)
---   or       app.openDialog("Test123", {"Yes", "No"}, "cb")
--- Pops up a message box with two buttons "Yes" and "No" and executed function "cb" whose single parameter is the number
--- corresponding to the button the user clicked.
--- 
--- If the optional boolean parameter is true, the dialog is treated as an error message
--- Example 2: app.openDialog("Invalid parameter", {"Ok"}, "", true)
--- 
--- Warning: the callback function is never called if the dialog is closed without pressing one of the custom buttons.
function app.openDialog(message, options, cb, error) end

--- Allow to register menupoints and toolbar buttons. This needs to be called from initUi
--- 
--- @param opts {menu: string, callback: string, toolbarID: string, mode:integer, accelerator:string} options (`mode`,
---  `toolbarID` and `accelerator` are optional)
--- @return {menuId:integer}
--- 
--- Example 1: app.registerUi({["menu"] = "HelloWorld", callback="printMessage", mode=1, accelerator="<Control>a"})
--- registers a menupoint with name "HelloWorld" executing a function named "printMessage", in mode 1,
--- which can be triggered via the "<Control>a" keyboard accelerator
--- 
--- Example 2: app.registerUi({callback ="blueDashedPen", toolbarId="CUSTOM_PEN_1", iconName="bluePenIcon"})
--- registers a toolbar icon named "bluePenIcon" executing a function named "blueDashedPen", which can be added
--- to a toolbar via toolbar customization or by editing the toolbar.ini file using the name "Plugin::CUSTOM_PEN_1"
--- Note that in toolbar.ini the string "Plugin::" must always be prepended to the toolbarId specified in the plugin
--- 
--- The mode and accelerator are optional. When specifying the mode, the callback function should have one parameter
---    that receives the mode. This is useful for callback functions that are shared among multiple menu entries.
function app.registerUi(opts) end

--- Execute an UI action (usually internally called from Toolbar / Menu)
--- The argument consists of a Lua table with 3 keys: "action", "group" and "enabled"
--- The key "group" is currently only used for debugging purpose and can safely be omitted.
--- The key "enabled" is true by default.
--- 
--- @param opts {action: string, enabled:boolean} options (`enabled` is `true` by default)
--- 
--- Example 1: app.uiAction({["action"] = "ACTION_PASTE"})
--- pastes the clipboard content into the document
--- 
--- Example 2: app.uiAction({["action"] = "ACTION_TOOL_DRAW_ELLIPSE", ["enabled"] = false})
--- turns off the Ellipse drawing type
function app.uiAction(opts) end

--- Execute action from sidebar menu
--- 
--- @param action string the desired action
--- 
--- Example: app.sidebarAction("MOVE_DOWN")
--- moves down the current page or layer, depending on which sidebar tab is selected
function app.sidebarAction(action) end

--- Get the index of the currently active sidebar-page.
--- 
--- @return integer pageNr pageNr of the sidebar page
--- 
--- Example: app.getSidebarPageNo() -- returns e.g. 1
function app.getSidebarPageNo() end

--- Set the currently active sidebar-page by its index.
--- 
--- @param pageNr integer pageNr of the sidebar page
--- 
--- Look at src/core/gui/sidebar/Sidebar.cpp to find out which index corresponds to which page (e.g. currently 1 is the
--- page with the TOC/index if available). Note that indexing the sidebar-pages starts at 1 (as usual in lua).
--- 
--- Example: app.setSidebarPageNo(3) -- sets the sidebar-page to preview Layer
function app.setSidebarPageNo(pageNr) end

--- Execute action from layer controller
--- 
--- @param action string the desired action
--- 
--- Example: app.layerAction("ACTION_DELETE_LAYER")
--- deletes the current layer
function app.layerAction(action) end

--- Given a table containing a series of splines, draws a batch of strokes on the canvas.
--- Expects a table of tables containing eight coordinate pairs, along with attributes of the stroke.
--- 
--- @param opts {splines:{coordinates:number[], tool:string, width:number, color:integer, fill:number,
--- linestyle:string}[], allowUndoRedoAction:string}
--- @return lightuserdata[] references to the created strokes
--- 
--- Required Arguments: splines
--- Optional Arguments: pressure, tool, width, color, fill, lineStyle
--- 
--- If optional arguments are not provided, the specified tool settings are used.
--- If the tool is not provided, the current pen settings are used.
--- The only tools supported are Pen and Highlighter.
--- 
--- The function expects 8 points per spline segment. Due to the nature of cubic
--- splines, you must pass your points in a repeating pattern:
--- startX, startY, ctrl1X, ctrl1Y, ctrl2X, ctrl2Y, endX, endY, startX, startY, ...
--- 
--- The function checks that the length of the coordinate table is divisible by eight, and will throw
--- an error if it is not.
--- 
--- Example: local refs = app.addSplines({
---            ["splines"] = { -- The outer table is a table of strokes
---                ["coordinates"] = { -- Each inner table is a coord stream that represents SplineSegments that can be
--- assembled into a stroke
---                  [1] = 880.0, // Every eight coordinates (4 pairs) is a SplineSegment
---                  [2] = 874.0,
---                  [3] = 881.3295,
---                  [4] = 851.5736,
---                  [5] = 877.2915,
---                  [6] = 828.2946,
---                  [7] = 875.1697,
---                  [8] = 806.0,
---                  ... -- A spline can be made up of as many SplineSegments as is necessary.
---                },
---                -- Tool options are also specified per-stroke
---                ["width"] = 1.4,
---                ["color"] = 0xff0000,
---                ["fill"] = 0,
---                ["tool"] = "pen",
---                ["lineStyle"] = "plain"
---            },
---            ["allowUndoRedoAction"] = "grouped", -- Each batch of splines can be grouped into one undo/redo action (or
--- "individual" or "none")
---        })
function app.addSplines(opts) end

--- Given a table of sets of points, draws a batch of strokes on the canvas.
--- Expects three tables of equal length: one for X, one for Y, and one for
--- stroke pressure, along with attributes of the stroke. Each stroke has
--- attributes handled individually.
--- 
--- @param opts {strokes:{X:number[], Y:number[], pressure:number[], tool:string, width:number, color:integer,
--- fill:number, linestyle:string}[], allowUndoRedoAction:string}
--- @return lightuserdata[] references to the created strokes
--- 
--- Required Arguments: X, Y
--- Optional Arguments: pressure, tool, width, color, fill, lineStyle
--- 
--- If optional arguments are not provided, the specified tool settings are used.
--- If the tool is not provided, the current pen settings are used.
--- The only tools supported are Pen and Highlighter.
--- 
--- The function checks for consistency among table lengths, and throws an
--- error if there is a discrepancy
--- 
--- Example:
--- 
--- local refs = app.addStrokes({
---     ["strokes"] = { -- The outer table is a table of strokes
---         {   -- Inside a stroke are three tables of equivalent length that represent a series of points
---             ["x"]        = { [1] = 110.0, [2] = 120.0, [3] = 130.0, ... },
---             ["y"]        = { [1] = 200.0, [2] = 205.0, [3] = 210.0, ... },
---             ["pressure"] = { [1] = 0.8,   [2] = 0.9,   [3] = 1.1, ... },
---             -- Each stroke has individually handled options
---             ["tool"] = "pen",
---             ["width"] = 3.8,
---             ["color"] = 0xa000f0,
---             ["fill"] = 0,
---             ["lineStyle"] = "solid",
---         },
---         {
---             ["x"]        = { [1] = 310.0, [2] = 320.0, [3] = 330.0, ... },
---             ["y"]        = { [1] = 300.0, [2] = 305.0, [3] = 310.0, ... },
---             ["pressure"] = { [1] = 3.0,   [2] = 3.0,   [3] = 3.0, ... },
---             ["tool"] = "pen",
---             ["width"] = 1.21,
---             ["color"] = 0x808000,
---             ["fill"] = 0,
---             ["lineStyle"] = "solid",
---         },
---         {
---             ["x"]        = { [1] = 27.0,  [2] = 28.0,  [3] = 30.0, ... },
---             ["y"]        = { [1] = 100.0, [2] = 102.3, [3] = 102.5, ... },
---             ["pressure"] = { [1] = 1.0,   [2] = 1.0,   [3] = 1.0, ... },
---             ["tool"] = "pen",
---             ["width"] = 1.0,
---             ["color"] = 0x00aaaa,
---             ["fill"] = 0,
---             ["lineStyle"] = "dashdot",
---         },
---     },
---     ["allowUndoRedoAction"] = "grouped", -- Each batch of strokes can be grouped into one undo/redo action (or
--- "individual" or "none")
--- })
function app.addStrokes(opts) end

--- Adds textboxes as specified to the current layer.
--- 
--- Global parameters:
---   - texts table: array of text-parameter-tables
---   - allowUndoRedoAction string: Decides how the change gets introduced into the undoRedo action list "individual",
--- "grouped" or "none"
--- 
--- @param opts {texts:{text:string, font:{name:string, size:number}, color:integer, x:number, y:number}[],
--- allowUndoRedoAction:string}
--- @return lightuserdata[] references to the created text elements
--- 
--- Parameters per textbox:
---   - text string: content of the textbox (required)
---   - font table {name string, size number} (default: currently configured font/size from the settings)
---   - color integer: RGB hex code for the text-color (default: color of text tool)
---   - x number: x-position of the box (upper left corner) (required)
---   - y number: y-position of the box (upper left corner) (required)
--- 
--- Example:
--- 
--- local refs = app.addTexts{texts={
---   {
---     text="Hello World",
---     font={name="Noto Sans Mono Medium", size=8.0},
---     color=0x1259b9,
---     x = 50.0,
---     y = 50.0,
---   },
---   {
---     text="Testing",
---     font={name="Noto Sans Mono Medium", size=8.0},
---     color=0x0,
---     x = 150.0,
---     y = 50.0,
---   },
--- }
function app.addTexts(opts) end

--- Returns a list of lua table of the texts (from current selection / current layer).
--- Is mostly inverse to app.addTexts (except getTexts will also retrieve the width/height of the textbox)
--- 
--- @param type string "selection" or "layer"
--- @return {text:string, font:{name:string, size:number}, color:integer, x:number, y:number, width:number,
--- height:number, ref:lightuserdata}[] texts
--- 
--- Required argument: type ("selection" or "layer")
--- 
--- Example: local texts = app.getTexts("layer")
--- 
--- possible return value:
--- {
---   {
---     text = "Hello World",
---     font = {
---             name = "Noto Sans Mono Medium",
---             size = 8.0,
---            },
---     color = 0x1259b9,
---     x = 127.0,
---     y = 70.0,
---     width = 55.0,
---     height = 23.0,
---     ref = userdata: 0x5f644c0700d0
---   },
---   {
---     text = "Testing",
---     font = {
---             name = "Noto Sans Mono Medium",
---             size = 8.0,
---            },
---     color = 0x0,
---     x = 150.0,,
---     y = 70.0,
---     width = 55.0,
---     height = 23.0,
---     ref = userdata: 0x5f644c0701e8
---   },
--- }
--- 
function app.getTexts(type) end

--- Puts a Lua Table of the Strokes (from the selection tool / selected layer) onto the stack.
--- Is inverse to app.addStrokes
--- 
--- @param type string "selection" or "layer"
--- @return {x:number[], y:number[], pressure:number[], tool:string, width:number, color:integer, fill:number,
--- linestyle:string, ref:lightuserdata}[] strokes
--- 
--- Required argument: type ("selection" or "layer")
--- 
--- Example: local strokes = app.getStrokes("selection")
--- 
--- possible return value:
--- {
---         {   -- Inside a stroke are three tables of equivalent length that represent a series of points
---             ["x"]        = { [1] = 110.0, [2] = 120.0, [3] = 130.0, ... },
---             ["y"]        = { [1] = 200.0, [2] = 205.0, [3] = 210.0, ... },
---             -- pressure is only present if pressure is set -> pressure member might be nil
---             ["pressure"] = { [1] = 0.8,   [2] = 0.9,   [3] = 1.1, ... },
---             -- Each stroke has individually handled options
---             ["tool"] = "pen",
---             ["width"] = 3.8,
---             ["color"] = 0xa000f0,
---             ["fill"] = 0,
---             ["lineStyle"] = "plain",
---             ["ref"] = userdata: 0x5f644c02c538
---         },
---         {
---             ["x"]         = {207, 207.5, 315.2, 315.29, 207.5844},
---             ["y"]         = {108, 167.4, 167.4, 108.70, 108.7094},
---             ["tool"]      = "pen",
---             ["width"]     = 0.85,
---             ["color"]     = 16744448,
---             ["fill"]      = -1,
---             ["lineStyle"] = "plain",
---             ["ref"] = userdata: 0x5f644c02d440
---         },
---         {
---             ["x"]         = {387.60, 387.6042, 500.879, 500.87, 387.604},
---             ["y"]         = {153.14, 215.8661, 215.866, 153.14, 153.148},
---             ["tool"]      = "pen",
---             ["width"]     = 0.85,
---             ["color"]     = 16744448,
---             ["fill"]      = -1,
---             ["lineStyle"] = "plain",
---             ["ref"] = userdata: 0x5f644c0700d0
---         },
--- }
function app.getStrokes(type) end

--- Notifies program of any updates to the working document caused
--- by the API.
--- 
--- Example: app.refreshPage()
function app.refreshPage() end

--- Change page background of current page
--- 
--- @param background string backgroundType (e.g. "graph")
--- 
--- Example: app.changeCurrentPageBackground("graph")
--- changes the page background of the current page to graph paper
function app.changeCurrentPageBackground(background) end

--- Query all colors of the current palette.
--- 
--- @return {color: integer, name: string}[] options
--- 
--- Example: palette = app.getColorPalette()
--- possible return value:
--- {
---     [1] = {color = 0xffffff, name = "white" },
---     [2] = {color = 0x000000, name = "black" },
---     [3] = {color = 0xdc8a78, name = "Rosewater" },
--- }
--- 
function app.getColorPalette() end

--- Change color of a specified tool or of the current tool
--- 
--- @param opts {color: integer, tool: string, selection: boolean} options (`selection=true` -> new color applies to
--- active selection as well, `tool=nil` (unset) -> currently active tool is being used)
--- 
--- Example 1: app.changeToolColor({["color"] = 0xff00ff, ["tool"] = "PEN"})
--- changes the color of the pen tool to violet without applying this change to the current selection
--- 
--- Example 2: app.changeToolColor({["color"] = 0xff0000, ["selection"] = true })
--- changes the color of the current tool to red and also applies it to the current selection if there is one
function app.changeToolColor(opts) end

--- Select Background Pdf Page for Current Page
--- First argument is an integer (page number) and the second argument is a boolean (isRelative)
--- specifying whether the page number is relative to the current pdf page or absolute
--- 
--- @param pageNr integer page-nunmber
--- @param relative boolean should `pageNr` be interpreted relative?
--- 
--- Example 1: app.changeBackgroundPdfPageNr(1, true)
--- changes the pdf page to the next one (relative mode)
--- 
--- Example 2: app.changeBackgroundPdfPageNr(7, false)
--- changes the page background to the 7th pdf page (absolute mode)
function app.changeBackgroundPdfPageNr(pageNr, relative) end

--- Returns a table encoding all info on the chosen tool (active, pen, highlighter, eraser or text) in a Lua table.
--- 
--- @param tool string tool to get the information for
--- @return {} info varies a lot depending on the tool
--- 
--- The Lua table will be of one of the following shapes
--- 
--- for pen:
--- {
---   "size" = string
---   "color" = integer
---   "filled" = bool
---   "fillOpacity" = integer (0 to 255)
---   "drawingType" = string
---   "lineStyle" = string
--- }
--- 
--- See /src/control/ToolEnums.cpp for possible values of "size".
--- 
--- for text:
--- {
---   "font" = {
---     name = string.
---     size = number
---   }
---   "color" = integer
--- }
--- 
--- for active tool:
--- {
---   "type" = string
---   "size" = {
---     name = string.
---     value = number
---   }
---   "color" = integer
---   "fillOpacity" = integer (0 to 255)
---   "drawingType" = string
---   "lineStyle" = string
---   "thickness" = number
--- }
--- 
--- See /src/control/ToolEnums.cpp for possible values of "type", "size", "drawingType" and "lineStyle".
--- 
--- for eraser:
--- {
---   "type" = string
---   "size" = string
--- }
--- 
--- See /src/control/ToolEnums.cpp for possible values of "type" and "size".
--- 
--- for highlighter:
--- {
---   "size" = string
---   "color" = integer
---   "filled" = bool
---   "fillOpacity" = integer (0 to 255)
---   "drawingType" = string
--- }
--- 
--- See /src/control/ToolEnums.cpp for possible values of "size".
--- 
--- for seiection:
--- {
---   -- bounding box as drawn in the UI (includes padding on all sides)
---   "boundingBox" = {
---      "width"  = number
---      "height" = number
---      "x"      = number
---      "y"      = number
---   }
---   -- same as "boundingBox" but the state before any transformation was applied
---   "originalBounds" = {
---      "width"  = number
---      "height" = number
---      "x"      = number
---      "y"      = number
---   }
---   -- bounds used for snapping (doesn't include padding and doesn't account to line width)
---   -- for more information see https://github.com/xournalpp/xournalpp/pull/4359#issuecomment-1304395011
---   "snappedBounds" = {
---      "width"  = number
---      "height" = number
---      "x"      = number
---      "y"      = number
---   }
---   "rotation" = number
---   "isRotationSupported" = bool
--- }
--- 
--- Example 1: local penInfo = app.getToolInfo("pen")
---            local size = penInfo["size"]
---            local opacity = penInfo["fillOpacity"]
--- *
--- Example 2: local font = app.getToolInfo("text")["font"]
---            local fontname = font["name"]
---            local fontsize = font["size"]
--- 
--- Example 3: local color = app.getToolInfo("text")["color"]
---            local red = color >> 16 & 0xff
---            local green = color >> 8 & 0xff
---            local blue = color & 0xff
--- 
--- Example 4: local activeToolInfo = app.getToolInfo("active")
---            local thickness = activeToolInfo["thickness"]
---            local drawingType = activeToolInfo["drawingType"]
--- 
--- Example 5: local eraserInfo = app.getToolInfo("eraser")
---            local type = eraserInfo["type"]
---            local size = eraserInfo["size"]
---            local sizeName = size["name"]
---            local thickness = size["value"]
--- 
--- Example 6: local highlighterInfo = app.getToolInfo("highlighter")
---            local sizeName = highlighterInfo["size"]["name"]
---            local opacity = highlighterInfo["fillOpacity"]
--- 
--- Example 7: local selectionInfo = app.getToolInfo("selection")
---            local rotation = selectionInfo["rotation"]
---            local boundingX = selectionInfo["boundingBox"]["x"]
---            local snappedBoundsWidth = selectionInfo["snappedBounds"]["width"]
function app.getToolInfo(tool) end

--- Returns a table encoding the document structure in a Lua table.
--- 
--- @return {pages:{pageWidth:number, pageHeight:number, isAnnotated:boolean, pageTypeFormat:string,
--- pageTypeConfig:string, backgroundColor:integer, pdfBackgroundPageNo:integer, layers:{isVisible:boolean,
--- isAnnotated:boolean}[], currentLayer:integer}[], currentPage:integer, pdfBackgroundFilename:string,
--- xoppFilename:string}
--- 
--- The shape of the returned Lua table will be:
--- {
---   "pages" = {
---     {
---       "pageWidth" = number,
---       "pageHeight" = number,
---       "isAnnotated" = bool,
---       "pageTypeFormat" = string,
---       "pageTypeConfig" = string,
---       "backgroundColor" = integer,
---       "pdfBackgroundPageNo" = integer (0, if there is no pdf background page),
---       "layers" = {
---         [0] = {
---           "isVisible" = bool
---         },
---         [1] = {
---           "isVisible" = bool,
---           "isAnnotated" = bool
---         },
---         ...
---       },
---       "currentLayer" = integer
---     },
---     ...
---   }
---   "currentPage" = integer,
---   "pdfBackgroundFilename" = string (empty if there is none)
---   "xoppFilename" = string (empty if there is none)
--- }
--- 
--- Example: local docStructure = app.getDocumentStructure()
function app.getDocumentStructure() end

--- Scrolls to the page specified relatively or absolutely (by default)
--- The page number is clamped to the range between the first and last page
--- 
--- @param page integer (automatically clamped)
--- @param relative boolean
--- 
--- Example 1: app.scrollToPage(1, true)
--- scrolls to the next page (relative mode)
--- 
--- Example 2: app.scrollToPage(10)
--- scrolls to page 10 (absolute mode)
function app.scrollToPage(page, relative) end

--- Scrolls to the position on the selected page specified relatively (by default) or absolutely
--- 
--- @param x number
--- @param y number
--- @param relative boolean
--- 
--- Example 1: app.scrollToPos(20,10)
--- scrolls 20pt right and 10pt down (relative mode)
--- 
--- Example 2: app.scrollToPos(200, 50, false)
--- scrolls to page position 200pt right and 50pt down from the left page corner  (absolute mode)
function app.scrollToPos(x, y, relative) end

--- Obtains the label of the specified page in the pdf background.
--- 
--- @param page integer
--- @return string|nil Returns the label on success and (nil, message) if page number is out of range.
--- @return string
--- 
--- Example 1: local label = app.getPageLabel(10)
---  ' obtains the page label of page 10 in the background pdf
function app.getPageLabel(page) end

--- Sets the current page as indicated (without scrolling)
--- The page number passed is clamped to the range between first page and last page
--- 
--- @param pageNr integer (automatically clamped)
--- 
--- Example: app.setCurrentPage(1)
--- makes the first page the new current page
function app.setCurrentPage(pageNr) end

--- Sets the width and height of the current page in pt = 1/72 inch either relatively or absolutely (by default)
--- 
--- @param width number
--- @param height number
--- @param relative boolean
--- 
--- Example 1: app.setPageSize(595.275591, 841.889764)
--- makes the current page have standard (A4 paper) width and height (absolute mode)
--- 
--- Example 2: app.setPageSize(0, 14.17*6, true)
--- adds 14.17*6 pt = 3cm to the height of the page (relative mode)
function app.setPageSize(width, height, relative) end

--- Sets the current layer of the current page as indicated and updates visibility if specified (by default it does not)
--- Displays an error message, if the selected layer does not exist
--- 
--- @param layerNr integer number of the (non-background) layer
--- @param change_visibility boolean true -> makes all layers up to the selected one visible, all others invisible
--- 
--- Example 1: app.setCurrentLayer(2, true)
--- makes the second (non-background) layer the current layer and makes layers 1, 2 and the background layer visible, the
--- others hidden
--- 
--- Example 2: app.setCurrentLayer(2, false)
--- makes the second (non-background) layer the current layer and does not change visibility
function app.setCurrentLayer(layerNr, change_visibility) end

--- Sets the visibility of the current layer
--- 
--- @param visible boolean
--- 
--- Example: app.setLayerVisibility(true)
--- makes the current layer visible
function app.setLayerVisibility(visible) end

--- Sets currently selected layer's name.
--- 
--- @param name string
--- 
--- Example: app.setCurrentLayerName("Custom name 1")
--- Changes current layer name to "Custom name 1"
function app.setCurrentLayerName(name) end

--- Sets background name.
--- 
--- @param name string
--- 
--- Example: app.setBackgroundName("Custom name 1")
--- Changes background name to "Custom name 1"
function app.setBackgroundName(name) end

--- Scales all text elements of the current layer by the given scale factor.
--- This means the font sizes get scaled, wheras the position of the left upper corner
--- of the bounding box remains unchanged
--- 
--- @param factor number
--- 
--- Example: app.scaleTextElements(2.3)
--- scales all text elements on the current layer with factor 2.3
function app.scaleTextElements(factor) end

--- Gets the display DPI.
--- 
--- @return integer displayDpi dpi of the display
--- 
--- Example: app.getDisplayDpi()
function app.getDisplayDpi() end

--- Gets the current zoom.
--- 
--- @return double current zoom level
--- 
--- Example: app.getZoom()
function app.getZoom() end

--- Sets the current zoom.
--- 
--- @param zoom number
--- 
--- Example: app.setZoom(2.5)
--- Changes zoom level to 2.5
function app.setZoom(zoom) end

--- Exports the current document as a pdf or as a svg or png image
--- 
--- @param opts {outputFile:string, range:string, background:string, progressiveMode: boolean}
--- 
--- Example 1:
--- app.export({["outputFile"] = "Test.pdf", ["range"] = "2-5; 7", ["background"] = "none", ["progressiveMode"] = true})
--- uses progressiveMode, so for each page of the document, instead of rendering one PDF page, the page layers are
--- rendered one by one to produce as many pages as there are layers.
--- 
--- Example 2:
--- app.export({["outputFile"] = "Test.svg", ["range"] = "3-", ["background"] = "unruled"})
--- 
--- Example 3:
--- app.export({["outputFile"] = "Test.png", ["layerRange"] = "1-2", ["background"] = "all", ["pngWidth"] = 800})
function app.export(opts) end

--- Opens a file and by default asks the user what to do with the old document.
--- Returns true when successful, false otherwise.
--- 
--- @param path string
--- @param pageNr integer scroll to this page (defaults to 1)
--- @param oldDocument boolean don't ask weather to save the old document? (defaults to false)
--- @return boolean success
--- 
--- Example 1: local success = app.openFile("home/username/bg.pdf")
---            asks what to do with the old document and loads the file afterwards, scrolls to top
--- 
--- Example 2: local sucess = app.openFile("home/username/paintings.xopp", 3, true)
---            opens the file, scrolls to the 3rd page and does not ask to save the old document
function app.openFile(path, pageNr, oldDocument) end

--- Adds images from the provided paths or the provided image data on the current page on the current layer.
--- 
--- @param opts {images:{path:string, data:string, x:number, y:number, maxHeight:number, maxWidth:number,
--- aspectRatio:boolean}[], allowUndoRedoAction:string}
--- @return lightuserdata|string[]
--- 
--- Global parameters:
---  - images table: array of image-parameter-tables
---  - allowUndoRedoAction string: Decides how the change gets introduced into the undoRedo action list "individual",
---    "grouped" or "none"
--- 
--- Parameters per image:
---  - path string: filepath to the image
---    or
--- - data string: (file) content of the image (exactly one of path and data should be specified)
---  - x number: x-Coordinate where to place the left upper corner of the image (default: 0)
---  - y number: y-Coordinate where to place the left upper corner of the image (default: 0)
---  scaling options:
---  - maxWidth integer: sets the width of the image in pixels. If that is too large for the page the image gets scaled
---    down keeping the aspect-ratio provided by maxWidth/maxHeight (default: -1)
---  - maxHeight integer: sets the height of the image in pixels. If that is too large for the page the image gets scaled
---    down keeping the aspect-ratio provided by maxWidth/maxHeight (default: -1)
---  - aspectRatio boolean: should the image be scaled in the other dimension to preserve the aspect ratio? Is only set
---    to false if the parameter is false, nil leads to the default value of true (default: true)
---  - scale number: factor to apply to the both dimensions in the end after setting width/height (with/without
---    keeping aspect ratio) (default: 1)
--- 
--- Note about scaling:
--- If the maxHeight-, the maxWidth- as well as the aspectRatio-parameter are given, the image will fit into the
--- rectangle specified with maxHeight/maxWidth. To preserve the aspect ratio, one dimension will be smaller than
--- specified. Still, the scale parameter is applied after this width/height scaling and if after that the dimensions are
--- too large for the page, the image still gets scaled down afterwards.
--- 
--- Returns a table with as many values as images were passed. A value of type lightuserdata represents the reference to
--- the image, while on error the value corresponding to the image will be a string with the error message. If the
--- parameters don't fit at all, a real lua error might be thrown immediately.
--- 
--- Example 1: local refs = app.addImages{images={{path="/media/data/myImg.png", x=10, y=20, scale=2},
---                                            {path="/media/data/myImg2.png", maxHeight=300, aspectRatio=true}},
---                                    allowUndoRedoAction="grouped",
---                                            }
--- Example 2: local refs = app.addImages{images={{data="<binary image data>", x=100, y=200, maxHeight=300,
--- maxWidth=400}}}
function app.addImages(opts) end

--- Puts a Lua Table of the Images (from the selection tool / selected layer) onto the stack.
--- Is inverse to app.addImages
--- 
--- @param type string "selection" or "layer"
--- @return {x:number, y:number, width:number, height:number, data:string, format:string, imageWidth:number,
--- imageHeight:number, ref:lightuserdata}[] images
--- 
--- Required argument: type ("selection" or "layer")
--- 
--- Example: local images = app.getImages("selection")
--- 
--- return value:
--- {
---     {
---         ["x"] = number,
---         ["y"] = number,
---         ["width"] = number,    (width when inserted into Xournal++)
---         ["height"] = number,   (height when inserted into Xournal++)
---         ["data"] = string,
---         ["format"] = string,
---         ["imageWidth"] = integer,
---         ["imageHeight"] = integer,
---         ["ref"] = userdata: 0x5f644c0700d0
---     },
---     {
---         ...
---     },
---     ...
--- }
function app.getImages(type) end

--- Get the plugin's folder for writing one of
--- - config files
--- - data files
--- - state files
--- and forces it to exist.
--- 
--- @param type string "config", "data" or "state"
--- @return string file path
--- 
--- Example 1: local configFolder = app.getFolder("config")
--- 
--- Example 2: local stateFolder = app.getFolder("state")
--- 
function app.getFolder(type) end

--- Clears a selection by releasing its elements to the current layer.
--- 
--- Example: app.clearSelection()
--- 
function app.clearSelection() end

--- Adds those elements from the current layer to the current selection,
--- whose addresses are in refs. If there is no selection create one.
--- 
--- @param refs lightuserdata[] references to elements from the current layer
--- 
--- Required argument: refs
--- 
--- Example: local refs = app.addStrokes( <some stroke data> )
---          app.addToSelection(refs)
--- 
function app.addToSelection(refs) end

