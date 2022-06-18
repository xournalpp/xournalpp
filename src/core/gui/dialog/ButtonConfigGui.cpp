#include "ButtonConfigGui.h"

#include <memory>   // for allocator, allocator_trai...
#include <utility>  // for pair

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GDK_TYPE_PIXBUF, GdkPixbuf
#include <gdk/gdk.h>                // for GdkRGBA
#include <glib-object.h>            // for g_value_get_int, GValue

#include "control/DeviceListHelper.h"       // for InputDevice, getDeviceList
#include "control/settings/ButtonConfig.h"  // for ButtonConfig
#include "control/settings/Settings.h"      // for Settings
#include "util/Color.h"                     // for GdkRGBA_to_argb, rgb_to_G...
#include "util/i18n.h"                      // for _

class GladeSearchpath;

ButtonConfigGui::ToolSizeIndexMap ButtonConfigGui::toolSizeIndexMap = {{0, TOOL_SIZE_NONE},  {1, TOOL_SIZE_VERY_FINE},
                                                                       {2, TOOL_SIZE_FINE},  {3, TOOL_SIZE_MEDIUM},
                                                                       {4, TOOL_SIZE_THICK}, {5, TOOL_SIZE_VERY_THICK}};

std::string ButtonConfigGui::toolSizeToLabel(ToolSize size) {
    switch (size) {
        case TOOL_SIZE_NONE:
            return "Thickness - don't change";
        case TOOL_SIZE_VERY_FINE:
            return "Very thin";
        case TOOL_SIZE_FINE:
            return "Thin";
        case TOOL_SIZE_MEDIUM:
            return "Medium";
        case TOOL_SIZE_THICK:
            return "Thick";
        case TOOL_SIZE_VERY_THICK:
            return "Very thick";
        default:
            return "";
    }
}

void addToolToList(GtkListStore* typeModel, const char* icon, const char* name, ToolType action) {
    GtkTreeIter iter;

    gtk_list_store_append(typeModel, &iter);
    GdkPixbuf* pixbuf = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), icon, 24,
                                                 static_cast<GtkIconLookupFlags>(0), nullptr);
    gtk_list_store_set(typeModel, &iter, 0, pixbuf, -1);
    gtk_list_store_set(typeModel, &iter, 1, name, 2, action, -1);
}

ButtonConfigGui::ButtonConfigGui(GladeSearchpath* gladeSearchPath, GtkWidget* w, Settings* settings, int button,
                                 bool withDevice):
        GladeGui(gladeSearchPath, "settingsButtonConfig.glade", "offscreenwindow"),
        settings(settings),
        button(button),
        withDevice(withDevice),
        iconNameHelper(settings) {

    GtkWidget* mainGrid = get("mainGrid");
    gtk_container_remove(GTK_CONTAINER(getWindow()), mainGrid);
    gtk_container_add(GTK_CONTAINER(w), mainGrid);
    gtk_widget_show_all(mainGrid);

    this->cbDevice = get("labelDevice");
    this->cbDisableDrawing = get("cbDisableDrawing");

    if (withDevice) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDevice), _("No device"));

        this->deviceList = DeviceListHelper::getDeviceList(this->settings, true);
        for (InputDevice const& dev: this->deviceList) {
            std::string txt = dev.getName() + " (" + dev.getType() + ")";
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDevice), txt.c_str());
        }
    } else {
        gtk_widget_hide(get("lbDevice"));
        gtk_widget_hide(this->cbDevice);
        gtk_widget_hide(this->cbDisableDrawing);
    }

    GtkListStore* typeModel = gtk_list_store_new(3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_INT);  // NOLINT

    auto addTypeCB = [=](const char* icon, const char* name, ToolType action) {
        addToolToList(typeModel, iconNameHelper.iconName(icon).c_str(), name, action);
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
    addTypeCB("hand", _("Hand"), TOOL_HAND);
    addTypeCB("floating-toolbox", _("Floating Toolbox (experimental)"), TOOL_FLOATING_TOOLBOX);
    addTypeCB("select-pdf-text-head-tail", _("Select Text from pdf"), TOOL_SELECT_PDF_TEXT_LINEAR);
    addTypeCB("select-pdf-text-area", _("Select Area Text from pdf"), TOOL_SELECT_PDF_TEXT_RECT);

    this->cbTool = get("cbTool");
    gtk_combo_box_set_model(GTK_COMBO_BOX(this->cbTool), GTK_TREE_MODEL(typeModel));
    g_signal_connect(cbTool, "changed", G_CALLBACK(&cbSelectCallback), this);

    GtkCellRenderer* renderer = gtk_cell_renderer_pixbuf_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(this->cbTool), renderer, false);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(this->cbTool), renderer, "pixbuf", 0, nullptr);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(this->cbTool), renderer, true);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(this->cbTool), renderer, "text", 1, nullptr);

    this->cbThickness = get("cbThickness");
    for (auto const& size: toolSizeIndexMap) {
        gtk_combo_box_text_insert_text(GTK_COMBO_BOX_TEXT(cbThickness), size.first,
                                       toolSizeToLabel(size.second).c_str());
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(cbThickness), 0);

    this->colorButton = get("colorButton");

    this->cbDrawingType = get("cbDrawingType");
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
    // DRAWING_TYPE_COORDINATE_SYSTEM
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Draw coordinate system"));
    // DRAWING_TYPE_STROKE_RECOGNIZER
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Stroke recognizer"));
    // DRAWING_TYPE_SPLINE
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(this->cbDrawingType), _("Draw Spline"));


    // Values in glade GUI!
    this->cbEraserType = get("cbEraserType");

    loadSettings();
}

ButtonConfigGui::~ButtonConfigGui() = default;

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

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbDisableDrawing), cfg->disableDrawing);
    }
}

void ButtonConfigGui::show(GtkWindow*) {
    // Not implemented! This is not a dialog!
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

    if (this->withDevice) {
        int dev = gtk_combo_box_get_active(GTK_COMBO_BOX(cbDevice)) - 1;
        cfg->device = (dev < 0 || this->deviceList.size() <= dev) ? "" : this->deviceList[dev].getName();
        cfg->disableDrawing = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbDisableDrawing));
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
        case TOOL_HIGHLIGHTER:
        case TOOL_SELECT_PDF_TEXT_LINEAR:
        case TOOL_SELECT_PDF_TEXT_RECT:
            gtk_widget_set_visible(cbThickness, true);
            gtk_widget_set_visible(colorButton, true);
            gtk_widget_set_visible(cbDrawingType, true);
            gtk_widget_set_visible(cbEraserType, false);
            break;

        case TOOL_ERASER:
            gtk_widget_set_visible(cbThickness, true);
            gtk_widget_set_visible(colorButton, false);
            gtk_widget_set_visible(cbDrawingType, false);
            gtk_widget_set_visible(cbEraserType, true);
            break;

        case TOOL_TEXT:
            gtk_widget_set_visible(cbThickness, false);
            gtk_widget_set_visible(colorButton, true);
            gtk_widget_set_visible(cbDrawingType, false);
            gtk_widget_set_visible(cbEraserType, false);
            break;

        case TOOL_NONE:
        case TOOL_IMAGE:
            // case TOOL_DRAW_RECT:
            // case TOOL_DRAW_ELLIPSE:
        case TOOL_SELECT_RECT:
        case TOOL_SELECT_REGION:
        case TOOL_VERTICAL_SPACE:
        case TOOL_HAND:
            gtk_widget_set_visible(cbThickness, false);
            gtk_widget_set_visible(colorButton, false);
            gtk_widget_set_visible(cbDrawingType, false);
            gtk_widget_set_visible(cbEraserType, false);
            break;
        default:
            break;
    }
}
