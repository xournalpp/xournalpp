#include "FloatingToolbox.h"

#include <algorithm>  // for max
#include <memory>     // for allocator

#include <gdk/gdk.h>      // for GdkRectangle, GDK_LEAVE_...
#include <glib-object.h>  // for G_CALLBACK, g_signal_con...

#include "control/Control.h"                 // for Control
#include "control/ToolEnums.h"               // for TOOL_FLOATING_TOOLBOX
#include "control/settings/ButtonConfig.h"   // for ButtonConfig
#include "control/settings/Settings.h"       // for Settings
#include "control/settings/SettingsEnums.h"  // for BUTTON_COUNT
#include "util/glib_casts.h"

#include "MainWindow.h"          // for MainWindow
#include "ToolbarDefinitions.h"  // for ToolbarEntryDefintion
#include "XournalView.h"


FloatingToolbox::FloatingToolbox(MainWindow* theMainWindow, GtkOverlay* overlay) {
    this->mainWindow = theMainWindow;
    this->floatingToolbox = theMainWindow->get("floatingToolbox");
    this->floatingToolboxX = 200;
    this->floatingToolboxY = 200;
    this->floatingToolboxState = recalcSize;

    gtk_overlay_add_overlay(overlay, this->floatingToolbox);
    gtk_overlay_set_overlay_pass_through(overlay, this->floatingToolbox, true);
    gtk_widget_add_events(this->floatingToolbox, GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect(this->floatingToolbox, "leave-notify-event",
                     xoj::util::wrap_for_g_callback_v<handleLeaveFloatingToolbox>, this);
    // position overlay widgets
    g_signal_connect(overlay, "get-child-position", xoj::util::wrap_for_g_callback_v<getOverlayPosition>, this);
}


FloatingToolbox::~FloatingToolbox() = default;


void FloatingToolbox::show(int x, int y) {
    this->floatingToolboxX = x;
    this->floatingToolboxY = y;
    this->show();
}


/****
 * floatingToolboxActivated
 *  True if the user has:
 *    assigned a mouse or stylus button to bring up the floatingToolbox;
 *    or enabled tapAction and Show FloatingToolbox( prefs->DrawingArea->ActionOnToolTap );
 *    or put tools in the FloatingToolbox.
 *
 */
auto FloatingToolbox::floatingToolboxActivated() -> bool {
    Settings* settings = this->mainWindow->getControl()->getSettings();
    ButtonConfig* cfg = nullptr;

    // check if any buttons assigned to bring up toolbox
    for (unsigned int id = 0; id < BUTTON_COUNT; id++) {
        cfg = settings->getButtonConfig(id);

        if (cfg->getAction() == TOOL_FLOATING_TOOLBOX) {
            return true;  // return true
        }
    }

    // check if user can show Floating Menu with tap.
    if (settings->getDoActionOnStrokeFiltered() && settings->getStrokeFilterEnabled()) {
        return true;  // return true
    }

    return this->hasWidgets();
}


auto FloatingToolbox::hasWidgets() -> bool {
    for (int index = TBFloatFirst; index <= TBFloatLast; index++) {
        GtkToolbar* toolbar1 = GTK_TOOLBAR(this->mainWindow->get(TOOLBAR_DEFINITIONS[index].guiName));
        if (gtk_toolbar_get_n_items(toolbar1) > 0) {
            return true;
        }
    }
    return false;
}


void FloatingToolbox::showForConfiguration() {
    if (this->floatingToolboxActivated()) {
        this->floatingToolboxState = configuration;
        this->show();
    }
}


void FloatingToolbox::show() {
    gtk_widget_show_all(this->floatingToolbox);
    gtk_widget_set_visible(this->mainWindow->get("labelFloatingToolbox"), this->floatingToolboxState == configuration);
    gtk_widget_set_visible(this->mainWindow->get("showIfEmpty"),
                           this->floatingToolboxState != configuration && !hasWidgets());
}


void FloatingToolbox::hide() {
    if (this->floatingToolboxState == configuration) {
        this->floatingToolboxState = recalcSize;
    }

    gtk_widget_hide(this->floatingToolbox);
}


void FloatingToolbox::flagRecalculateSizeRequired() { this->floatingToolboxState = recalcSize; }


/**
 * getOverlayPosition - this is how we position the widget in the overlay under the mouse
 *
 * The requested location is communicated via the FloatingToolbox member variables:
 * ->floatingToolbox,		so we can operate on the right widget
 * ->floatingToolboxState,	are we configuring, resizing or just moving
 * ->floatingToolboxX,		where to display
 * ->floatingToolboxY.
 *
 */
auto FloatingToolbox::getOverlayPosition(GtkOverlay* overlay, GtkWidget* widget, GdkRectangle* allocation,
                                         FloatingToolbox* self) -> bool {
    if (widget != self->floatingToolbox) {
        return false;
    }

    gtk_widget_get_allocation(widget, allocation);

    if (self->floatingToolboxState != noChange || allocation->height < 2) {
        GtkRequisition natural;
        gtk_widget_get_preferred_size(widget, nullptr, &natural);
        allocation->width = natural.width;
        allocation->height = natural.height;
    }

    GtkWidget* scrolledWindow =
            gtk_widget_get_ancestor(self->mainWindow->getXournal()->getWidget(), GTK_TYPE_SCROLLED_WINDOW);

    switch (self->floatingToolboxState) {
        case recalcSize:
            [[fallthrough]];
        case noChange: {
            int centerX = self->floatingToolboxX - allocation->width / 2;
            int centerY = self->floatingToolboxY - allocation->height / 2;

            // Clamp to scrolled window bounds with margin
            constexpr int margin = 10;
            int minX = margin;
            int maxX = gtk_widget_get_allocated_width(scrolledWindow) - allocation->width - margin;
            int minY = margin;
            int maxY = gtk_widget_get_allocated_height(scrolledWindow) - allocation->height - margin;

            // Ensure valid clamp bounds when toolbox is larger than viewport
            maxX = std::max(maxX, minX);
            maxY = std::max(maxY, minY);

            allocation->x = std::clamp(centerX, minX, maxX);
            allocation->y = std::clamp(centerY, minY, maxY);
            self->floatingToolboxState = noChange;
            break;
        }

        case configuration:
            allocation->x = 40;
            allocation->y = 40;
            allocation->width = std::max(allocation->width + 32, 50);
            allocation->height = std::max(allocation->height, 50);
            break;
    }

    gtk_widget_translate_coordinates(scrolledWindow, GTK_WIDGET(overlay), allocation->x, allocation->y, &allocation->x,
                                     &allocation->y);

    return true;
}


bool FloatingToolbox::handleLeaveFloatingToolbox(GtkWidget* floatingToolbox, GdkEvent* event, FloatingToolbox* self) {
    if (floatingToolbox == self->floatingToolbox) {
        if (self->floatingToolboxState != configuration) {
            self->hide();
        }
        return true;
    }
    return false;
}
