_M = {}

local function compute_bounds(strokes)
    local min_x, min_y, max_x, max_y = math.huge, math.huge, -math.huge, -math.huge
    for _, stroke in ipairs(strokes) do
        for _, x in ipairs(stroke.x) do
            min_x = math.min(min_x, x)
            max_x = math.max(max_x, x)
        end
        for _, y in ipairs(stroke.y) do
            min_y = math.min(min_y, y)
            max_y = math.max(max_y, y)
        end
    end
    return { x = min_x, width = max_x - min_x, y = min_y, height = max_y - min_y }
end

local function decimal_to_rgb(color)
    local r = (color >> 16) & 0xFF   -- Extract the red component
    local g = (color >> 8) & 0xFF    -- Extract the green component
    local b = color & 0xFF           -- Extract the blue component
    return r / 255, g / 255, b / 255 -- Normalize to 0-1 range
end

local dashes_table = {
    dash = { 10, 5 },
    dot = { 2, 4 },
    dashdot = { 10, 5, 2, 5 },
}
setmetatable(dashes_table, { __index = function(self, index) return {} end }) -- default value
function _M.draw_strokes(strokes)
    local bounds = compute_bounds(strokes)
    return function(self, cr)
        cr:save()
        -- transform coordinate system

        -- Get the dimensions of the drawing area
        local width = self.width_request
        local height = self.height_request

        -- Calculate scaling factors and translations
        local scale_x = width / bounds.width
        local scale_y = height / bounds.height
        local scale = math.min(scale_x, scale_y) * 0.8 -- Scale to fit, with padding

        if scale > 1 then
            scale = 1 + (scale - 1) * 0.2 -- Keep the actual size (slightly larger)
        end

        -- Center the shape in the drawing area
        local offset_x = (width - (bounds.width * scale)) / 2
        local offset_y = (height - (bounds.height * scale)) / 2

        -- Apply transformations
        cr:translate(offset_x, offset_y)
        cr:scale(scale, scale)
        cr:translate(-bounds.x, -bounds.y) -- Translate to the top-left of the shape

        -- Draw the strokes
        for _, stroke in ipairs(strokes) do
            if #stroke.x > 0 then
                local r, g, b = decimal_to_rgb(stroke.color)
                cr:set_source_rgb(r, g, b)

                -- Define line type (dashed/dotted)
                cr:set_dash(dashes_table[stroke.lineStyle], 0)

                -- Adjust line thickness based on the scale factor
                local line_thickness = stroke.width / scale * 1.2 -- 20% thicker for better view

                local has_pressure = stroke.pressure and #stroke.pressure > 0
                if has_pressure then -- breaks the stroke into line segments to simulate variable width for a stroke
                    for k = 2, #stroke.x do
                        cr:move_to(stroke.x[k - 1], stroke.y[k - 1])
                        cr:line_to(stroke.x[k], stroke.y[k])
                        -- Adjust line width dynamically for each segment
                        local line_width = has_pressure and stroke.pressure[k] or line_thickness
                        cr:set_line_width(line_width)
                        cr:stroke() -- Draw the current segment
                    end
                else                -- if no pressure value then width is a usual (if this is not used then fill property won't work for segmented strokes)
                    cr:set_line_width(line_thickness)

                    cr:move_to(stroke.x[1], stroke.y[1])
                    for k = 2, #stroke.x do
                        cr:line_to(stroke.x[k], stroke.y[k])
                    end
                end

                if stroke.fill > 2 then -- Some strokes have fill = 1 though no need, so to avoid this we use 2 here
                    cr:set_source_rgba(r, g, b, stroke.fill / 255)
                    cr:fill_preserve() -- Fill the shape but keep the path for stroking
                end
                cr:stroke()
            end
        end
        cr:restore()
    end
end

return _M
