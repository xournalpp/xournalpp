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
  print(notePath)

  local f = assert(io.popen("unzip -Z -1 " .. notePath .. " | head -1 | cut -d'/' -f1"))
  local dirname = assert(f:read('*a'))
  f:close()
  dirname = dirname:gsub('%\n', '')  --remove newline
  print(dirname)

  local contents = extract(dirname, notePath, "Session.plist")
  parsedContents = plistParse(contents)

  local pdfFileName = getValue({"richText", "pdfFiles", 1, "pdfFileName"})
  print(pdfFileName)
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

  local numpoints = getData("curvesnumpoints", "u")
  local coords = getData("curvespoints", "f")
  local widths = getData("curveswidth", "f")
  local pressures = getData("curvesfractionalwidths", "f")
  local styles = getData("curvesstyles", "b") -- can be {}
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
     opacity = (cols[i] >> 24) % 256
     tools[i] = opacity < 128 and "highlighter" or "pen"
     r =  (cols[i] >> 16) % 256
     g = (cols[i] >> 8) % 256
     b = cols[i] % 256
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
          table.insert(strokesOfCurrentPage, newStroke)
        end
      end
      if #strokesOfCurrentPage > 0 then
        app.addStrokes({strokes = strokesOfCurrentPage})
        app.refreshPage()
      end
      if j < maxOffset then app.uiAction({action = "ACTION_NEW_PAGE_AFTER"}) end
    end
  end
  
  print("Text in document: ")
  local rawText = getValue({"richText", "attributedString", 1})
  print(rawText)
end
