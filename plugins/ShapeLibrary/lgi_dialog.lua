local sep = package.config:sub(1, 1) -- path separator depends on OS
local sourcePath = debug.getinfo(1).source:match("@?(.*" .. sep .. ")")
local shapes_dict = require "config"
local stroke_io = require "stroke_io"
local insertion_helper = require "insertion_helper"

local _M = {}
local hasLgi, lgi = pcall(require, "lgi")
if not hasLgi then
    app.openDialog(
        "You need to have the Lua lgi-module installed and included in your Lua package path in order to use the GUI for migrating font sizes. \n\n",
        { "OK" }, "", true)
    return
end
local function get_strokes(category, i)
    local shape_name = shapes_dict[category].shapes[i].shapeName
    local filepath = sourcePath .. "Shapes" .. sep .. shape_name .. ".lua"
    return stroke_io.read_strokes_from_file(filepath)
end

--lgi module has been found
local Gtk = lgi.require("Gtk", "3.0")
local drawing = require "drawing"
local index = 1 -- the index is set when one of the shapes is selected

function _M.showMainShapeDialog()
    local window = Gtk.Window {
        title = 'Insert shapes',
        default_width = 600,
        default_height = 500,

        Gtk.Box {
            orientation = "HORIZONTAL",
            Gtk.ScrolledWindow {
                margin_bottom = 24,
                margin_top = 24,
                propagate_natural_width = true,
                Gtk.Box {
                    orientation = "VERTICAL",
                    spacing=24,
                    Gtk.Label {
                        use_markup=true,
                        label = "<b>Category</b>",
                    },
                    Gtk.StackSidebar {
                    height_request = 400,
                    id = 'switcher',
                    }
                },
            },
            Gtk.Box {
                orientation = "VERTICAL",
                spacing = 12,
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
                column_spacing = 12,
                row_spacing = 12,
                homogeneous = true,
                selection_mode = "SINGLE",
                on_child_activated = function(_, child) index = child:get_index() + 1 end
            }
        }
        local flow_box = scrolled_window.child['flow_box']

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
            function drawing_area:on_draw(cr)
                drawing.draw_strokes(strokes)(self, cr)
                return true
            end

            flow_box:add(shape_box)
        end

        stack:add_titled(scrolled_window, i, category.name)
    end

    window:show_all()
end

return _M
