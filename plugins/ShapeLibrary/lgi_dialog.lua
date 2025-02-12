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
local assert = lgi.assert
local drawing = require "drawing"
local index = 1 -- the index is set when one of the shapes is selected

function _M.showMainShapeDialog()
    local provider = Gtk.CssProvider()
    assert(provider:load_from_path(sourcePath .. sep .. "main.css"), "ERROR: main.css not found")
    -- local screen = Gdk.Display.get_default_screen(Gdk.Display:get_default())
    -- Gtk.StyleContext.add_provider_for_screen(screen, provider, Gtk.STYLE_PROVIDER_PRIORITY_USER)

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
                    spacing = 24,
                    Gtk.Label {
                        id = 'lbl_category',
                        label = "Category",
                    },
                    Gtk.StackSidebar {
                        height_request = 300,
                        id = 'switcher',
                    },
                    Gtk.Box {
                        orientation = "HORIZONTAL",
                        hexpand = true,
                        halign = "CENTER",
                        spacing = 12,
                        Gtk.Button {
                            id = 'add_category_button',
                            hexpand = false,
                            label = '+',
                            tooltip_text = 'Add new category',
                        },
                        Gtk.Button {
                            id = 'remove_category_button',
                            hexpand = false,
                            label = '-',
                            tooltip_text = 'Remove selected category',
                        },
                    },
                },
            },
            Gtk.Box {
                orientation = "VERTICAL",
                spacing = 12,
                margin_bottom = 24,
                margin_top = 24,
                Gtk.Label {
                    id = 'lbl_shape',
                    label = "Shape",
                },
                Gtk.Stack {
                    id = 'stack',
                    width_request = 400,
                },
                Gtk.Box {
                    orientation = "HORIZONTAL",
                    spacing = 96,
                    margin_bottom = 6,
                    margin_top = 6,
                    margin_left = 6,
                    margin_right = 6,
                    hexpand = true,
                    Gtk.Button {
                        id = 'insert_button',
                        halign = "CENTER",
                        label = "Insert",
                        tooltip_text = "Insert shape into document",
                    },
                    Gtk.Box {
                        orientation = "HORIZONTAL",
                        halign = "END",
                        spacing = 12,
                        Gtk.Button {
                            id = 'update_shape_button',
                            hexpand = false,
                            label = "Update",
                            tooltip_text = "Update selected shape",
                        },
                        Gtk.Button {
                            id = 'add_shape_button',
                            hexpand = false,
                            label = "+",
                            tooltip_text = "Add shape from selection",
                        },
                        Gtk.Button {
                            id = 'remove_shape_button',
                            hexpand = false,
                            label = "-",
                            tooltip_text = "Remove selected shape",
                        },
                    },
                }
            }
        }
    }
    local lbl_category = window.child['lbl_category']
    lbl_category:get_style_context():add_provider(provider, Gtk.STYLE_PROVIDER_PRIORITY_USER)
    local lbl_shape = window.child['lbl_shape']
    lbl_shape:get_style_context():add_provider(provider, Gtk.STYLE_PROVIDER_PRIORITY_USER)

    local insert_button = window.child['insert_button']
    insert_button.on_button_press_event = function(event)
        local category = tonumber(window.child['stack']:get_visible_child_name())
        local shape_name = shapes_dict[category].shapes[index].shapeName
        insertion_helper.insert_stroke(shape_name)
    end

    local add_category_button = window.child['add_category_button']
    add_category_button.on_button_press_event = function(event)
        print("Add category")
    end

    local remove_category_button = window.child['remove_category_button']
    remove_category_button.on_button_press_event = function(event)
        print("Remove category")
    end

    local update_shape_button = window.child['update_shape_button']
    update_shape_button.on_button_press_event = function(event)
        print("Update shape")
    end

    local add_shape_button = window.child['add_shape_button']
    add_shape_button.on_button_press_event = function(event)
        print("Add shape")
    end

    local remove_shape_button = window.child['remove_shape_button']
    remove_shape_button.on_button_press_event = function(event)
        print("Remove shape")
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
