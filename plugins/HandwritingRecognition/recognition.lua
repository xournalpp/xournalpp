-- Register all Toolbar actions and intialize all UI stuff
function initUi()
  app.registerUi({["menu"] = "Recognize Handwritten Text", ["callback"] = "recognize"});
  package.path = package.path .. ";../?.lua;/usr/local/share/lua/5.3/?.lua" -- must currently be customized by the user
  sourcePath = debug.getinfo(1).source:match("@?(.*/)")
end

local sel = "Layer"

function recognize()
  -- Initialize UI
  local hasLgi, lgi = pcall(require, "lgi")
  if not hasLgi then
    app.msgbox("You need to have the Lua lgi-module installed and included in your Lua package path in order to use the GUI for migrating font sizes. \n\n", {[1]="OK"})
    return
  end

  --lgi module has been found
  local Gtk = lgi.require("Gtk", "3.0")
  local Gdk = lgi.Gdk
  local assert = lgi.assert
  local builder = Gtk.Builder()
  assert(builder:add_from_file(sourcePath .. "Dialog.glade"))
  local ui = builder.objects
  local dialog1 = ui.dialog1
  local dialog2 = ui.dialog2
  local nb = ui.nb

  -- Connect actions
  function ui.btOk.on_clicked()
    local lang = tostring(ui.languageEntries.active_id)
    local choices = tonumber(ui.choiceEntries.active_id)
    local strokes = app.getStrokes(sel)
    ink = {}
    for i = 1, #strokes do 
      ink[i] = { strokes[i]["x"], strokes[i]["y"] }
    end
    
    local selectionBox = app.getToolInfo("selection")["boundingBox"]
    local width, height = selectionBox["width"], selectionBox["height"]
    require("fetch")
    alternatives = sendRequest(ink, lang, width, height)

    dialog1:hide()
    -- Add notebook tabs programmatically
    numTabs = math.min(choices, #alternatives)
    for i = 1, numTabs do
      local tabLabel = "Alternative " .. tostring(i-1)
      if (i==1) then
        tabLabel = "Recognized Text"
      end
      nb:append_page(Gtk.ScrolledWindow {
        Gtk.TextView {
          id = "page" .. tostring(i),
          buffer = Gtk.TextBuffer { 
            text = alternatives[i] 
          },
          wrap_mode = 'WORD'
       } 
      }, 
      Gtk.Label {label = tabLabel }
    )      
    end

    dialog2:show_all()
  end

  function ui.rd_1.on_toggled()
    sel = "layer"
  end

  function ui.rd_2.on_toggled()
    sel = "selection"
  end

  function ui.btBack.on_clicked()
    dialog2:hide()
    for i = 1, numTabs do
      nb:remove_page(0)
    end
    dialog2:resize(100,100)
    dialog1:show_all()
  end

  function ui.btDone.on_clicked()
    dialog1:destroy()
    dialog2:destroy()
  end

  function get_text_view()
    local currentPage = math.floor(nb:get_current_page()+0.5) -- rounding to an int
    return nb.child[currentPage+1].child[1]
  end

  function ui.btClipboard.on_clicked()
    local textView = get_text_view()
    local currentText = textView.buffer.text; 
    local clipboard = textView:get_clipboard(Gdk.SELECTION_CLIPBOARD)
    clipboard:set_text(currentText,-1)
  end

  function ui.btSave.on_clicked()
    local textView = get_text_view()
    local currentText = textView.buffer.text; 

    local pathseparator = package.config:sub(1,1);
    local path = ui.pickFolder:get_filename()
    local name = ui.txtFilename.text
    local empty = (not name or name=="")
    local overwrite = ui.checkOverwrite:get_active()
    if (path and not empty) then
      local filename = path .. pathseparator .. name
      if (not overwrite and io.open(filename, "r")) then
        app.msgbox("File already exists!", {[1] = "OK"})
      else
        file = assert(io.open(filename, "w"))
        file:write(currentText)
        file:close()
      end
    elseif (not path and empty) then
      app.msgbox("No folder and no name specified!", {[1] = "OK"})
    elseif (not path and not empty) then
      app.msgbox("No folder specified!", {[1] = "OK"})
    elseif (path and empty) then
      app.msgbox("No name specified!", {[1] = "OK"})      
    end
  end

  dialog1:show_all()

end