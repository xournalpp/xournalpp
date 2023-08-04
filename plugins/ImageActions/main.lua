function initUi()
  app.registerUi({["menu"] = "Invert selected images", ["callback"] = "cb", ["mode"] = 1})
  app.registerUi({["menu"] = "Downscale resolution of selected images by factor 0.5", ["callback"] = "cb", ["mode"] = 2})
  app.registerUi({["menu"] = "Flip selected images horizontally", ["callback"] = "cb", ["mode"] = 3})
  app.registerUi({["menu"] = "Flip selected images vertically", ["callback"] = "cb", ["mode"] = 4})
  app.registerUi({["menu"] = "Rotate selected images 90 degree clockwise", ["callback"] = "cb", ["mode"] = 5})
  app.registerUi({["menu"] = "Rotate selected images 90 degree counterclockwise", ["callback"] = "cb", ["mode"] = 6})
  app.registerUi({["menu"] = "Make white pixels transparent", ["callback"] = "cb", ["mode"] = 7})
end

function cb(mode)
  local images = app.getImages("selection")
  local hasVips, vips = pcall(require, "vips")
  if not hasVips then
    app.msgbox("You need to have lua-vips and luaffifb installed and included in your Lua package path in order to use this plugin. \n" .. 
               "See https://github.com/rolandlo/lua-vips/tree/fix-lua-5.3#installation for installation instructions.\n\n", {[1]="OK"})
    return
  end
  imdata = {}
  for i=1,#images do
    im = images[i]
    image = vips.Image.new_from_buffer(im["data"])
    x, y, maxWidth, maxHeight=im["x"], im["y"], math.ceil(im["width"]), math.ceil(im["height"])
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
      if image:bands()<4 then
        image = image:bandjoin(255) -- add alpha channel
      end
      white = {240, 240, 240, 0} -- pixels are considered white, if its r,g,b values are >=240 and alpha is arbitrary
      transparent = {0, 0, 0, 0}
      image = image:more(white):bandand():ifthenelse(transparent, image)
    end
    table.insert(imdata, {data = image:write_to_buffer(".png"), x=x, y=y, maxWidth=maxWidth, maxHeight=maxHeight})
  end
  app.uiAction({action = "ACTION_DELETE"})
  app.addImages({images=imdata, allowUndoRedoAction = "grouped"})
end
