#include "ColorToolItem.h"

#include <config.h>

#include "control/ToolEnums.h"
#include "gui/toolbarMenubar/icon/ColorSelectImage.h"
#include "model/ToolbarColorNames.h"

#include "StringUtils.h"
#include "Util.h"
#include "i18n.h"

bool ColorToolItem::inUpdate = false;

ColorToolItem::ColorToolItem(ActionHandler* handler, ToolHandler* toolHandler, GtkWindow* parent, Color color,
                             bool selektor):
        AbstractToolItem("", handler, selektor ? ACTION_SELECT_COLOR_CUSTOM : ACTION_SELECT_COLOR),
        color(color),
        toolHandler(toolHandler),
        parent(parent) {
    this->group = GROUP_COLOR;

    updateName();
}

ColorToolItem::~ColorToolItem() { freeIcons(); }

/**
 * Free the allocated icons
 */
void ColorToolItem::freeIcons() {
    delete this->icon;
    this->icon = nullptr;
}

auto ColorToolItem::isSelector() -> bool { return this->action == ACTION_SELECT_COLOR_CUSTOM; }

void ColorToolItem::updateName() {
    if (this->action == ACTION_SELECT_COLOR_CUSTOM) {
        this->name = _("Select color");
    } else {
        this->name = ToolbarColorNames::getInstance().getColorName(this->color);
    }
}

void ColorToolItem::actionSelected(ActionGroup group, ActionType action) {
    inUpdate = true;
    if (this->group == group && this->item) {
        if (isSelector()) {
            gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), isSelector());
        }
        enableColor(toolHandler->getColor());
    }
    inUpdate = false;
}

void ColorToolItem::enableColor(Color color) {
    if (isSelector()) {
        if (this->icon) {
            this->icon->setColor(color);
        }

        this->color = color;
        if (GTK_IS_TOGGLE_BUTTON(this->item)) {
            gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), false);
        }
    } else {
        if (this->item) {
            gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), this->color == color);
        }
    }
}

auto ColorToolItem::getColor() const -> Color { return this->color; }

auto ColorToolItem::getId() -> string {
    if (isSelector()) {
        return "COLOR_SELECT";
    }

    char buffer[64];
    sprintf(buffer, "COLOR(0x%06x)", uint32_t{this->color});
    string id = buffer;

    return id;
}

/**
 * Show colochooser to select a custom color
 */
void ColorToolItem::showColorchooser() {
    GtkWidget* dialog = gtk_color_chooser_dialog_new(_("Select color"), parent);
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dialog), false);

    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        GdkRGBA color;
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &color);
        this->color = Util::GdkRGBA_to_argb(color);
    }

    gtk_widget_destroy(dialog);
}

/**
 * Enable / Disable the tool item
 */
void ColorToolItem::enable(bool enabled) {
    if (!enabled && !toolHandler->hasCapability(TOOL_CAP_COLOR, ToolPointer::current) &&
        toolHandler->hasCapability(TOOL_CAP_COLOR, ToolPointer::toolbar)) {
        if (this->icon && toolHandler->triggeredByButton) {
            // allow changes if currentTool has no colour capability
            // and mainTool has Colour capability
            icon->setState(COLOR_ICON_STATE_PEN);
            AbstractToolItem::enable(true);
        } else {
            // disallow changes in color
            icon->setState(COLOR_ICON_STATE_DISABLED);
            AbstractToolItem::enable(false);
        }
        return;
    }

    AbstractToolItem::enable(enabled);
    if (this->icon) {
        if (enabled) {
            icon->setState(COLOR_ICON_STATE_ENABLED);
        } else {
            icon->setState(COLOR_ICON_STATE_DISABLED);
        }
    }
}

void ColorToolItem::activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton) {
    if (inUpdate) {
        return;
    }
    inUpdate = true;

    if (isSelector()) {
        showColorchooser();
    }

    toolHandler->setColor(this->color, true);

    inUpdate = false;
}

auto ColorToolItem::newItem() -> GtkToolItem* {
    this->icon = new ColorSelectImage(this->color, !isSelector());

    GtkToolItem* it = gtk_toggle_tool_button_new();

    const gchar* name = this->name.c_str();
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(it), name);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(it), name);

    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(it), this->icon->getWidget());

    return it;
}

auto ColorToolItem::getToolDisplayName() -> string { return this->name; }

auto ColorToolItem::getNewToolIcon() -> GtkWidget* {
    return ColorSelectImage::newColorIcon(this->color, 16, !isSelector());
}
