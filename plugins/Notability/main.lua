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

function getValue(keySeq, startIndex)
  local id = startIndex == nil and 1 or startIndex  -- Lua list index is always one bigger, since list indices start from 1
  for i=1, #keySeq do
    local key = keySeq[i]
    if type(key) == "string" then
      id = parsedContents["$objects"][id+1][key]["CF$UID"]
    elseif type(key) == "number" then
      tmp = parsedContents["$objects"][id+1]["NS.objects"][key]
      if tmp ~= nil then
        id = tmp["CF$UID"]
      else 
        return nil
      end
    else 
      print("type of key " .. key .. " is not number and not string")
      return nil -- not supported
    end
  end
  return parsedContents["$objects"][id+1]
end

function getData(str, fmt)
  print("Accessing " .. str)
  local data = getValue({"richText", "Handwriting Overlay", "SpatialHash"})[str]
  data = string.gsub(data, "%s+", "") -- remove white spaces
  return dec(data, fmt)
end

function extract(notePath, filename)
  local f = assert(io.popen("unzip -Z -1 " .. notePath .. " | head -1 | cut -d'/' -f1"))
  local dirname = assert(f:read('*a'))
  f:close()
  dirname = dirname:gsub('%\n', '')  --remove newline
  local plistPath = "'" .. dirname .. "/" .. filename .. "'"
  print(plistPath)

  local runCommand = assert(io.popen("unzip -p " .. notePath .. " " .. plistPath .. " | plistutil"))
  local result = runCommand:read('*all')
  runCommand:close()
  return result
end

function import()
  require("plist")     -- for reading the plist in xml format to a Lua table
  require("base64")    -- for base64 decription

  local notePath = app.getFilePath({'*.note'})
  notePath = "'" .. notePath .. "'" -- take care of spaces in file name
  print(notePath)

  local contents = extract(notePath, "Session.plist")
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
      if widths[i] ~= nil then 
        styles[i] = "cust: " .. 2*widths[i] .. " "  .. 2*widths[i]
      else 
        styles[i] = "cust: 20 20"   -- TODO: Fix this (rolandlo)
      end
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
  print(paperSize)
  paperWidth = paperSizeDict[paperSize]["width"]
  paperHeight = paperSizeDict[paperSize]["height"]
  app.setPageSize(paperWidth, paperHeight)

  numPages = 0
  pageOffset = {}
  for i=1, #strokes do
    pageOffset[i] = math.floor(strokes[i]["y"][1] // paperHeight)
    print("Page offset of stroke " .. i .. " is " .. pageOffset[i])
  end
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
        table.insert(strokesOfCurrentPage, newStroke)
        print("Adding new stroke to page with offset " .. j)
      end
    end
    if #strokesOfCurrentPage > 0 then
      app.addStrokes({strokes = strokesOfCurrentPage})
      app.refreshPage()
    end
    if j < maxOffset then app.uiAction({action = "ACTION_NEW_PAGE_AFTER"}) end
  end

  print("Text in document: ")
  local rawText = getValue({"richText", "attributedString", 1})
  print(rawText)
  local pdfFileName = getValue({"richText", "pdfFiles", 1, "pdfFileName"})
  print(pdfFileName)
end
