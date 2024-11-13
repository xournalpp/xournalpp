local m = {
  INVERT = 1,
  DOWNSCALE = 2,
  FLIP_H = 3,
  FLIP_V = 4,
  ROTATE_CW = 5,
  ROTATE_CCW = 6,
  TRANSPARENT = 7,
  TRIM = 8,
  SPLIT_V = 9,
  SPLIT_H = 10,
}

function initUi()
  app.registerUi({ ["menu"] = "Invert selected images", ["callback"] = "Cb", ["mode"] = m.INVERT })
  app.registerUi({
    ["menu"] = "Downscale resolution of selected images by factor 0.5",
    ["callback"] = "Cb",
    ["mode"] = m.DOWNSCALE
  })
  app.registerUi({ ["menu"] = "Flip selected images horizontally", ["callback"] = "Cb", ["mode"] = m.FLIP_H })
  app.registerUi({ ["menu"] = "Flip selected images vertically", ["callback"] = "Cb", ["mode"] = m.FLIP_V })
  app.registerUi({
    ["menu"] = "Rotate selected images 90 degree clockwise",
    ["callback"] = "Cb",
    ["mode"] = m.ROTATE_CW
  })
  app.registerUi({
    ["menu"] = "Rotate selected images 90 degree counterclockwise",
    ["callback"] = "Cb",
    ["mode"] = m.ROTATE_CCW
  })
  app.registerUi({ ["menu"] = "Make white pixels transparent", ["callback"] = "Cb", ["mode"] = m.TRANSPARENT })
  app.registerUi({ ["menu"] = "Trim selected images", ["callback"] = "Cb", ["mode"] = m.TRIM })
  app.registerUi({
    ["menu"] = "Split images vertically by largest white block",
    ["callback"] = "Split",
    ["mode"] = m.SPLIT_V
  })
  app.registerUi({
    ["menu"] = "Split images horizontally by largest white block",
    ["callback"] = "Split",
    ["mode"] = m.SPLIT_H
  })
end

local function checkVips()
  local hasVips, vips = pcall(require, "vips")
  if not hasVips then
    app.openDialog(
      "You need to have lua-vips installed and included in your Lua package path in order to use this plugin. \n" ..
      "Use luarocks install lua-vips and see https://github.com/libvips/lua-vips for further information.\n\n",
      { "OK" },
      "", true)
    return nil
  end
  return vips
end

function Cb(mode)
  local vips = checkVips()
  if not vips then return end

  local images = app.getImages("selection")
  local imdata = {}
  local refs = {}
  for i = 1, #images do
    local im = images[i]
    local image = vips.Image.new_from_buffer(im["data"])
    local x, y, maxWidth, maxHeight = im["x"], im["y"], math.ceil(im["width"]), math.ceil(im["height"])
    refs[i] = im.ref
    if mode == m.INVERT then
      image = image:invert()
    elseif mode == m.DOWNSCALE then
      image = image:resize(0.5)
    elseif mode == m.FLIP_H then
      image = image:flip("horizontal")
    elseif mode == m.FLIP_V then
      image = image:flip("vertical")
    elseif mode == m.ROTATE_CW then
      image = image:rot("d90")
      maxWidth, maxHeight = maxHeight, maxWidth
    elseif mode == m.ROTATE_CCW then
      image = image:rot("d270")
      maxWidth, maxHeight = maxHeight, maxWidth
    elseif mode == m.TRANSPARENT then
      if image:bands() == 3 then
        image = image:bandjoin(255)       -- add alpha channel
      end
      local white = { 240, 240, 240, -1 } -- pixels are considered white enough, if its r,g,b values are >240
      local transparent = { 0, 0, 0, 0 }
      local filter = image:more(white)    -- make pixels white if they are white enough, and black otherwise
          :bandand()                      -- joins bands with bitwise and into a single band, so values are
      -- 255 (true) for white enough pixels,
      -- 0 (false) otherwise
      image = filter:ifthenelse(transparent, image) -- now white enough pixels become transparent, other are taken from image
    elseif mode == m.TRIM then
      local width = image:width()
      local height = image:height()
      image = image:crop(image:find_trim())
      local newWidth = image:width()   -- now newWidth / width is the factor by which the pixbuf got smaller
      local newHeight = image:height() -- and newHeight / height is the factor by which the pixbuf height got smaller
      maxWidth, maxHeight = maxWidth * newWidth // width, maxHeight * newHeight // height
    end
    table.insert(imdata, {
      data = image:write_to_buffer(".png"),
      x = x,
      y = y,
      maxWidth = maxWidth,
      maxHeight = maxHeight
    })
  end
  app.clearSelection()
  app.addToSelection(refs)
  app.uiAction({ action = "ACTION_DELETE" })
  app.addImages({ images = imdata, allowUndoRedoAction = "grouped" })
end

function Split(mode)
  PADDING = 15 -- margin between image parts

  local ffi = require "ffi"
  local vips = checkVips()
  if not vips then return end

  local vertical = mode == m.SPLIT_V
  local images = app.getImages("selection")
  local imdata = {}
  local refs = {}
  for i = 1, #images do
    local im = images[i]
    local image = vips.Image.new_from_buffer(im["data"])
    local x, y, maxWidth, maxHeight = im["x"], im["y"], math.ceil(im["width"]), math.ceil(im["height"])
    refs[i] = im.ref
    -- find largest sequence of white or transparent pixel rows/columns
    local filter = image:colourspace("b-w") -- convert image to grayscale colour space
        :extract_band(0)                    -- ignore alpha channel if it exists
        :cast("uint")                       -- cast from double to uint data type
        :more(240)                          -- replace value by 255 if it is > 240 (whiteish), 0 otherwise
    local width, height = filter:width(), filter:height()
    local last = vertical and height or width
    local colSums, rowSums = filter:project() -- width x 1 column sums, 1 x height row sums

    -- now cast rowSums (or colSums) into an array for fast access
    local sum = ffi.cast(ffi.typeof("unsigned int*"), vertical and rowSums:write_to_memory() or colSums:write_to_memory())
    local maxSum = vertical and 255 * width or 255 * height
    local whiteEnds = {}
    local whiteLengths = {}
    local firstAfter = false
    local currentLength = 0
    local n = 0
    for j = 1, last do
      if sum[j - 1] == maxSum then -- completely white row (or column)
        if currentLength > 0 then
          currentLength = currentLength + 1
        else
          currentLength = 1
          firstAfter = true
        end
      else
        if firstAfter then
          whiteEnds[n] = j - 1
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

    local maxBlockLength = 0
    local endIndex = 0
    for j = 1, #whiteEnds do
      if whiteLengths[j] > maxBlockLength then
        maxBlockLength = whiteLengths[j]
        endIndex = whiteEnds[j]
      end
    end
    if vertical then
      local height1 = endIndex - maxBlockLength // 2 -- white block is split equally
      local height2 = height - height1
      local image1 = image:extract_area(0, 0, width, height1)
      local image2 = image:extract_area(0, height1, width, height2)
      table.insert(imdata, { data = image1:write_to_buffer(".png"), x = x, y = y, maxWidth = maxWidth })
      table.insert(imdata, {
        data = image2:write_to_buffer(".png"),
        maxWidth = maxWidth,
        x = x,
        y = y + height1 * maxHeight // height + PADDING, -- maxHeight / height scales from pixbuf height to height in document
      })
    else
      local width1 = endIndex - maxBlockLength // 2 -- white block is split equally
      local width2 = width - width1
      local image1 = image:extract_area(0, 0, width1, height)
      local image2 = image:extract_area(width1, 0, width2, height)
      table.insert(imdata, { data = image1:write_to_buffer(".png"), x = x, y = y, maxHeight = maxHeight })
      table.insert(imdata, {
        data = image2:write_to_buffer(".png"),
        maxHeight = maxHeight,
        x = x + width1 * maxWidth // width + PADDING, -- maxWidth / width scales from pixbuf width to width in document
        y = y,
      })
    end
    ::continue::
  end
  app.clearSelection()
  app.addToSelection(refs)
  app.uiAction({ action = "ACTION_DELETE" })
  app.addImages({ images = imdata, allowUndoRedoAction = "grouped" })
end
