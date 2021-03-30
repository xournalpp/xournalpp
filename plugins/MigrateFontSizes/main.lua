-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Migrate font sizes with factor displayDPI / 72", ["callback"] = "migrate"});
  app.registerUi({["menu"] = "Show font size migration dialog", ["callback"] = "showDialog"});

  sourcePath = debug.getinfo(1).source:match("@?(.*/)")
end

function migrate()
  local displayDpi = app.getDisplayDpi()
  local dpiNormalizationFactor = 72
  local factor = displayDpi / dpiNormalizationFactor
  -- print("Display DPI is " .. displayDpi .. " => scaling by factor " .. displayDpi .. "/72 = " .. factor)
  local result = app.msgbox("Display DPI is " .. displayDpi .. ". By proceeding the font sizes of all text elements will be scaled by the factor " .. displayDpi .. "/72 = " .. factor, {[1]="Cancel", [2]="OK"})
  if result == 2 then 
    resize(factor)
  end
end

local currDpi

function showDialog()
  local hasLgi, lgi = pcall(require, "lgi")
  if not hasLgi then
    app.msgbox("You need to have the Lua lgi-module installed and included in your Lua package path in order to use the GUI for migrating font sizes. \n\n", {[1]="OK"})
    return
  end

  --lgi module has been found
  local Gtk = lgi.Gtk
  local Gdk = lgi.Gdk
  local assert = lgi.assert
  local builder = Gtk.Builder()
  assert(builder:add_from_file(sourcePath .. "dialog.glade"))
  local ui = builder.objects
  local dialog = ui.dlgMigrateFontSizes

  if not currDpi then 
    currDpi = app.getDisplayDpi()
  end
  ui.spbtOldDpi:set_value(currDpi)
  ui.lblCurrentDpi:set_text(app.getDisplayDpi())
  ui.spbtScaleFactor:set_value(currDpi/72)

-- Connect actions
  function ui.btApply.on_clicked()
    local factor = ui.spbtScaleFactor:get_value()
    resize(factor)
  end

  function ui.btCancel.on_clicked()
    dialog:destroy()
  end

  function ui.spbtScaleFactor.on_value_changed()
    factor = ui.spbtScaleFactor:get_value()
    currDpi = math.floor(factor*72+0.5)
    ui.spbtOldDpi:set_value(currDpi)
  end

  function ui.spbtOldDpi.on_value_changed()
    oldDpi = ui.spbtOldDpi:get_value()
    ui.spbtScaleFactor:set_value(oldDpi/72)
  end

  dialog:show_all()
end

function resize(factor)
  local docStructure = app.getDocumentStructure()
  local numPages = #docStructure["pages"]
  local page = docStructure["currentPage"]
  local layer = docStructure["pages"][page]["currentLayer"]
  
  for i=1, numPages do
    app.setCurrentPage(i)
    local numLayers = #docStructure["pages"][page]["layers"]
    for j=1, numLayers do
      app.setCurrentLayer(j)
      app.scaleTextElements(factor)
    end
  end
  app.setCurrentPage(page)
  app.setCurrentLayer(layer)
end
