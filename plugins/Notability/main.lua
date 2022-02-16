function initUi()
  app.registerUi({["menu"] = "Import .note file", ["callback"] = "import"});
end

function getValue(keySeq)
  local id = 1   -- Lua list index is always one bigger, since list indices start from 1
  for i=1, #keySeq do
    id = parsedContents["$objects"][id+1][keySeq[i]]["CF$UID"]
  end
  return parsedContents["$objects"][id+1]
end

function getData(str, fmt)
  print("Accessing " .. str)
  local data = getValue({"richText", "Handwriting Overlay", "SpatialHash"})[str]
  data = string.gsub(data, "%s+", "") -- remove white spaces
  return dec(data, fmt)
end

function extract()
  local notePath = app.getFilePath({'*.note'})
  print(notePath)

  local f = assert(io.popen("unzip -Z -1 " .. notePath .. " | head -1 | cut -d'/' -f1"))
  local dirname = assert(f:read('*a'))
  dirname = dirname:gsub('%\n', '')  --remove newline
  f:close()

  os.execute("unzip -o " .. notePath)

  local plistPath = "'" .. dirname .. "/Session.plist'"

  os.execute("plistutil -i " .. plistPath .. " -o /tmp/Session.xml")
end

function import()
  extract()
  require("plist")     -- for reading the plist in xml format to a Lua table
  require("base64")    -- for base64 decription

  local file = io.open("/tmp/Session.xml")
  local contents = file:read("*all")
  parsedContents = plistParse(contents)

  local numpoints = getData("curvesnumpoints", "u")
  local coords = getData("curvespoints", "f")
  local widths = getData("curveswidth", "f")
  local pressures = getData("curvesfractionalwidths", "f")
  local styles = getData("curvesstyles", "b")
  local cols = getData("curvescolors", "u")

  -- post-process styles and cols
  for i = 1, #styles, 2 do
    tmp = styles[i]
    styles[i] = styles[i+1]
    styles[i+1] = tmp
  end

  for i = 1, #styles do
    if math.floor(styles[i]) == 3 then
      styles[i] = "solid"
    elseif math.floor(styles[i]) == 4 then
      styles[i] = "cust: " .. 2*widths[i] .. " "  .. 2*widths[i] 
    else
      styles[i] = "solid"  -- don't know that style, so choose "solid" for now
    end
  end
  local opacity = {} 
  local tools = {}
  for i = 1, #cols do
     opacity = rsh(cols[i], 24)
     tools[i] = opacity < 128 and "highlighter" or "pen"
     r =  rsh(cols[i], 16)
     g = rsh(cols[i], 8)
     b = rsh(cols[i], 0)
     cols[i] = r + g*256 + b*256^2
  end

  strokes = {}
  pi = 0
  local numCurves = getValue({"richText", "Handwriting Overlay", "SpatialHash", "numcurves"})
  print("Number of curves in document: " .. numCurves)
  for s = 1, numCurves do
    x = {}; y = {}; pressure = {}
    a = 1; b = numpoints[1]
    for i=1, s-1 do
      a=a+numpoints[i]
      b=b+numpoints[i+1]
    end

    for i = a, b do
      if i==a or ((i-a) % 3 == 1 and i>a+1) then pi = pi+1 end  -- the first point on each stroke and from there one each 3 consecutive points share the same pressure value
      table.insert(x, coords[2*i-1]) 
      table.insert(y, coords[2*i])
      table.insert(pressure, pressures[pi]*widths[s])
    end


    stroke = {x=x, y=y, width = widths[s], pressure = pressure, color = math.floor(cols[s]), lineStyle = styles[s], tool = tools[s]}
    table.insert(strokes, stroke)
  end

  app.addStrokes({strokes = strokes})
  app.refreshPage()

  print("Text in document: ")
  local indRawText = getValue({"richText", "attributedString"})["NS.objects"][1]["CF$UID"]
  local rawText = parsedContents["$objects"][indRawText+1]
  print(rawText)
end
