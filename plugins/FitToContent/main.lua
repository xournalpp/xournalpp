-- Register all Toolbar actions and intialize all UI stuff
function initUi()
    app.registerUi{["menu"] = "Fit page to layer", ["callback"] = "fit_to_layer"};
    app.registerUi{["menu"] = "Fit page to selection", ["callback"] = "fit_to_selection"};
end

local function fit_to_layer_or_selection(type)
    if type ~= "selection" and type ~= "layer" then
        app.openDialog("Invalid type for fitting", {"Ok"}, "", true)
        return
    end

    local strokes = app.getStrokes(type)
    -- padd the whole region just a little bit. Might be a thing to configure in the future
    local padding = {x=0, y=0}
    -- search for the outermost strokes which span the rectangle of the to be moved area
    -- take the stroke width, or if set the pressure, into account hereby
    local mima    = {minX=100000, maxX=0, minY=100000, maxY=0}
    for _,stroke in ipairs(strokes) do
        stroke.ref = nil
        if stroke.pressure then
            for i,x in ipairs(stroke.x) do
                mima.maxX = math.max(mima.maxX, x + stroke.pressure[i]/2)
                mima.minX = math.min(mima.minX, x - stroke.pressure[i]/2)
            end
            for i,y in ipairs(stroke.y) do
                mima.maxY = math.max(mima.maxY, y + stroke.pressure[i]/2)
                mima.minY = math.min(mima.minY, y - stroke.pressure[i]/2)
            end
        else
            for _,x in ipairs(stroke.x) do
                mima.maxX = math.max(mima.maxX, x + stroke.width/2)
                mima.minX = math.min(mima.minX, x - stroke.width/2)
            end
            for _,y in ipairs(stroke.y) do
                mima.maxY = math.max(mima.maxY, y + stroke.width/2)
                mima.minY = math.min(mima.minY, y - stroke.width/2)
            end
        end
    end
    -- move strokes by minX and minY
    for _,stroke in ipairs(strokes) do
        for i,x in ipairs(stroke.x) do
            stroke.x[i] = x - mima.minX + padding.x
        end
        for i,y in ipairs(stroke.y) do
            stroke.y[i] = y - mima.minY + padding.y
        end
    end

    -- make the current layer invisible and add the moved strokes to a new layer
    app.setLayerVisibility(false)
    app.layerAction("ACTION_NEW_LAYER")
    app.setCurrentLayerName("moved for fitting")
    app.addStrokes{strokes=strokes}

    app.setPageSize(mima.maxX-mima.minX+2*padding.x, mima.maxY-mima.minY+2*padding.y)
end

function fit_to_layer() fit_to_layer_or_selection("layer") end
function fit_to_selection() fit_to_layer_or_selection("selection") end
