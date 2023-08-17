function initUi()
  app.registerUi({["menu"] = "Import .note file", ["callback"] = "import"});
end

local paperSizeDict = {
  letter = {width = 611.86, height = 791.82},
  legal = {width = 611.86, height = 1007.77},
  a3 = {width = 841.70, height = 1190.28},
  a4 = {width = 595.14, height = 841.70},
  a5 = {width = 420.85, height = 595.14}
}

local push = table.insert

function getValue(keySeq, startIndex)
  local id = startIndex == nil and 1 or startIndex  -- Lua list index is always one bigger, since list indices start from 1
  for i=1, #keySeq do
    local key = keySeq[i]
    if type(key) == "string" then
      tmp = parsedContents["$objects"][id+1][key]
    elseif type(key) == "number" then
      tmp = parsedContents["$objects"][id+1]["NS.objects"][key]
    else 
      print("type of key " .. key .. " is not number and not string")
      return nil -- not supported
    end
    if tmp ~= nil then
      id = tmp["CF$UID"]
    else 
      return nil
    end
  end
  return parsedContents["$objects"][id+1]
end

function getData(str, fmt)
  print("Accessing " .. str)
  local data = getValue({"richText", "Handwriting Overlay", "SpatialHash"})[str]
  if data == nil then
    print("the key " .. str .. " was not found") 
    return {}
  end
  data = string.gsub(data, "%s+", "") -- remove white spaces
  return dec(data, fmt)
end

function extract(dirname, notePath, filename)
  local plistPath = "'" .. dirname .. "/" .. filename .. "'"
  print(plistPath)

  local cmd = "unzip -p " .. notePath .. " " .. plistPath .. " | plistutil"
  print(cmd)
  local runCommand = assert(io.popen(cmd))
  local result = runCommand:read('*all')
  runCommand:close()
  return result
end

function import()
  require("plist")     -- for reading the plist in xml format to a Lua table
  require("base64")    -- for base64 decription

  local notePath = app.getFilePath({'*.note'})
  notePath = "'" .. notePath .. "'" -- take care of spaces in file name

  local time1 = os.clock()
  local f = assert(io.popen("unzip -Z -1 " .. notePath .. " | head -1 | cut -d'/' -f1"))
  local dirname = assert(f:read('*a'))
  f:close()
  dirname = dirname:gsub('%\n', '')  --remove newline

  local contents = extract(dirname, notePath, "Session.plist")
  parsedContents = plistParse(contents)
  local time2 = os.clock()

  local pdfFileName = getValue({"richText", "pdfFiles", 1, "pdfFileName"})

  if pdfFileName then
    ans = app.msgbox("There is a PDF background in the .note-file. Do you want to save and use it? A new document will be created in that case. ", {[1]="Yes", [2]="No"}) 
    if ans == 1 then
      local pdfFilePath = app.saveAs(pdfFileName)
      print(pdfFilePath)
      local originalPdfPath = "'" .. dirname .. "/PDFs/" .. pdfFileName .. "'"
      print(originalPdfPath)
      local runCommand = assert(io.popen("unzip -p " .. notePath .. " " .. originalPdfPath .. " > " .. pdfFilePath))
      local res = runCommand:read("*all")
      print(res)
      runCommand:close()
      app.openFile(pdfFilePath, 0, true);
    else 
      print("Dismissing PDF background from .note file")
    end
  end

  local rawText = getValue({"richText", "attributedString", 1})
  if (rawText and #rawText>0) then
    local font = {name = "Noto Sans Mono Medium", size=12.0}
    app.addTexts{texts={{text=rawText, font=font, color=0x000000, x = 0.0,y = 0.0}}}
  end

  local time3 = os.clock()
  
  local numpoints = getData("curvesnumpoints", "I")
  local coords = getData("curvespoints", "f")
  local widths = getData("curveswidth", "f")
  local pressures = getData("curvesfractionalwidths", "f")
  local styles = getData("curvesstyles", "H") -- can be {}
  local cols = getData("curvescolors", "I")

  local time4 = os.clock()

  -- post-process styles
  for i = 1, #styles do
    if math.floor(styles[i]) == 3 then
      styles[i] = "solid"
    elseif math.floor(styles[i]) == 4 then
      if widths[i] ~= nil then 
        styles[i] = "cust: " .. 2*widths[i] .. " "  .. 2*widths[i]
      else 
        styles[i] = "cust: 20 20"   -- TODO: Fix this (rolandlo)
      end
    else
      styles[i] = "solid" 
      print("Unsupported style, choosing 'solid'")
    end
  end
  local opacity = {} 
  local tools = {}
  for i = 1, #cols do
     opacity = (cols[i] >> 24) % 256
     tools[i] = opacity < 128 and "highlighter" or "pen"
     r =  (cols[i] >> 16) % 256
     g = (cols[i] >> 8) % 256
     b = cols[i] % 256
     cols[i] = r + g*256 + b*256^2
  end

  local time5 = os.clock()

  local strokes = {}
  local pi = 0
  local numCurves = getValue({"richText", "Handwriting Overlay", "SpatialHash", "numcurves"})
  print("Number of curves in document: " .. numCurves)
  local b = 0

  for s = 1, numCurves do
    local x, y, pressure, pointIndex, strokeIndex = {}, {}, {}, 0
    a,b = b+1, b + numpoints[s]

    for i = a, b do
      if i==a or ((i-a) % 3 == 1 and i>a+1) then pi = pi+1 end  -- the first point on each stroke and from there one each 3 consecutive points share the same pressure value
      pointIndex = pointIndex + 1
      x[pointIndex] = coords[2*i-1] 
      y[pointIndex] = coords[2*i]
      pressure[pointIndex] = pressures[pi]*widths[s]
    end

    strokes[s] = {x=x, y=y, width = widths[s], pressure = pressure, color = math.floor(cols[s]), lineStyle = styles[s], tool = tools[s]}
  end

  local time6 = os.clock()

  local shapesBinary = getValue({"richText", "Handwriting Overlay", "SpatialHash", "shapes"})
  if shapesBinary then
    shapesBinary = shapesBinary:gsub("[\n\t ]", "") -- remove line breaks, tabs and spaces
    local cmd = "echo '" .. shapesBinary .. "' | base64 -d | plistutil"
    local runCommand = assert(io.popen(cmd))
    local result = runCommand:read('*all')
    local shapes = plistParse(result)
    local numShapes = #shapes["kinds"]
    for i = 1, numShapes do
      handle = true
      x = {}; y = {}
      local kind = shapes["kinds"][i]
      local shape = shapes["shapes"][i]
      if kind == "line" then
        x = {shape["startPt"][1], shape["endPt"][1]}
        y = {shape["startPt"][2], shape["endPt"][2]}
      elseif kind == "circle" then
        print("circle")
        local rx = shape["rect"][2][1]/2
        local ry = shape["rect"][2][2]/2
        local cx = shape["rect"][1][1] + rx
        local cy = shape["rect"][1][2] + ry
        local npts = math.floor(math.max(24, 2*rx, 2*ry))
        for k = 0, npts do
          push(x, cx+rx*math.cos(2*math.pi*k/npts))
          push(y, cy+ry*math.sin(2*math.pi*k/npts))
        end
      elseif kind == "square" or kind == "polygon" then
        local points = shape["points"]
        local numVertices = #points
        print("polygon with " .. numVertices .. " vertices")
        for j = 1, numVertices do
          push(x, points[j][1])
          push(y, points[j][2])
        end
        if shape["isClosed"] then
          push(x, x[1])
          push(y, y[1])
        end
      elseif kind == "partialshape" then
        print("Partial shape")
        -- ['rect', 'appearance', 'rotatedRect', 'extremePoints', 'outlinePath', 'strokePath']
        handle = false
      else
        print("Unknown kind: " .. kind)
        handle = false
      end
      if handle then
        local r,g,b,a = table.unpack (shape["appearance"]["strokeColor"]["rgba"])
        local tool = a < 0.5 and "highlighter" or "pen"
        local style = shape["appearance"]["style"]
        local width = shape["appearance"]["strokeWidth"]
        local stroke = {x=x, y=y, width = width, color = math.floor(b*255 + g*256*255 + r*256^2*255), tool=tool}
        push(strokes, stroke)
      end
    end
  end

  local time7 = os.clock()

  local docStructure = app.getDocumentStructure()
  local currentPage = docStructure["currentPage"]
  local isAnnotated = docStructure["pages"][currentPage]["isAnnotated"]
  local bgCurrentPage = docStructure["pages"][currentPage]["pageTypeFormat"]
  local hasPdfBackground = (bgNextPage == ":pdf")

  if isAnnotated or hasPdfBackground then
    app.uiAction({action = "ACTION_NEW_PAGE_AFTER"})
    currentPage = currentPage + 1
    app.changeCurrentPageBackground("plain")
  end

  local paperSize = getValue({"NBNoteTakingSessionDocumentPaperLayoutModelKey", "documentPaperAttributes", "paperSize"})
  paperSize = paperSize ~= nil and paperSize or "a4"
  print(paperSize)
  paperWidth = paperSizeDict[paperSize]["width"]
  paperHeight = paperSizeDict[paperSize]["height"]
  app.setPageSize(paperWidth, paperHeight)

  numPages = 0
  pageOffset = {}
  for i=1, #strokes do
    pageOffset[i] = math.floor(strokes[i]["y"][1] // paperHeight)
  end
  if #pageOffset>0 then
    maxOffset = math.max(table.unpack(pageOffset))
    print("Max offset = " .. maxOffset)
    for j = 0, maxOffset do
      strokesOfCurrentPage = {}
      for k = 1, #strokes do
        if pageOffset[k] == j then
          newStroke = strokes[k]
          for l = 1, #newStroke["y"] do 
            newStroke["y"][l] = newStroke["y"][l] - pageOffset[k]*paperHeight
          end
          push(strokesOfCurrentPage, newStroke)
        end
      end
      if #strokesOfCurrentPage > 0 then
        app.addStrokes({strokes = strokesOfCurrentPage})
        app.refreshPage()
      end
      if j < maxOffset then app.uiAction({action = "ACTION_NEW_PAGE_AFTER"}) end
    end
  end

  local time8 = os.clock()

  print(string.format("Extract and parse file contents: %.2f sec", time2 - time1))
  print(string.format("Open PDF background: %.2f sec", time3 - time2))
  print(string.format("Decode points, styles and colors: %.2f sec", time4 - time3))
  print(string.format("Postprocess styles and colors: %.2f sec", time5 - time4))
  print(string.format("Create strokes table: %.2f sec", time6 - time5))
  print(string.format("Process shapes: %.2f sec", time7 - time6))
  print(string.format("Add elements to document: %.2f sec", time8 - time7))
end
