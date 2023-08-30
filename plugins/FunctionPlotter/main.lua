function initUi()
  app.registerUi({["menu"] = "Plot function...", ["callback"] = "showDialog"});
  
  sourcePath = debug.getinfo(1).source:match("@?(.*/)")
end

function showDialog()
  local math = require("math")
  local hasLuaxp, luaxp = pcall(require, "luaxp")
  if not hasLuaxp then
    app.openDialog("You should have a copy of the luaxp module inside the plugin folder.", {"OK"}, "", true)
    return
  end

  local hasLgi, lgi = pcall(require, "lgi")
  if not hasLgi then
    app.openDialog("You should have the Lua lgi-module installed and included in your Lua package path.", {"OK"}, "", true)
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
    local xc, yc = {}, {}
    local autofit = ui.cbAutoFit:get_active()
    if autofit then
      print("autofitting")
      local xMin, xMax, yMin, yMax = 0/0, 0/0, 0/0, 0/0
      -- draw graph
      for i = 0, samples-1 do
        local t = tMin + i/(samples-1)*(tMax-tMin)
        local xt = eval("x", t) 
        local yt = eval("y", t)
        local defined = xt~=nil and yt~=nil and xt==xt and yt==yt  -- not nil and not nan
        if defined then
          xMin, xMax = math.min(xt, xMin), math.max(xt, xMax)
          yMin, yMax = math.min(yt, yMin), math.max(yt, yMax)
          table.insert(xc, xt)
          table.insert(yc, yt)
        else
          table.insert(xc, nil)
          table.insert(yc, nil)
        end
      end
      wc.xMin = xMin
      wc.xMax = xMax
      wc.yMin = yMin
      wc.yMax = yMax
      print(wc.xMin, wc.xMax, wc.yMin, wc.yMax)
      for i = 1, #xc do
        local xt, yt = xc[i], yc[i]
        local defined = xt~=nil
        if defined then
          table.insert(xcoord, fitX(xt))
          table.insert(ycoord, fitY(yt))
        else 
          appendStroke(strokes, xcoord, ycoord)
          xcoord = {}; ycoord = {}
        end
      end
    else 
      local lastInside = false
      local last = nil
      local p = nil
      -- draw graph
      for i = 0, samples-1 do
        local t = tMin + i/(samples-1)*(tMax-tMin)
        local xt = eval("x", t) 
        local yt = eval("y", t)
        local defined = xt~=nil and yt~=nil and xt==xt and yt==yt  -- not nil and not nan
        local inside = (wc.xMin <= xt and xt<=wc.xMax and wc.yMin <= yt and yt <= wc.yMax)
        if not defined then 
          last = nil
        elseif inside or lastInside then
          if inside and (last==nil or lastInside) then
            table.insert(xcoord, fitX(xt))
            table.insert(ycoord, fitY(yt))
            last = {x=xt, y=yt}
            lastInside = true
          elseif inside then
            p = interpolate(xt, yt, last.x, last.y)
            print(xt, yt, last.x, last.y)
            table.insert(xcoord, fitX(p.x))
            table.insert(ycoord, fitY(p.y))
            table.insert(xcoord, fitX(xt))
            table.insert(ycoord, fitY(yt))  
            last = {x=xt, y=yt}
            lastInside = true
          elseif lastInside then
            p = interpolate(last.x, last.y, xt, yt)
            table.insert(xcoord, fitX(p.x))
            table.insert(ycoord, fitY(p.y))
            last = {x=xt, y=yt}
            lastInside = false
          end
        else
          appendStroke(strokes, xcoord, ycoord)
          xcoord = {}; ycoord = {}
          last = {x=xt, y=yt}
        end
      end
    end
    appendStroke(strokes, xcoord, ycoord)

    local colAxis, alphaAxis = packRgba(ui.colAxis:get_rgba())
    local colGrid, alphaGrid = packRgba(ui.colGrid:get_rgba())
    local axisStyle = ui.comboentryAxis:get_text()
    local gridStyle = ui.comboentryGrid:get_text()
    local lwAxis = ui.spbtLineWidthAxis:get_value()
    local lwGrid = ui.spbtLineWidthGrid:get_value()
    local tickHeight = ui.spbtTickHeight:get_value()
    local xLabel = ui.entryLabelX:get_text()
    local yLabel = ui.entryLabelY:get_text()
    local fontName = ui.fnt:get_font()
    local fontSize = ui.fnt:get_font_size()
    local arrowLength = 8
    local mx, my = fit(wc.xMin, wc.yMin)
    local Mx, My = fit(wc.xMax, wc.yMax)
    local x0, y0 = fit(0, 0)

    local drawAxisLines = ui.cbAxisLines:get_active()
    local drawTicks = ui.cbTicks:get_active()
    local drawGrid = ui.cbGrid:get_active()
    local displayNumbers = ui.cbNumbers:get_active()
    local displayAxisLabels = ui.cbAxisLabels:get_active()

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

    local texts = {}
    local PANGO_SCALE = fontSize > 500 and 1024 or 1   -- accounting for what looks like a bug in lua-lgi
    local font = {name = fontName, size = fontSize/PANGO_SCALE}

    if displayNumbers then
      -- draw number on x-axis
      for i = (wc.xMin // wc.xUnit) + 1, (wc.xMax // wc.xUnit) do
        local v = i*wc.xUnit
        local xval = fitX(v)
        local text = (v == math.floor(v)) and tostring(math.floor(v)) or tostring(v)
        if xval >= Mx - 2*arrowLength then break end
        table.insert(texts, {text = text, font = font, x = xval-font.size/2, y = y0, color = colAxis})
      end
      -- draw ticks on y-axis
      for i = (wc.yMin // wc.yUnit) + 1, (wc.yMax // wc.yUnit) do
        local v = i*wc.yUnit
        local yval = fitY(v)
        local text = (v == math.floor(v)) and tostring(math.floor(v)) or tostring(v)
        if yval <= My + 2*arrowLength then break end
        if (v~=0) then
          table.insert(texts, {text = text, font = font, x = x0+tickHeight, y = yval-font.size, color = colAxis}) 
        end
      end
    end

    if displayAxisLabels then
      local xval = fitX(wc.xMax)
      local yval = fitY(wc.yMax)
      table.insert(texts, {text = xLabel, font = font, x=xval-font.size, y=y0+font.size, color=colAxis})
      table.insert(texts, {text = yLabel, font = font, x=x0-2*font.size, y=yval-font.size, color=colAxis})
    end

    app.addTexts({texts = texts})
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

  function interpolate(x0,y0,x1,y1)
    -- Assume (x0, y0) inside, (x1, y1) outside
    local candidates = {}
    if x1>wc.xMax then
      t = (wc.xMax-x0)/(x1-x0)        -- x0+t*(x1-x0) = wc.xMax
      table.insert(candidates, {x=wc.xMax, y=y0+t*(y1-y0)})
    end
    if x1<wc.xMin then
      t = (wc.xMin-x0)/(x1-x0)        -- x0+t*(x1-x0) = wc.xMin
      table.insert(candidates, {x=wc.xMin, y=y0+t*(y1-y0)})
    end
    if y1>wc.yMax then
      t = (wc.yMax-y0)/(y1-y0)        -- y0+t*(y1-y0) = wc.yMax
      table.insert(candidates, {x=x0+t*(x1-x0), y=wc.yMax})
    end
    if y1<wc.yMin then
      t = (wc.yMin-y0)/(y1-y0)        -- y0+t*(y1-y0) = wc.yMin
      table.insert(candidates, {x=x0+t*(x1-x0), y=wc.yMin})
    end
    -- return the first candidate inside the rectangle
    for i=1,#candidates do
      c = candidates[i]
      if c.x>=wc.xMin and c.x<=wc.xMax and c.y>=wc.yMin and c.y<=wc.yMax then
        return c
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

  function ui.cbAutoFit.on_toggled()
    local active = not ui.cbAutoFit:get_active()
    ui.spbtXMin:set_sensitive(active)
    ui.spbtXMax:set_sensitive(active)
    ui.spbtYMin:set_sensitive(active)
    ui.spbtYMax:set_sensitive(active)
  end

  function ui.cbAxisLines.on_toggled()
    local active = ui.cbAxisLines:get_active()
    ui.colAxis:set_sensitive(active)
    ui.comboAxis:set_sensitive(active)
    ui.spbtLineWidthAxis:set_sensitive(active)
  end

  function ui.cbTicks.on_toggled()
    local active = ui.cbTicks:get_active()
    ui.spbtXUnit:set_sensitive(active)
    ui.spbtYUnit:set_sensitive(active)
    ui.spbtTickHeight:set_sensitive(active)
  end

  function ui.cbGrid.on_toggled()
    local active = ui.cbGrid:get_active()
    ui.colGrid:set_sensitive(active)
    ui.comboGrid:set_sensitive(active)
    ui.spbtLineWidthGrid:set_sensitive(active)
    ui.spbtYMax:set_sensitive(active)
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
    return math.floor(256^2*255*rgba.red + 256*255*rgba.green+255*rgba.blue), math.floor(rgba.alpha*255)
  end

  dialog:show_all()
end
