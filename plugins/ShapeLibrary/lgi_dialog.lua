local sep = package.config:sub(1, 1) -- path separator depends on OS
local sourcePath = debug.getinfo(1).source:match("@?(.*" .. sep .. ")")
local shapes_dict = require "config"
local stroke_io = require "stroke_io"
local insertion_helper = require "insertion_helper"

local _M = {}
local hasLgi, lgi = pcall(require, "lgi")
if not hasLgi then
  app.openDialog("You need to have the Lua lgi-module installed and included in your Lua package path in order to use the GUI for migrating font sizes. \n\n", {"OK"}, "", true)
  return
end

local index = 1 -- the index is set when one of the shapes is selecteda
function get_strokes(category, index)
    local shape_name = shapes_dict[category].shapes[index].shapeName
    local filepath = sourcePath .. "Shapes" .. sep .. shape_name .. ".lua"
    return stroke_io.read_strokes_from_file(filepath)
end

function compute_bounds(strokes)
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
    return {x = min_x, width = max_x-min_x, y = min_y, height = max_y-min_y}
end

--lgi module has been found
local Gtk = lgi.require("Gtk", "3.0")
local Gdk = lgi.require("Gdk", "3.0")
function _M.showMainShapeDialog()
    local window = Gtk.Window {
        title = 'Insert shapes',
        default_width = 500,
        default_height = 500,

        Gtk.Box {
            orientation = "VERTICAL",
            spacing = 12,
            Gtk.StackSwitcher {
                id = 'switcher',
            },
            Gtk.Stack {
                id = 'stack',
            },
            Gtk.Button {
                id = 'insert_button',
                hexpand = false,
                halign = "CENTER",
                label = "Insert shape into document",
            },
            Gtk.Box {
                orientation = "HORIZONTAL",
                spacing = 12,
                margin_top = 24,
                halign = "CENTER",

                Gtk.Label {
                    label = "Name: ",
                },
                Gtk.Entry {
                    id = 'add_entry',
                    placeholder_text = "MyShape",
                },
                Gtk.Button {
                    id = 'add_button',
                    label = "Add shape from selection",
                },
            }
        }
    }

    local insert_button = window.child['insert_button']
    insert_button.on_button_press_event = function(event)
        local category = tonumber(window.child['stack']:get_visible_child_name())
        local shape_name = shapes_dict[category].shapes[index].shapeName
        insertion_helper.insert_stroke(shape_name)
    end

    local stack = window.child['stack']
    local switcher = window.child['switcher']
    switcher:set_stack(stack)

    for i, category in ipairs(shapes_dict) do
        local scrolled_window = Gtk.ScrolledWindow {
            margin_bottom = 24,
            margin_top = 24,
            propagate_natural_height = true,
            max_content_height = 400,
            Gtk.FlowBox {
                id = 'flow_box',
                orientation = "HORIZONTAL",
                max_children_per_line = 6,
                min_children_per_line = 3,
                column_spacing=12,
                row_spacing=12,
                homogeneous=true,
                selection_mode="SINGLE",
                on_child_activated = function(_, child) index = child:get_index()+1 end
            }
        }
        local flow_box = scrolled_window.child['flow_box']

        local category = shapes_dict[i]

        for j, shape in ipairs(category.shapes) do
            local shape_box = Gtk.Box {
                orientation = "VERTICAL",
                spacing = 12,
                Gtk.Label {
                    label = shape.name,
                },
                Gtk.Frame {
                    shadow_type = 'IN',
                    expand = true,
                    Gtk.DrawingArea {
                        id = 'drawing_area',
                        width = 100,
                        height = 100,
                    }
                }
            }
            local drawing_area = shape_box.child['drawing_area']
            local strokes = get_strokes(i, j)
            local bounds = compute_bounds(strokes)
            function drawing_area:on_draw(cr)
                cr:save()
                -- transform coordinate system
                local scale_x = self.width / bounds.width
                local scale_y = self.height / bounds.height
                cr:translate(0.1*self.width, 0.1*self.height)
                cr:scale(0.8*scale_x, 0.8*scale_y)
                cr:translate(-bounds.x, -bounds.x)

                -- Draw strokes (fill, pressure, line style, color are ignored so far)
                for _, stroke in ipairs(strokes) do
                    cr:set_source_rgb(0, 1, 0)
                    cr:move_to(stroke.x[1], stroke.y[1])
                    local num = #stroke.x
                    for i = 2,num do
                        cr:line_to(stroke.x[i], stroke.y[i])
                    end
                    cr:stroke()
                end
                cr:restore()

                return true
            end

            flow_box:add(shape_box)
        end

        stack:add_titled (scrolled_window, i, category.name)
    end

    window:show_all()
end

return _M
