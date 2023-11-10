#include "ColorToolItem.h"

#include <utility>  // for move

#include "enums/Action.enum.h"                  // for Action
#include "gui/OpacityToolbox.h"                 // for OpacityToolbox
#include "gui/toolbarMenubar/icon/ColorIcon.h"  // for ColorIcon
#include "util/GtkUtil.h"                       // for setToggleButtonUnreleasable
#include "util/gtk4_helper.h"                   // for gtk_button_set_child

ColorToolItem::ColorToolItem(NamedColor namedColor, OpacityToolbox* opacityToolbox):
        AbstractToolItem(std::string("COLOR(") + std::to_string(namedColor.getIndex()) + ")"),
        namedColor(std::move(namedColor)),
        target(xoj::util::makeGVariantSPtr(namedColor.getColor())),
        opacityToolbox(opacityToolbox) {}

ColorToolItem::~ColorToolItem() = default;

auto ColorToolItem::getColor() const -> Color { return this->namedColor.getColor(); }

auto ColorToolItem::createItem(bool) -> GtkWidget* {
    auto* btn = gtk_toggle_button_new();
    gtk_widget_set_can_focus(btn, false);  // todo(gtk4) not necessary anymore
    auto actionName = std::string("win.") + Action_toString(Action::TOOL_COLOR);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), target.get());
    xoj::util::gtk::setToggleButtonUnreleasable(GTK_TOGGLE_BUTTON(btn));

    gtk_widget_set_tooltip_text(btn, this->namedColor.getName().c_str());
    gtk_button_set_child(GTK_BUTTON(btn), getNewToolIcon());

#if GTK_MAJOR_VERSION == 3
    GtkGesture* singleClickGesture = gtk_gesture_click_new(btn);
    GtkEventController* enterLeaveController = gtk_event_controller_motion_new(btn);
#else
    GtkGesture* singleClickGesture = gtk_gesture_click_new();
    GtkEventController* enterLeaveController = gtk_event_controller_motion_new();
#endif

    this->singleClickGesture.reset(singleClickGesture, xoj::util::refsink);
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(singleClickGesture), GDK_BUTTON_PRIMARY);
    GtkEventController* singleClickController = GTK_EVENT_CONTROLLER(singleClickGesture);
    gtk_event_controller_set_propagation_phase(singleClickController, GTK_PHASE_TARGET);
    gtk_widget_add_controller(btn, singleClickController);
    g_signal_connect(singleClickGesture, "released", G_CALLBACK(handleRelease), this);

    this->enterLeaveController.reset(enterLeaveController, xoj::util::refsink);
    gtk_event_controller_set_propagation_phase(enterLeaveController, GTK_PHASE_TARGET);
    gtk_widget_add_controller(btn, enterLeaveController);
    g_signal_connect(enterLeaveController, "leave", G_CALLBACK(handleLeave), this);
    g_signal_connect(enterLeaveController, "enter", G_CALLBACK(handleEnter), this);

    // For the sake of deprecated GtkToolbar, wrap the button in a GtkToolItem
    // Todo(gtk4): remove
    GtkToolItem* it = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(it), btn);
    /// Makes a proxy item for the toolbar's overflow menu
    auto createProxy = [this]() {
        GtkWidget* proxy = gtk_check_menu_item_new();
        gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(proxy), true);

        auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_container_add(GTK_CONTAINER(proxy), box);
        gtk_box_append(GTK_BOX(box), getNewToolIcon());
        gtk_box_append(GTK_BOX(box), gtk_label_new(getToolDisplayName().c_str()));

        gtk_actionable_set_action_name(GTK_ACTIONABLE(proxy),
                                       (std::string("win.") + Action_toString(Action::TOOL_COLOR)).c_str());
        if (target) {
            gtk_actionable_set_action_target_value(GTK_ACTIONABLE(proxy), target.get());
        }
        xoj::util::gtk::fixActionableInitialSensitivity(GTK_ACTIONABLE(proxy));
        return proxy;
    };
    gtk_tool_item_set_proxy_menu_item(it, "", createProxy());
    this->item.reset(GTK_WIDGET(it), xoj::util::adopt);

    return this->item.get();
}

void ColorToolItem::handleRelease(GtkGesture* gesture, int n_press, double x, double y, ColorToolItem* self) {
    if (self->singleClickGesture.get() == gesture) {
        if (n_press == 1) {
            GtkWidget* colorWidget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));

            // Set the colorWidget in the opacity toolbox before Control->toolColorChanged()
            // is invoked when processing the action of the colorItem.
            // Control->toolColorChanged() will then call opacityToolbox->update()
            // in which will be used the colorWidget to show the opacity toolbox at
            // its coordinates
            self->opacityToolbox->setColorWidget(colorWidget);
        }
    }
}

bool ColorToolItem::handleEnter(GtkEventController* eventController, gdouble x, gdouble y, ColorToolItem* self) {
    if (self->enterLeaveController.get() == eventController) {
        GtkToggleButton* colorWidget = GTK_TOGGLE_BUTTON(gtk_event_controller_get_widget(eventController));

        if (self->opacityToolbox->isEnabled() && gtk_toggle_button_get_active(colorWidget)) {
            self->opacityToolbox->showAt(GTK_WIDGET(colorWidget));
        }
        return true;
    }
    return false;
}

bool ColorToolItem::handleLeave(GtkEventController* eventController, ColorToolItem* self) {
    if (self->enterLeaveController.get() == eventController) {
        if (self->opacityToolbox->isEnabled() && !self->opacityToolbox->isHidden()) {
            GtkToggleButton* colorWidget = GTK_TOGGLE_BUTTON(gtk_event_controller_get_widget(eventController));

            if (gtk_toggle_button_get_active(colorWidget)) {
                // Hide opacity toolbox on leaving colorWidget
                // if pointer is neither over opacity toolbox nor over colorWidget.
                // This condition is necessary to handle crossing events
                // (leave signal will be emitted if entering the opacity toolbox)
                if (!xoj::util::gtk::isEventOverWidget(eventController, self->opacityToolbox->widget.get()) &&
                    !xoj::util::gtk::isEventOverWidget(eventController, GTK_WIDGET(colorWidget), {0, -1, -1, 0})) {
                    self->opacityToolbox->hide();
                }
            }
        }
        return true;
    }
    return false;
}

auto ColorToolItem::getToolDisplayName() const -> std::string { return this->namedColor.getName(); }

auto ColorToolItem::getNewToolIcon() const -> GtkWidget* {
    return ColorIcon::newGtkImage(this->namedColor.getColor(), 16, true);
}
