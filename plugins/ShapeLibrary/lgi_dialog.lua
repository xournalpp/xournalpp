local _M = {}

local Mode = {
    SYSTEM = true,
    USER = false
}

local sep = package.config:sub(1, 1) -- path separator depends on OS
local sourcePath = debug.getinfo(1).source:match('@?(.*' .. sep .. ')')
local stroke_io = require('stroke_io')
local insertion_helper = require('insertion_helper')
local config_helper = require('config_helper')

local system_shapes_dict = config_helper.getShapesData(Mode.SYSTEM) or {}
local user_shapes_dict = config_helper.getShapesData(Mode.USER) or {}

local hasLgi, lgi = pcall(require, 'lgi')
if not hasLgi then
    app.openDialog(
        'You need to have the Lua lgi-module installed and included in your Lua package path for using this plugin. \n\n',
        { 'OK' }, '', true)
    return
end

--lgi module has been found
local Gtk = lgi.require('Gtk', '3.0')
local assert = lgi.assert
local drawing = require('drawing')
local index = 1 -- the index is set when one of the shapes is selected

local function getStrokes(category, i, systemMode)
    local shapes_dict = systemMode and system_shapes_dict or user_shapes_dict
    local filename = shapes_dict[category].shapes[i].filename
    local filepath = systemMode and sourcePath .. 'Shapes' .. sep .. filename or app.getFolder("data") .. sep .. filename
    return stroke_io.readStrokesFromFile(filepath)
end

local function loadShapes(window, systemMode)

    local dict = systemMode and system_shapes_dict or user_shapes_dict
    if not systemMode then
        for i, page in ipairs(window.child.stack:get_children()) do
            if i > #system_shapes_dict then
                page:destroy()
            end
        end
    end

    for i, category in ipairs(dict) do
        local scrolled_window = Gtk.ScrolledWindow {
            margin_bottom = 24,
            margin_top = 24,
            propagate_natural_height = true,
            max_content_height = 450,
            Gtk.FlowBox {
                id = 'flow_box',
                orientation = 'HORIZONTAL',
                max_children_per_line = 6,
                min_children_per_line = 3,
                column_spacing = 12,
                row_spacing = 12,
                homogeneous = true,
                selection_mode = 'SINGLE',
                on_child_activated = function(_, child) index = child:get_index() + 1 end
            }
        }
        for j, shape in ipairs(category.shapes) do
            local shape_box = Gtk.Box {
                orientation = 'VERTICAL',
                spacing = 12,
                Gtk.Label {
                    label = shape.name,
                },
                Gtk.Frame {
                    shadow_type = 'IN',
                    expand = true,
                    Gtk.DrawingArea {
                        id = 'drawing_area',
                        width = 120,
                        height = 100,
                    }
                }
            }
            local strokes = getStrokes(i, j, systemMode)
            function shape_box.child.drawing_area:on_draw(cr)
                drawing.drawStrokes(strokes)(self, cr)
                return true
            end

            scrolled_window.child.flow_box:add(shape_box)
        end
        local ind = systemMode and i or i + #system_shapes_dict
        local name = systemMode and category.name or category.name .. " (*)"
        window.child.stack:add_titled(scrolled_window, ind, name)
    end
    window:show_all()
end

function _M.showMainShapeDialog()
    local provider = Gtk.CssProvider()
    assert(provider:load_from_path(sourcePath .. sep .. 'main.css'), 'ERROR: main.css not found')
    -- local screen = Gdk.Display.get_default_screen(Gdk.Display:get_default())
    -- Gtk.StyleContext.add_provider_for_screen(screen, provider, Gtk.STYLE_PROVIDER_PRIORITY_USER)

    local window = Gtk.Window {
        title = 'Manage and Insert Shapes',
        default_width = 700,
        default_height = 500,

        Gtk.Box {
            orientation = 'HORIZONTAL',
            Gtk.Box {
                orientation = 'VERTICAL',
                spacing = 24,
                margin_bottom = 24,
                margin_top = 24,
                Gtk.Label {
                    id = 'lbl_category',
                    label = 'Category',
                },
                Gtk.ScrolledWindow {
                    margin_bottom = 24,
                    margin_top = 24,
                    propagate_natural_width = true,
                    height_request = 300,
                    Gtk.StackSidebar {
                        height_request = 300,
                        id = 'switcher',
                    },
                },
                Gtk.Box {
                    orientation = 'VERTICAL',
                    spacing = 12,
                    Gtk.Box {
                        orientation = 'HORIZONTAL',
                        hexpand = true,
                        halign = 'CENTER',
                        spacing = 12,
                        Gtk.CheckButton {
                            id = 'system_check_button',
                            label = 'System cat.',
                            active = true,
                        },
                        Gtk.CheckButton {
                            id = 'user_check_button',
                            label = 'User cat.',
                            active = true,
                        },
                    },
                    Gtk.Box {
                        orientation = 'HORIZONTAL',
                        halign = 'CENTER',
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
                        Gtk.Button {
                            id = 'rename_category_button',
                            hexpand = false,
                            label = 'Rename',
                            tooltip_text = 'Rename selected category',
                        },
                    },
                },

            },
            Gtk.Box {
                orientation = 'VERTICAL',
                spacing = 12,
                margin_bottom = 24,
                margin_top = 24,
                Gtk.Label {
                    id = 'lbl_shape',
                    label = 'Shape',
                },
                Gtk.Stack {
                    id = 'stack',
                    width_request = 400,
                },
                Gtk.Box {
                    orientation = 'HORIZONTAL',
                    spacing = 12,
                    margin_bottom = 6,
                    margin_top = 6,
                    margin_left = 6,
                    margin_right = 6,
                    hexpand = true,
                    halign = 'CENTER',
                    Gtk.Button {
                        id = 'insert_button',
                        halign = 'CENTER',
                        label = 'Insert',
                        tooltip_text = 'Insert shape into document',
                    },
                    Gtk.Button {
                        id = 'replace_shape_button',
                        hexpand = false,
                        label = 'Replace',
                        tooltip_text = 'Replace selected shape',
                    },
                    Gtk.Button {
                        id = 'add_shape_button',
                        hexpand = false,
                        label = '+',
                        tooltip_text = 'Add shape from selection',
                    },
                    Gtk.Button {
                        id = 'remove_shape_button',
                        hexpand = false,
                        label = '-',
                        tooltip_text = 'Remove selected shape',
                    },
                    Gtk.Button {
                        id = 'rename_shape_button',
                        hexpand = false,
                        label = 'Rename',
                        tooltip_text = 'Rename selected shape'
                    }
                },
            }
        }
    }

    window:set_position(Gtk.WindowPosition.CENTER)
    function window.child.stack:on_notify(param)
        if param:get_name() == 'visible-child-name' then
            local category = tonumber(self:get_visible_child_name())
            local userCat = category > #system_shapes_dict
            window.child.remove_category_button:set_sensitive(userCat)
            window.child.rename_category_button:set_sensitive(userCat)
            window.child.add_shape_button:set_sensitive(userCat)
            window.child.remove_shape_button:set_sensitive(userCat)
            window.child.replace_shape_button:set_sensitive(userCat)
            window.child.rename_shape_button:set_sensitive(userCat)
        end
    end

    function window.child.system_check_button:on_toggled()
        for i=1, #system_shapes_dict do
            local page = window.child.stack:get_child_by_name(tostring(i))
            page:set_visible(self:get_active())
        end
    end

    function window.child.user_check_button:on_toggled()
        for i=1, #user_shapes_dict do
            local page = window.child.stack:get_child_by_name(tostring(i + #system_shapes_dict))
            page:set_visible(self:get_active())
        end
    end

    window.child.lbl_category:get_style_context():add_provider(provider, Gtk.STYLE_PROVIDER_PRIORITY_USER)
    window.child.lbl_shape:get_style_context():add_provider(provider, Gtk.STYLE_PROVIDER_PRIORITY_USER)
    window.child.insert_button.on_button_press_event = function(event)
        local category = tonumber(window.child.stack:get_visible_child_name())
        local systemMode = category <= #system_shapes_dict
        local shapes_dict = systemMode and system_shapes_dict or user_shapes_dict
        if not systemMode then
            category = category - #system_shapes_dict
        end
        if not shapes_dict[category].shapes[index] then
            print('Select shape first')
            return
        end
        local filename = shapes_dict[category].shapes[index].filename
        insertion_helper.insertStroke(filename, systemMode)
    end

    window.child.switcher:set_stack(window.child.stack)
    loadShapes(window, true) -- system shapes
    loadShapes(window, false) -- user shapes

    -- Dialogs --
    window.child.add_category_button.on_button_press_event = function(event)
        local dialog = Gtk.Dialog {
            title = 'Add Category',
            transient_for = window,
            modal = true,
            destroy_with_parent = true,
            buttons = {
                { Gtk.STOCK_OK, Gtk.ResponseType.OK },
                { '_Cancel',    Gtk.ResponseType.CANCEL },
            },
        }
        local hbox = Gtk.Box {
            orientation = 'HORIZONTAL',
            spacing = 8,
            border_width = 8,
            Gtk.Label {
                label = 'Category name',
            },
            Gtk.Entry {
                id = 'name_entry',
                placeholder_text = 'MyCategory',
            },

        }
        dialog:get_content_area():add(hbox)
        hbox:show_all()

        function dialog:on_response(response)
            if response == Gtk.ResponseType.OK then
                config_helper.addCategory(hbox.child.name_entry.text)
                user_shapes_dict = config_helper.getShapesData(Mode.USER)
                loadShapes(window, false)
                window.child.stack:set_visible_child_name(tostring(#user_shapes_dict) + #system_shapes_dict)
            end
            dialog:destroy()
        end

        dialog:show()
    end
    window.child.remove_category_button.on_button_press_event = function(event)
        local category = tonumber(window.child.stack:get_visible_child_name()) - #system_shapes_dict
        local category_name = user_shapes_dict[category].name

        local dialog = Gtk.Dialog {
            width_request = 300,
            title = 'Remove Category',
            transient_for = window,
            modal = true,
            destroy_with_parent = true,
            buttons = {
                { Gtk.STOCK_OK, Gtk.ResponseType.OK },
                { '_Cancel',    Gtk.ResponseType.CANCEL },
            },
        }
        local vbox = Gtk.Box {
            orientation = 'VERTICAL',
            Gtk.Label {
                use_markup = true,
                wrap = true,
                label = 'Remove category <b>' .. category_name .. '</b> and all shapes in it.'
            },
        }
        dialog:get_content_area():add(vbox)
        vbox:show_all()

        function dialog:on_response(response)
            if response == Gtk.ResponseType.OK then
                config_helper.removeCategory(category_name)
                user_shapes_dict = config_helper.getShapesData(Mode.USER)
                loadShapes(window, false)
            end
            dialog:destroy()
        end

        dialog:show()
    end

    window.child.rename_category_button.on_button_press_event = function(event)
        local category = tonumber(window.child.stack:get_visible_child_name()) - #system_shapes_dict
        local category_name = user_shapes_dict[category].name

        local dialog = Gtk.Dialog {
            width_request = 300,
            title = 'Rename Category',
            transient_for = window,
            modal = true,
            destroy_with_parent = true,
            buttons = {
                { Gtk.STOCK_OK, Gtk.ResponseType.OK },
                { '_Cancel',    Gtk.ResponseType.CANCEL },
            },
        }
        local vbox = Gtk.Box {
            orientation = 'VERTICAL',
            Gtk.Label {
                use_markup = true,
                wrap = true,
                label = 'Rename category <b>' .. category_name .. '</b>'
            },
            Gtk.Box {
                Gtk.Label {
                    label = 'New category name',
                },
                Gtk.Entry {
                    id = 'new_category_name_entry',
                    placeholder_text = 'My New Category',
                }
            },
        }
        dialog:get_content_area():add(vbox)
        vbox:show_all()

        function dialog:on_response(response)
            if response == Gtk.ResponseType.OK then
                config_helper.renameCategory(category_name, vbox.child.new_category_name_entry.text)
                user_shapes_dict = config_helper.getShapesData(Mode.USER)
                loadShapes(window, false)
                window.child.stack:set_visible_child_name(tostring(category) + #system_shapes_dict)
            end
            dialog:destroy()
        end

        dialog:show()
    end

    window.child.replace_shape_button.on_button_press_event = function(event)
        local category = tonumber(window.child.stack:get_visible_child_name()) - #system_shapes_dict
        local category_name = user_shapes_dict[category].name
        local name = user_shapes_dict[category].shapes[index].name
        local filename = user_shapes_dict[category].shapes[index].filename

        local dialog = Gtk.Dialog {
            width_request = 300,
            title = 'Replace Shape',
            transient_for = window,
            modal = true,
            destroy_with_parent = true,
            buttons = {
                { Gtk.STOCK_OK, Gtk.ResponseType.OK },
                { '_Cancel',    Gtk.ResponseType.CANCEL },
            },
        }
        local vbox = Gtk.Box {
            orientation = 'VERTICAL',
            Gtk.Label {
                use_markup = true,
                wrap = true,
                label = 'Replace shape <b>' .. name .. '</b> by the strokes in the selection.'
            },
        }
        dialog:get_content_area():add(vbox)
        vbox:show_all()

        function dialog:on_response(response)
            if response == Gtk.ResponseType.OK then
                local strokes = app.getStrokes('selection')
                local filePath = app.getFolder("data") .. sep .. filename
                stroke_io.storeStrokeInfoInFile(strokes, filePath)
                loadShapes(window, false)
                window.child.stack:set_visible_child_name(tostring(category) + #system_shapes_dict)
            end
            dialog:destroy()
        end

        dialog:show()
    end

    window.child.add_shape_button.on_button_press_event = function(event)
        local category = tonumber(window.child.stack:get_visible_child_name()) - #system_shapes_dict
        local category_name = user_shapes_dict[category].name
        local dialog = Gtk.Dialog {
            title = 'Add Shape',
            transient_for = window,
            modal = true,
            destroy_with_parent = true,
            buttons = {
                { Gtk.STOCK_OK, Gtk.ResponseType.OK },
                { '_Cancel',    Gtk.ResponseType.CANCEL },
            },
        }
        local vbox = Gtk.Box {
            orientation = 'VERTICAL',
            spacing = 8,
            Gtk.Label {
                label = 'Insert selected strokes as new shape',
            },
            Gtk.Grid {
                orientation = 'HORIZONTAL',
                border_width = 8,
                { left_attach = 0, top_attach = 0,
                    Gtk.Label {
                        label = 'Shape name',
                    },
                },
                { left_attach = 1, top_attach = 0,
                    Gtk.Entry {
                        id = 'name_entry',
                        placeholder_text = 'MyShape',
                    },
                },
                { left_attach = 0, top_attach = 1,
                    Gtk.Label {
                        label = 'File name',
                    },
                },
                { left_attach = 1, top_attach = 1,
                    Gtk.Entry {
                        id = 'filename_entry',
                        placeholder_text = 'myshape.lua',
                    },
                }
            },
        }
        dialog:get_content_area():add(vbox)
        vbox:show_all()

        function dialog:on_response(response)
            if response == Gtk.ResponseType.OK then
                local strokes = app.getStrokes('selection')
                local name = vbox.child.name_entry.text
                local filename = vbox.child.filename_entry.text
                local filepath = app.getFolder("data") .. sep .. filename
                stroke_io.storeStrokeInfoInFile(strokes, filepath)
                config_helper.addShape(category_name, name, filename)
                user_shapes_dict = config_helper.getShapesData(Mode.USER)
                loadShapes(window, false)
                window.child.stack:set_visible_child_name(tostring(category) + #system_shapes_dict)
            end
            dialog:destroy()
        end

        dialog:show()
    end

    window.child.remove_shape_button.on_button_press_event = function(event)
        local category = tonumber(window.child.stack:get_visible_child_name()) - #system_shapes_dict
        local name = user_shapes_dict[category].shapes[index].name
        local category_name = user_shapes_dict[category].name

        local dialog = Gtk.Dialog {
            width_request = 300,
            title = 'Remove Shape',
            transient_for = window,
            modal = true,
            destroy_with_parent = true,
            buttons = {
                { Gtk.STOCK_OK, Gtk.ResponseType.OK },
                { '_Cancel',    Gtk.ResponseType.CANCEL },
            },
        }
        local vbox = Gtk.Box {
            orientation = 'VERTICAL',
            Gtk.Label {
                use_markup = true,
                wrap = true,
                label = 'Remove shape <b>' .. name .. '</b>'
            },
        }
        dialog:get_content_area():add(vbox)
        vbox:show_all()

        function dialog:on_response(response)
            if response == Gtk.ResponseType.OK then
                config_helper.removeShape(category_name, name)
                user_shapes_dict = config_helper.getShapesData(Mode.USER)
                loadShapes(window, false)
                window.child.stack:set_visible_child_name(tostring(category) + #system_shapes_dict)
            end
            dialog:destroy()
        end

        dialog:show()
    end

    window.child.rename_shape_button.on_button_press_event = function(event)
        local category = tonumber(window.child.stack:get_visible_child_name()) - #system_shapes_dict
        local category_name = user_shapes_dict[category].name
        local name = user_shapes_dict[category].shapes[index].name
        local filename = user_shapes_dict[category].shapes[index].filename

        local dialog = Gtk.Dialog {
            width_request = 300,
            title = 'Rename Shape',
            transient_for = window,
            modal = true,
            destroy_with_parent = true,
            buttons = {
                { Gtk.STOCK_OK, Gtk.ResponseType.OK },
                { '_Cancel',    Gtk.ResponseType.CANCEL },
            },
        }
        local vbox = Gtk.Box {
            orientation = 'VERTICAL',
            Gtk.Label {
                use_markup = true,
                wrap = true,
                label = 'Rename shape <b>' .. name .. '</b>'
            },
            Gtk.Box {
                Gtk.Label {
                    label = 'New shape name',
                },
                Gtk.Entry {
                    id = 'new_shape_name_entry',
                    placeholder_text = 'My New Shape',
                }
            },
        }
        dialog:get_content_area():add(vbox)
        vbox:show_all()

        function dialog:on_response(response)
            if response == Gtk.ResponseType.OK then
                config_helper.renameShape(category_name, name, vbox.child.new_shape_name_entry.text, filename, filename)
                user_shapes_dict = config_helper.getShapesData(Mode.USER)
                loadShapes(window, false)
                window.child.stack:set_visible_child_name(tostring(category) + #system_shapes_dict)
            end
            dialog:destroy()
        end

        dialog:show()
    end
end

return _M
