#include "ButtonConfigGui.h"

#include <memory>   // for allocator, allocator_trai...
#include <utility>  // for pair

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GDK_TYPE_PIXBUF, GdkPixbuf
#include <gdk/gdk.h>                // for GdkRGBA
#include <glib-object.h>            // for g_value_get_int, GValue

#include "control/DeviceListHelper.h"       // for InputDevice, getDeviceList
#include "control/settings/ButtonConfig.h"  // for ButtonConfig
#include "control/settings/Settings.h"      // for Settings
#include "gui/Builder.h"
#include "util/Color.h"  // for GdkRGBA_to_argb, rgb_to_G...
#include "util/i18n.h"  // for _

class GladeSearchpath;

constexpr auto UI_FILE = "settingsButtonConfig.ui";
constexpr auto UI_WIDGET_NAME = "mainBox";

ButtonConfigGui::ToolSizeIndexMap ButtonConfigGui::toolSizeIndexMap = {{0, TOOL_SIZE_NONE},  {1, TOOL_SIZE_VERY_FINE},
                                                                       {2, TOOL_SIZE_FINE},  {3, TOOL_SIZE_MEDIUM},
                                                                       {4, TOOL_SIZE_THICK}, {5, TOOL_SIZE_VERY_THICK}};

std::string ButtonConfigGui::toolSizeToLabel(ToolSize size) {
    switch (size) {
        case TOOL_SIZE_NONE:
            return _("Thickness - don't change");
        case TOOL_SIZE_VERY_FINE:
            return _("Very thin");
        case TOOL_SIZE_FINE:
            return _("Thin");
        case TOOL_SIZE_MEDIUM:
            return _("Medium");
        case TOOL_SIZE_THICK:
            return _("Thick");
        case TOOL_SIZE_VERY_THICK:
            return _("Very thick");
        default:
            return "";
    }
}

ButtonConfigGui::ButtonConfigGui(GladeSearchpath* gladeSearchPath, GtkBox* box, Settings* settings, unsigned int button,
                                 bool withDevice):
        settings(settings), button(button), withDevice(withDevice), iconNameHelper(settings) {
    Builder builder(gladeSearchPath, UI_FILE);

    gtk_box_append(box, builder.get(UI_WIDGET_NAME));  // box takes ownership of it all!

    this->cbDevice = builder.get("labelDevice");
    this->cbDisableDrawing = builder.get("cbDisableDrawing");

    if (withDevice) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDevice), _("No device"));

        this->deviceList = DeviceListHelper::getDeviceList(this->settings, true);
        for (InputDevice const& dev: this->deviceList) {
            std::string txt = dev.getName() + " (" + dev.getType() + ")";
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDevice), txt.c_str());
        }
    } else {
        gtk_widget_hide(builder.get("lbDevice"));
        gtk_widget_hide(this->cbDevice);
        gtk_widget_hide(this->cbDisableDrawing);
    }

    GtkListStore* typeModel = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);

    auto addTypeCB = [=](const char* icon, const char* name, ToolType action) {
        GtkTreeIter iter;
        gtk_list_store_append(typeModel, &iter);
        gtk_list_store_set(typeModel, &iter, 0, iconNameHelper.iconName(icon).c_str(), 1, name, 2, action, -1);
    };

    addTypeCB("transparent", _("Tool - don't change"), TOOL_NONE);
    addTypeCB("tool-pencil", _("Pen"), TOOL_PEN);
    addTypeCB("tool-eraser", _("Eraser"), TOOL_ERASER);
    addTypeCB("tool-highlighter", _("Highlighter"), TOOL_HIGHLIGHTER);
    addTypeCB("tool-text", _("Text"), TOOL_TEXT);
    addTypeCB("tool-image", _("Insert image"), TOOL_IMAGE);
    addTypeCB("spacer", _("Vertical space"), TOOL_VERTICAL_SPACE);
    addTypeCB("select-lasso", _("Select region"), TOOL_SELECT_REGION);
    addTypeCB("select-rect", _("Select rectangle"), TOOL_SELECT_RECT);
    addTypeCB("select-multilayer-lasso", _("Select multi layer region"), TOOL_SELECT_MULTILAYER_REGION);
    addTypeCB("select-multilayer-rect", _("Select multi layer rect"), TOOL_SELECT_MULTILAYER_RECT);
    addTypeCB("hand", _("Hand"), TOOL_HAND);
    addTypeCB("floating-toolbox", _("Floating Toolbox (experimental)"), TOOL_FLOATING_TOOLBOX);
    addTypeCB("select-pdf-text-ht", _("Select Text from pdf"), TOOL_SELECT_PDF_TEXT_LINEAR);
    addTypeCB("select-pdf-text-area", _("Select Area Text from pdf"), TOOL_SELECT_PDF_TEXT_RECT);

    this->cbTool = builder.get("cbTool");
    gtk_combo_box_set_model(GTK_COMBO_BOX(this->cbTool), GTK_TREE_MODEL(typeModel));
    g_signal_connect(cbTool, "changed", G_CALLBACK(&cbSelectCallback), this);

    GtkCellRenderer* renderer = gtk_cell_renderer_pixbuf_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(this->cbTool), renderer, false);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(this->cbTool), renderer, "icon-name", 0, nullptr);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(this->cbTool), renderer, true);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(this->cbTool), renderer, "text", 1, nullptr);

    this->cbThickness = builder.get("cbThickness");
    for (auto const& size: toolSizeIndexMap) {
        gtk_combo_box_text_insert_text(GTK_COMBO_BOX_TEXT(cbThickness), size.first,
                                       toolSizeToLabel(size.second).c_str());
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(cbThickness), 0);

    this->colorButton = builder.get("colorButton");

    this->cbDrawingType = builder.get("cbDrawingType");
    // DRAWING_TYPE_DONT_CHANGE
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Drawing Type - don't change"));
    // DRAWING_TYPE_DEFAULT
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Normal drawing"));
    // DRAWING_TYPE_RULER
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Draw Line"));
    // DRAWING_TYPE_RECTANGLE
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Draw Rectangle"));
    // DRAWING_TYPE_ELLIPSE
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Draw Ellipse"));
    // DRAWING_TYPE_ARROW
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Draw Arrow"));
    // DRAWING_TYPE_DOUBLE_ARROW
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Draw Double Arrow"));
    // DRAWING_TYPE_COORDINATE_SYSTEM
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Draw Coordinate System"));
    // DRAWING_TYPE_STROKE_RECOGNIZER
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Shape Recognizer"));
    // DRAWING_TYPE_SPLINE
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Draw Spline"));


    // Values in glade GUI!
    this->cbEraserType = builder.get("cbEraserType");

    // Possible values are defined in the .glade file
    this->cbStrokeType = builder.get("cbStrokeType");

    loadSettings();
}

void ButtonConfigGui::loadSettings() {
    ButtonConfig* cfg = settings->getButtonConfig(button);

    GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(cbTool));
    GtkTreeIter iter;

    if (!gtk_tree_model_get_iter_first(model, &iter)) {
        return;
    }

    GValue value = {0};
    int i = 0;

    gtk_combo_box_set_active(GTK_COMBO_BOX(cbTool), 0);

    do {
        gtk_tree_model_get_value(model, &iter, 2, &value);

        int action = g_value_get_int(&value);

        if (action == cfg->action) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(cbTool), i);
            break;
        }

        i++;
        g_value_unset(&value);
    } while (gtk_tree_model_iter_next(model, &iter));

    // select thickness in combo box
    gtk_combo_box_set_active(GTK_COMBO_BOX(cbThickness), 0);
    for (auto const& size: toolSizeIndexMap) {
        if (size.second == cfg->size) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(cbThickness), size.first);
        }
    }

    GdkRGBA color = Util::rgb_to_GdkRGBA(cfg->color);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(colorButton), &color);

    gtk_combo_box_set_active(GTK_COMBO_BOX(this->cbDrawingType), cfg->drawingType);

    if (cfg->eraserMode == ERASER_TYPE_DEFAULT) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(cbEraserType), 1);
    } else if (cfg->eraserMode == ERASER_TYPE_WHITEOUT) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(cbEraserType), 2);
    } else if (cfg->eraserMode == ERASER_TYPE_DELETE_STROKE) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(cbEraserType), 3);
    } else {
        gtk_combo_box_set_active(GTK_COMBO_BOX(cbEraserType), 0);
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(cbStrokeType), cfg->strokeType);

    if (withDevice) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(cbDevice), 0);

        int i = 1;
        for (InputDevice const& dev: this->deviceList) {
            if (cfg->device == dev.getName()) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(cbDevice), i);
                break;
            }

            ++i;
        }

        gtk_check_button_set_active(GTK_CHECK_BUTTON(cbDisableDrawing), cfg->disableDrawing);
    }
}

void ButtonConfigGui::saveSettings() {
    GtkTreeIter iter;
    gtk_combo_box_get_active_iter(GTK_COMBO_BOX(cbTool), &iter);

    GValue value = {0};
    GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(cbTool));
    gtk_tree_model_get_value(model, &iter, 2, &value);

    auto action = static_cast<ToolType>(g_value_get_int(&value));
    ButtonConfig* cfg = settings->getButtonConfig(button);
    cfg->action = action;

    int thickness = gtk_combo_box_get_active(GTK_COMBO_BOX(cbThickness));
    if (toolSizeIndexMap.count(thickness) == 0) {
        cfg->size = TOOL_SIZE_NONE;
    } else {
        cfg->size = toolSizeIndexMap[thickness];
    }

    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colorButton), &color);
    cfg->color = Util::GdkRGBA_to_argb(color);

    cfg->drawingType = static_cast<DrawingType>(gtk_combo_box_get_active(GTK_COMBO_BOX(this->cbDrawingType)));

    int eraserMode = gtk_combo_box_get_active(GTK_COMBO_BOX(this->cbEraserType));

    if (eraserMode == 1) {
        cfg->eraserMode = ERASER_TYPE_DEFAULT;
    } else if (eraserMode == 2) {
        cfg->eraserMode = ERASER_TYPE_WHITEOUT;
    } else if (eraserMode == 3) {
        cfg->eraserMode = ERASER_TYPE_DELETE_STROKE;
    } else {
        cfg->eraserMode = ERASER_TYPE_NONE;
    }

    cfg->strokeType = static_cast<StrokeType>(gtk_combo_box_get_active(GTK_COMBO_BOX(this->cbStrokeType)));

    if (this->withDevice) {
        size_t dev = static_cast<size_t>(gtk_combo_box_get_active(GTK_COMBO_BOX(cbDevice)) - 1);
        cfg->device = (dev < 0 || this->deviceList.size() <= dev) ? "" : this->deviceList[dev].getName();
        cfg->disableDrawing = gtk_check_button_get_active(GTK_CHECK_BUTTON(cbDisableDrawing));
    }

    settings->customSettingsChanged();
}

void ButtonConfigGui::cbSelectCallback(GtkComboBox*, ButtonConfigGui* gui) { gui->enableDisableTools(); }

void ButtonConfigGui::enableDisableTools() {
    GtkTreeIter iter;
    gtk_combo_box_get_active_iter(GTK_COMBO_BOX(cbTool), &iter);
    GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(cbTool));

    GValue value = {0};
    gtk_tree_model_get_value(model, &iter, 2, &value);
    auto action = static_cast<ToolType>(g_value_get_int(&value));

    switch (action) {
        case TOOL_PEN:
            gtk_widget_set_visible(cbThickness, true);
            gtk_widget_set_visible(colorButton, true);
            gtk_widget_set_visible(cbDrawingType, true);
            gtk_widget_set_visible(cbEraserType, false);
            gtk_widget_set_visible(cbStrokeType, true);
            break;
        case TOOL_HIGHLIGHTER:
        case TOOL_SELECT_PDF_TEXT_LINEAR:
        case TOOL_SELECT_PDF_TEXT_RECT:
            gtk_widget_set_visible(cbThickness, true);
            gtk_widget_set_visible(colorButton, true);
            gtk_widget_set_visible(cbDrawingType, true);
            gtk_widget_set_visible(cbEraserType, false);
            gtk_widget_set_visible(cbStrokeType, false);
            break;

        case TOOL_ERASER:
            gtk_widget_set_visible(cbThickness, true);
            gtk_widget_set_visible(colorButton, false);
            gtk_widget_set_visible(cbDrawingType, false);
            gtk_widget_set_visible(cbEraserType, true);
            gtk_widget_set_visible(cbStrokeType, false);
            break;

        case TOOL_TEXT:
            gtk_widget_set_visible(cbThickness, false);
            gtk_widget_set_visible(colorButton, true);
            gtk_widget_set_visible(cbDrawingType, false);
            gtk_widget_set_visible(cbEraserType, false);
            gtk_widget_set_visible(cbStrokeType, false);
            break;

        case TOOL_NONE:
        case TOOL_IMAGE:
            // case TOOL_DRAW_RECT:
            // case TOOL_DRAW_ELLIPSE:
        case TOOL_SELECT_RECT:
        case TOOL_SELECT_REGION:
        case TOOL_SELECT_MULTILAYER_RECT:
        case TOOL_SELECT_MULTILAYER_REGION:
        case TOOL_VERTICAL_SPACE:
        case TOOL_HAND:
            gtk_widget_set_visible(cbThickness, false);
            gtk_widget_set_visible(colorButton, false);
            gtk_widget_set_visible(cbDrawingType, false);
            gtk_widget_set_visible(cbEraserType, false);
            gtk_widget_set_visible(cbStrokeType, false);
            break;
        default:
            break;
    }
}
