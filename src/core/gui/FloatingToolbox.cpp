#include "FloatingToolbox.h"

#include <algorithm>  // for max
#include <memory>     // for allocator

#include <gdk/gdk.h>      // for GdkRectangle, GDK_LEAVE_...
#include <glib-object.h>  // for g_signal_connect

#include "control/Control.h"                 // for Control
#include "control/ToolEnums.h"               // for TOOL_FLOATING_TOOLBOX
#include "control/settings/ButtonConfig.h"   // for ButtonConfig
#include "control/settings/Settings.h"       // for Settings
#include "control/settings/SettingsEnums.h"  // for BUTTON_COUNT
#include "gui/widgets/ToolbarBox.h"
#include "util/glib_casts.h"

#include "MainWindow.h"          // for MainWindow
#include "ToolbarDefinitions.h"  // for ToolbarEntryDefintion


FloatingToolbox::FloatingToolbox(MainWindow* theMainWindow, GtkOverlay* overlay) {
    this->mainWindow = theMainWindow;
    this->floatingToolbox = theMainWindow->get("floatingToolbox");
    this->floatingToolboxX = 200;
    this->floatingToolboxY = 200;
    this->floatingToolboxState = recalcSize;

    gtk_overlay_add_overlay(overlay, this->floatingToolbox);

    auto* ctrl = gtk_event_controller_motion_new();
    g_signal_connect(ctrl, "leave", xoj::util::wrap_for_g_callback_v<handleLeaveFloatingToolbox>, this);
    gtk_widget_add_controller(this->floatingToolbox, ctrl);

    // position overlay widgets
    g_signal_connect(overlay, "get-child-position", xoj::util::wrap_for_g_callback_v<getOverlayPosition>, this);
}


FloatingToolbox::~FloatingToolbox() = default;


void FloatingToolbox::show(double x, double y) {
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
    const auto& tbs = mainWindow->getToolbars();
    static_assert(std::tuple_size<ToolbarArray>() == std::tuple_size<decltype(TOOLBAR_DEFINITIONS)>() &&
                  std::tuple_size<ToolbarArray>() > TBFloatLast);
    for (size_t index = TBFloatFirst; index <= TBFloatLast; index++) {
        if (!tbs[index]->empty()) {
            return true;
        }
    }
    return false;
}


void FloatingToolbox::showForConfiguration() {
    if (this->floatingToolboxActivated())  // Do not show if not being used - at least while experimental.
    {
        GtkWidget* w = this->mainWindow->get("panedMainContents");
        auto p = GRAPHENE_POINT_INIT_ZERO;
        auto q = GRAPHENE_POINT_INIT_ZERO;
        [[maybe_unused]] bool ok = gtk_widget_compute_point(w, gtk_widget_get_ancestor(w, GTK_TYPE_WINDOW), &p, &q);
        xoj_assert(ok);

        this->floatingToolboxX = q.x + 40;  // when configuration state these are
        this->floatingToolboxY = q.y + 40;  // topleft coordinates( otherwise center).
        this->floatingToolboxState = configuration;
        this->show();
    }
}


void FloatingToolbox::show() {
    gtk_widget_show(this->floatingToolbox);
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
                                         FloatingToolbox* self) -> gboolean {
    if (widget == self->floatingToolbox) {
        gtk_widget_get_allocation(widget, allocation);  // get existing width and height

        if (self->floatingToolboxState != noChange ||
            allocation->height < 2)  // if recalcSize or configuration or  initiation.
        {
            GtkRequisition natural;
            gtk_widget_get_preferred_size(widget, nullptr, &natural);
            allocation->width = natural.width;
            allocation->height = natural.height;
        }

        switch (self->floatingToolboxState) {
            case recalcSize:  // fallthrough 		note: recalc done above
            case noChange:
                // show centered on x,y
                allocation->x = self->floatingToolboxX - allocation->width / 2;
                allocation->y = self->floatingToolboxY - allocation->height / 2;
                self->floatingToolboxState = noChange;
                break;

            case configuration:
                allocation->x = self->floatingToolboxX;
                allocation->y = self->floatingToolboxY;
                allocation->width = std::max(allocation->width + 32, 50);  // always room for one more...
                allocation->height = std::max(allocation->height, 50);
                break;
        }

        return true;
    }

    return false;
}


void FloatingToolbox::handleLeaveFloatingToolbox(GtkEventControllerMotion* ectrl, FloatingToolbox* self) {
    if (gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(ectrl)) == self->floatingToolbox) {
        if (self->floatingToolboxState != configuration) {
            self->hide();
        }
    }
}
