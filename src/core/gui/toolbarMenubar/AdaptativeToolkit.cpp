#include "AdaptativeToolkit.h"

#include <array>
#include <vector>

#include <control/settings/Settings.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "control/Control.h"
#include "control/ToolEnums.h"
#include "control/actions/ActionDatabase.h"
#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "util/glib_casts.h"
#include "util/gtk4_helper.h"
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr

#include "ColorToolItem.h"
#include "ToolMenuHandler.h"

static constexpr const char* ICON_NAME = "xopp-adaptative-toolkit";
static constexpr const char* ALL_COLORS_ID = "ALL_COLORS";  ///< Adds all the colors from the palette

static auto populateElements() {
    std::array<std::vector<const char*>, TOOL_COUNT> els;
    els[TOOL_PEN - TOOL_PEN] = {"PEN",    "DRAW",  "TOOL_FILL",  "FILL_OPACITY", "SEPARATOR",   "VERY_FINE",   "FINE",
                                "MEDIUM", "THICK", "VERY_THICK", "SEPARATOR",    ALL_COLORS_ID, "COLOR_SELECT"};
    els[TOOL_ERASER - TOOL_PEN] = {"ERASER", "SEPARATOR", "VERY_FINE", "FINE", "MEDIUM", "THICK", "VERY_THICK"};
    els[TOOL_HIGHLIGHTER - TOOL_PEN] = {"DRAW",       "TOOL_FILL", "FILL_OPACITY", "SEPARATOR",
                                        "VERY_FINE",  "FINE",      "MEDIUM",       "THICK",
                                        "VERY_THICK", "SEPARATOR", ALL_COLORS_ID,  "COLOR_SELECT"};
    els[TOOL_TEXT - TOOL_PEN] = {"CUT",         "COPY",      "PASTE",       "DELETE",      "SEPARATOR",
                                 "SELECT_FONT", "SEPARATOR", ALL_COLORS_ID, "COLOR_SELECT"};
    els[TOOL_IMAGE - TOOL_PEN] = {};
    els[TOOL_SELECT_RECT - TOOL_PEN] = {"CUT", "COPY", "PASTE", "DELETE"};
    els[TOOL_SELECT_REGION - TOOL_PEN] = {"CUT", "COPY", "PASTE", "DELETE"};
    els[TOOL_SELECT_MULTILAYER_RECT - TOOL_PEN] = {"CUT", "COPY", "PASTE", "DELETE"};
    els[TOOL_SELECT_MULTILAYER_REGION - TOOL_PEN] = {"CUT", "COPY", "PASTE", "DELETE"};
    els[TOOL_SELECT_OBJECT - TOOL_PEN] = {"CUT", "COPY", "PASTE", "DELETE"};
    els[TOOL_PLAY_OBJECT - TOOL_PEN] = {};
    els[TOOL_VERTICAL_SPACE - TOOL_PEN] = {};
    els[TOOL_HAND - TOOL_PEN] = {"ZOOM_IN", "ZOOM_FIT", "ZOOM_OUT", "ZOOM_100"};  //  TODO add navigation?
    els[TOOL_DRAW_RECT - TOOL_PEN] = {};
    els[TOOL_DRAW_ELLIPSE - TOOL_PEN] = {};
    els[TOOL_DRAW_ARROW - TOOL_PEN] = {};
    els[TOOL_DRAW_DOUBLE_ARROW - TOOL_PEN] = {};
    els[TOOL_DRAW_COORDINATE_SYSTEM - TOOL_PEN] = {};
    els[TOOL_FLOATING_TOOLBOX - TOOL_PEN] = {};
    els[TOOL_DRAW_SPLINE - TOOL_PEN] = {};
    els[TOOL_SELECT_PDF_TEXT_LINEAR - TOOL_PEN] = {ALL_COLORS_ID, "COLOR_SELECT"};
    els[TOOL_SELECT_PDF_TEXT_RECT - TOOL_PEN] = {ALL_COLORS_ID, "COLOR_SELECT"};
    els[TOOL_LASER_POINTER_PEN - TOOL_PEN] = {ALL_COLORS_ID, "COLOR_SELECT"};
    els[TOOL_LASER_POINTER_HIGHLIGHTER - TOOL_PEN] = {ALL_COLORS_ID, "COLOR_SELECT"};
    els[TOOL_LINK - TOOL_PEN] = {};
    els[TOOL_LATEX - TOOL_PEN] = {ALL_COLORS_ID, "COLOR_SELECT"};
    return els;
};
static std::array<std::vector<const char*>, TOOL_COUNT> ELEMENTS = populateElements();

AdaptativeToolkit::AdaptativeToolkit(const char* id, ToolMenuHandler* handler):
        AbstractToolItem(id, Category::MISC), handler(handler) {}

auto AdaptativeToolkit::createItem(bool horizontal) -> xoj::util::WidgetSPtr {
    const auto& factories = handler->getToolFactories();

    GtkWidget* stack = gtk_stack_new();
    auto orientation = horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;

    const auto& action = handler->getControl()->getActionDatabase()->getAction(Action::SELECT_TOOL);

    for (ToolType tool = TOOL_PEN; tool < TOOL_END_ENTRY; tool = static_cast<ToolType>(tool + 1)) {
        auto* tb = gtk_toolbar_new();
        gtk_orientable_set_orientation(GTK_ORIENTABLE(tb), orientation);
        gtk_toolbar_set_show_arrow(GTK_TOOLBAR(tb), false);

        for (const char* it: ELEMENTS[tool - TOOL_PEN]) {
            auto fact =
                    std::find_if(factories.begin(), factories.end(), [it](const auto& f) { return f->getId() == it; });
            if (fact != factories.end()) {
                gtk_toolbar_insert(GTK_TOOLBAR(tb), GTK_TOOL_ITEM((*fact)->createToolItem(horizontal).get()), -1);
            } else {
                if (std::string_view(it) == ALL_COLORS_ID) {
                    const auto& palette = handler->getControl()->getPalette();
                    const auto& recolorParams = handler->getControl()->getSettings()->getRecolorParameters();
                    auto recolor =
                            recolorParams.recolorizeMainView ? std::make_optional(recolorParams.recolor) : std::nullopt;

                    for (auto&& c: palette.getColors()) {
                        ColorToolItem i(c, recolor);
                        gtk_toolbar_insert(GTK_TOOLBAR(tb), GTK_TOOL_ITEM(i.createToolItem(horizontal).get()), -1);
                    }
                }
            }
        }

        gtk_stack_add_named(GTK_STACK(stack), tb, toolTypeToString(tool).data());
    }

    // This box is only visible if the right tool is selected
    g_object_bind_property_full(
            G_OBJECT(action.get()), "state", stack, "visible-child-name", G_BINDING_DEFAULT,
            +[](GBinding*, const GValue* from, GValue* to, gpointer) -> gboolean {
                g_value_set_string(to, toolTypeToString(getGVariantValue<ToolType>(g_value_get_variant(from))).data());
                return true;
            },
            nullptr, nullptr, nullptr);

    g_signal_connect_object(stack, "realize", G_CALLBACK(+[](GtkWidget* stack, gpointer p) {
                                xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(p)), xoj::util::adopt);
                                gtk_stack_set_visible_child_name(
                                        GTK_STACK(stack),
                                        toolTypeToString(getGVariantValue<ToolType>(state.get())).data());
                            }),
                            action.get(), GConnectFlags(0));

    return xoj::util::WidgetSPtr(stack, xoj::util::adopt);
}

auto AdaptativeToolkit::getToolDisplayName() const -> std::string { return "Adaptative toolkit"; }

auto AdaptativeToolkit::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(ICON_NAME, GTK_ICON_SIZE_LARGE_TOOLBAR);
}
