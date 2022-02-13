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
    local xMin = ui.spbtXMin:get_value()
    local xMax = ui.spbtXMax:get_value()
    local xUnit = ui.spbtXUnit:get_value()
    local yMin = ui.spbtYMin:get_value()
    local yMax = ui.spbtYMax:get_value()
    local yUnit = ui.spbtYUnit:get_value()
    local tMin = ui.spbtTMin:get_value()
    local tMax = ui.spbtTMax:get_value()
    local width = ui.spbtWidth:get_value()
    local height = ui.spbtHeight:get_value()
    local samples = ui.spbtSamples:get_value()
    local type = ui.comboentryType:get_text()
    local unit = ui.comboentryUnit:get_text()
    local factor = getFactor(unit)

    local xcoord, ycoord = {}, {}
    local cx, cy = 300, 200
    local xppWidth = width*factor
    local xppHeight = height*factor
    local strokes = {}
    for i = 0, samples-1 do
      local t = tMin + i/(samples-1)*(tMax-tMin)
      local xt = eval("x", t) 
      local yt = eval("y", t)
      if (xt~=nil and xt>=xMin and xt<=xMax and yt~=nil and yt>=yMin and yt<=yMax) then
        table.insert(xcoord, fitX(xMin, xMax, xppWidth, cx, xt))
        table.insert(ycoord, fitY(yMin, yMax, xppHeight, cy, yt))
      else 
        appendStroke(strokes, xcoord, ycoord)
        xcoord = {}; ycoord = {}
      end
    end
    appendStroke(strokes, xcoord, ycoord)

    local mx = fitX(xMin, xMax, xppWidth, cx, xMin)
    local Mx = fitX(xMin, xMax, xppWidth, cx, xMax)
    local x0 = fitX(xMin, xMax, xppWidth, cx, 0)
    local my = fitY(yMin, yMax, xppHeight, cy, yMin)
    local My = fitY(yMin, yMax, xppHeight, cy, yMax)
    local y0 = fitY(yMin, yMax, xppHeight, cy, 0)
    local xAxis = {x = {mx,Mx}, y={y0, y0}, color=0x000000, width=1}
    local yAxis = {x = {x0, x0}, y={my, My}, color=0x000000, width=1}
    table.insert(strokes, xAxis)
    table.insert(strokes, yAxis)

    local tickHeight = 4.0
    local xticks = { strokes = {}}
    for i = (xMin // xUnit), (xMax // xUnit) do
      if i*xUnit >= xMin and i*xUnit <= xMax then 
        local xval = fitX(xMin, xMax, xppWidth, cx, i*xUnit)
        table.insert(strokes, {x = {xval, xval}, y = {y0+tickHeight/2, y0-tickHeight/2}, width = 1.0, color = 0x000000})
      end
    end
    local yticks = { strokes = {}}
    for i = (yMin // yUnit), (yMax // yUnit) do
      if i*yUnit >= yMin and i*yUnit <= yMax then
        local yval = fitY(yMin, yMax, xppHeight, cy, i*yUnit)
        table.insert(strokes, {x = {x0-tickHeight/2, x0+tickHeight/2}, y = {yval, yval}, width = 1.0, color = 0x000000})
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

  function fitX(xmin, xmax, width, cx, t)
    return width/(xmax-xmin)*(t-xmin) + cx-width/2
  end

  function fitY(ymin, ymax, height, cy, t)
    return -height/(ymax-ymin)*(t-ymin) + cy+height/2
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

  dialog:show_all()
end
