function initUi()
  app.registerUi({["menu"] = "Plot function...", ["callback"] = "showDialog"});
  
  sourcePath = debug.getinfo(1).source:match("@?(.*/)")
end

function showDialog()
  local hasLuaxp, luaxp = pcall(require, "luaxp")
  if not hasLuaxp then
    app.msgbox("You should have a copy of the luaxp module inside the plugin folder")
    return
  end

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
  assert(builder:add_from_file(sourcePath .. "plotter.glade"))
  local ui = builder.objects
  local dialog = ui.dlgMain

-- Connect actions
  function ui.btnAddGraph.on_clicked()
    local tMin = ui.spbtTMin:get_value()
    local tMax = ui.spbtTMax:get_value()

    wc = { -- coordinate system window
      xMin = ui.spbtXMin:get_value(), 
      xMax = ui.spbtXMax:get_value(),
      xUnit = ui.spbtXUnit:get_value(),
      yMin = ui.spbtYMin:get_value(),
      yMax = ui.spbtYMax:get_value(),
      yUnit = ui.spbtYUnit:get_value(),
    }

    local unit = ui.comboentryUnit:get_text()
    local factor = getFactor(unit)

    wxpp = { -- xournalpp window
      width = ui.spbtWidth:get_value()*factor,
      height = ui.spbtHeight:get_value()*factor,
      cx = 300, 
      cy = 200
    }

    local xcoord, ycoord, strokes = {}, {}, {}
    local samples = ui.spbtSamples:get_value()

    -- draw graph
    for i = 0, samples-1 do
      local t = tMin + i/(samples-1)*(tMax-tMin)
      local xt = eval("x", t) 
      local yt = eval("y", t)
      if (xt~=nil and xt>=wc.xMin and xt<=wc.xMax and yt~=nil and yt>=wc.yMin and yt<=wc.yMax) then
        table.insert(xcoord, fitX(xt))
        table.insert(ycoord, fitY(yt))
      else 
        appendStroke(strokes, xcoord, ycoord)
        xcoord = {}; ycoord = {}
      end
    end
    appendStroke(strokes, xcoord, ycoord)

    local colAxis, alphaAxis = packRgba(ui.colAxis:get_rgba())
    local colGrid, alphaGrid = packRgba(ui.colGrid:get_rgba())
    local axisStyle = ui.comboentryAxis:get_text()
    local gridStyle = ui.comboentryGrid:get_text()
    local lwAxis = ui.spbtLineWidthAxis:get_value()
    local lwGrid = ui.spbtLineWidthGrid:get_value()
    
    local arrowLength = 8
    local mx, my = fit(wc.xMin, wc.yMin)
    local Mx, My = fit(wc.xMax, wc.yMax)
    local x0, y0 = fit(0, 0)

    local autofit = ui.cbAutoFit:get_active()
    local drawAxisLines = ui.cbAxisLines:get_active()
    local drawTicks = ui.cbTicks:get_active()
    local drawGrid = ui.cbGrid:get_active()

    if drawAxisLines then
      -- draw x-axis and y-axis
      local xAxis = {x = {mx, Mx}, y = {y0, y0}, lineStyle = axisStyle, color=colAxis, width=lwAxis}
      local yAxis = {x = {x0, x0}, y = {my, My}, lineStyle = axisStyle, color=colAxis, width=lwAxis}
      local xHead = {x = {Mx-arrowLength, Mx, Mx-arrowLength}, y = {y0-.6*arrowLength, y0, y0+.6*arrowLength}, color=colAxis, width=lwAxis, fill=-1}
      local yHead = {x = {x0-.6*arrowLength, x0, x0+.6*arrowLength}, y = {My+arrowLength, My, My+arrowLength}, color=colAxis, width=lwAxis, fill=-1}
      table.insert(strokes, xAxis)
      table.insert(strokes, yAxis)
      table.insert(strokes, xHead)
      table.insert(strokes, yHead)
    end

    if drawGrid then
      -- draw vertical grid lines
      for i = (wc.xMin // wc.xUnit) + 1, (wc.xMax // wc.xUnit) do
        local xval = fitX(i*wc.xUnit)
        table.insert(strokes, {x = {xval, xval}, y = {my, My}, width = lwGrid, lineStyle = gridStyle, color = colGrid})
      end
      -- draw horizontal grid lines
      for i = (wc.yMin // wc.yUnit) + 1, (wc.yMax // wc.yUnit) do
        local yval = fitY(i*wc.yUnit)
        table.insert(strokes, {x = {mx, Mx}, y = {yval, yval}, width = lwGrid, lineStyle = gridStyle, color = colGrid}) 
      end
    end

    if drawTicks then
      local tickHeight = 4.0
      -- draw ticks on x-axis
      for i = (wc.xMin // wc.xUnit) + 1, (wc.xMax // wc.xUnit) do
        local xval = fitX(i*wc.xUnit)
        if xval >= Mx - 2*arrowLength then break end
        table.insert(strokes, {x = {xval, xval}, y = {y0+tickHeight/2, y0-tickHeight/2}, width = lwAxis, color = colAxis})
      end
      -- draw ticks on y-axis
      for i = (wc.yMin // wc.yUnit) + 1, (wc.yMax // wc.yUnit) do
        local yval = fitY(i*wc.yUnit)
        if yval <= My + 2*arrowLength then break end
        table.insert(strokes, {x = {x0-tickHeight/2, x0+tickHeight/2}, y = {yval, yval}, width = lwAxis, color = colAxis}) 
      end
    end

    app.addStrokes({strokes = strokes})
    app.refreshPage()
  end

  function appendStroke(strokes, xcoord, ycoord)
    if #xcoord > 0 then
      table.insert(strokes, {x=xcoord, y=ycoord})
    end
  end

  function eval(f, t)
    if f~="x" and f~="y" then
      print("invalid function argument")
      return
    end
    local fstr = f=="x" and ui.entryTermX:get_text() or ui.entryTermY:get_text()

    local parsedExp, cerr = luaxp.compile(fstr)
    if parsedExp == nil then
      print("Parsing failed: " .. cerr.message) 
    end
    local context = {t = t, pi = math.pi}
    local resultValue, rerr = luaxp.run(parsedExp, context)
    if resultValue == nil then
      print("Evaluation failed: " .. rerr.message)
    else
      if luaxp.isNull(resultValue) then
        return nil
      else
        return resultValue
      end
    end
  end

  function fitX(t)
    return wxpp.width/(wc.xMax-wc.xMin)*(t-wc.xMin) + wxpp.cx-wxpp.width/2
  end

  function fitY(t)
    return -wxpp.height/(wc.yMax-wc.yMin)*(t-wc.yMin) + wxpp.cy+wxpp.height/2
  end

  function fit(x,y)
    return fitX(x), fitY(y)
  end

  function ui.btnDiscard.on_clicked()
    dialog:destroy()
  end

  function getFactor(unit)
    if unit == "inch" then
      return 72
    elseif unit == "cm" then
      return 28.34
    end
    return 1
  end

  function packRgba(rgba)
    local math = require("math")
    return math.floor(256^2*255*rgba.red + 256*255*rgba.green+255*rgba.blue), math.floor(rgba.alpha*255)
  end

  dialog:show_all()
end
