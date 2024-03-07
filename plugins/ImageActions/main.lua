function initUi()
  app.registerUi({ ["menu"] = "Invert selected images", ["callback"] = "cb", ["mode"] = 1 })
  app.registerUi({ ["menu"] = "Downscale resolution of selected images by factor 0.5", ["callback"] = "cb", ["mode"] = 2 })
  app.registerUi({ ["menu"] = "Flip selected images horizontally", ["callback"] = "cb", ["mode"] = 3 })
  app.registerUi({ ["menu"] = "Flip selected images vertically", ["callback"] = "cb", ["mode"] = 4 })
  app.registerUi({ ["menu"] = "Rotate selected images 90 degree clockwise", ["callback"] = "cb", ["mode"] = 5 })
  app.registerUi({ ["menu"] = "Rotate selected images 90 degree counterclockwise", ["callback"] = "cb", ["mode"] = 6 })
  app.registerUi({ ["menu"] = "Make white pixels transparent", ["callback"] = "cb", ["mode"] = 7 })
  app.registerUi({ ["menu"] = "Trim selected images", ["callback"] = "cb", ["mode"] = 8 })
  app.registerUi({ ["menu"] = "Split images vertically by largest white block", ["callback"] = "cb", ["mode"] = 9 })
  app.registerUi({ ["menu"] = "Split images horizontally by largest white block", ["callback"] = "cb", ["mode"] = 10 })
end

function cb(mode)
  local images = app.getImages("selection")
  local hasVips, vips = pcall(require, "vips")
  if not hasVips then
    app.openDialog(
      "You need to have lua-vips and luaffifb installed and included in your Lua package path in order to use this plugin. \n" ..
      "See https://github.com/rolandlo/lua-vips/tree/fix-lua-5.3#installation for installation instructions.\n\n", { "OK" },
      "", true)
    return
  end
  local imdata = {}
  for i = 1, #images do
    local im = images[i]
    local image = vips.Image.new_from_buffer(im["data"])
    local x, y, maxWidth, maxHeight = im["x"], im["y"], math.ceil(im["width"]), math.ceil(im["height"])
    if mode == 1 then
      image = image:invert()
    elseif mode == 2 then
      image = image:resize(0.5)
    elseif mode == 3 then
      image = image:flip("horizontal")
    elseif mode == 4 then
      image = image:flip("vertical")
    elseif mode == 5 then
      image = image:rot("d90")
      maxWidth, maxHeight = maxHeight, maxWidth
    elseif mode == 6 then
      image = image:rot("d270")
      maxWidth, maxHeight = maxHeight, maxWidth
    elseif mode == 7 then
      if image:bands() < 4 then
        image = image:bandjoin(255)    -- add alpha channel
      end
      local white = { 240, 240, 240, 0 } -- pixels are considered white, if its r,g,b values are >=240 and alpha is arbitrary
      local transparent = { 0, 0, 0, 0 }
      image = image:more(white):bandand():ifthenelse(transparent, image)
    elseif mode == 8 then
      local width = image:width()
      local height = image:height()
      image = image:crop(image:find_trim())
      local newWidth = image:width()
      local newHeight = image:height()
      maxWidth, maxHeight = maxWidth * newWidth // width, maxHeight * newHeight // height
    end

    if mode ~= 9 and mode ~= 10 then
      table.insert(imdata, { data = image:write_to_buffer(".png"), x = x, y = y, maxWidth = maxWidth, maxHeight =
      maxHeight })
    else
      local vertical = mode == 9
      -- find largest sequence of white or transparent pixel rows/columns
      local filter = image:colourspace("b-w"):extract_band(0):more(240)
      local width = filter:width()
      local height = filter:height()
      local last = vertical and height or width
      local isWhite = {}

      for i = 0, last - 1 do
        if vertical then
          isWhite[i + 1] = filter:extract_area(0, i, width, 1):avg() > 254
        else
          isWhite[i + 1] = filter:extract_area(i, 0, 1, height):avg() > 254
        end
      end

      if #isWhite == 0 then
        local word = vertical and "rows" or "columns"
        print("There are no completely white " .. word .. " in this image. Skipping.")
        table.insert(imdata, { data = im["data"], x = x, y = y, maxHeight = maxHeight, maxWidth = maxWidth })
        goto continue
      end

      local whiteEnds = {}
      local whiteLengths = {}
      local firstAfter = false
      local currentLength = 0
      local n = 0
      for i = 1, last do
        if isWhite[i] then
          if currentLength > 0 then
            currentLength = currentLength + 1
          else
            currentLength = 1
            firstAfter = true
          end
        else
          if firstAfter then
            whiteEnds[n] = i - 1
            whiteLengths[n] = currentLength
            n = n + 1
            firstAfter = false
            currentLength = 0
          end
        end
      end
      whiteEnds[0] = nil
      whiteLengths[0] = nil

      if #whiteEnds == 0 then
        print("Could not find a split")
        table.insert(imdata, { data = im["data"], x = x, y = y, maxHeight = maxHeight, maxWidth = maxWidth })
        goto continue
      end

      local maxLength = 0
      local index = 0
      for i = 1, #whiteEnds do
        if whiteLengths[i] > maxLength then
          maxLength = whiteLengths[i]
          index = whiteEnds[i]
        end
      end
      if vertical then
        local image1 = image:extract_area(0, 0, width, index - maxLength // 2)
        local image2 = image:extract_area(0, index - maxLength // 2, width, height - index + maxLength // 2)
        table.insert(imdata, { data = image1:write_to_buffer(".png"), x = x, y = y, maxWidth = maxWidth })
        table.insert(imdata, { data = image2:write_to_buffer(".png"), x = x, y = y + index * maxHeight / height, maxWidth =
        maxWidth })
      else
        local image1 = image:extract_area(0, 0, index - maxLength // 2, height)
        local image2 = image:extract_area(index - maxLength // 2, 0, width - index + maxLength // 2, height)
        table.insert(imdata, { data = image1:write_to_buffer(".png"), x = x, y = y, maxHeight = maxHeight })
        table.insert(imdata, { data = image2:write_to_buffer(".png"), x = x + index * maxWidth / width, y = y, maxHeight =
        maxHeight })
      end
      ::continue::
    end
  end
  app.uiAction({ action = "ACTION_DELETE" })
  app.addImages({ images = imdata, allowUndoRedoAction = "grouped" })
end
