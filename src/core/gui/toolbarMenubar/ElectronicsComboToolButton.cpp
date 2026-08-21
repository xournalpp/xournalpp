#include "ElectronicsComboToolButton.h"

#include <glib/gi18n.h>

#include "control/Control.h"
#include "control/ToolHandler.h"
#include "control/actions/ActionDatabase.h"
#include "control/Tool.h"

ElectronicsComboToolButton::ElectronicsComboToolButton(std::string id, ActionDatabase* db)
    : AbstractToolItem(std::move(id), Category::TOOLS), db(db) {}

ElectronicsComboToolButton::~ElectronicsComboToolButton() = default;

std::string ElectronicsComboToolButton::getToolDisplayName() const {
    return _("Electronics Components");
}

GtkWidget* ElectronicsComboToolButton::getNewToolIcon() const {
    return gtk_image_new_from_icon_name("draw-spline", GTK_ICON_SIZE_MENU);
}

void ElectronicsComboToolButton::onComponentSelected(GtkWidget* widget, gpointer data) {
    auto* self = static_cast<ElectronicsComboToolButton*>(data);
    auto type = static_cast<ElectronicsComponentType>(GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "comp_type")));

    // Set the component type for the handler
    if (auto* t = self->db->control->getToolHandler()->getActiveTool()) {
        t->setElectronicsComponentType(type);
    }

    // Activate the tool action
    self->db->setActionState(Action::TOOL_DRAW_ELECTRONICS, true);
    self->db->control->getToolHandler()->setDrawingType(DRAWING_TYPE_ELECTRONICS);
}

xoj::util::WidgetSPtr ElectronicsComboToolButton::createItem(bool horizontal) {
    GtkWidget* box = gtk_box_new(horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0);

    // Primary action button (activates the tool)
    GtkWidget* button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), getNewToolIcon());
    gtk_widget_set_tooltip_text(button, _("Draw Electronics Component"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button), "win.tool-draw-electronics");
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

    // Menu button for selection
    GtkWidget* menuButton = gtk_menu_button_new();
    gtk_widget_set_tooltip_text(menuButton, _("Select Electronics Component"));

    GtkWidget* menu = gtk_menu_new();

    // Helper to create submenus
    auto createSubMenu = [&](const char* title) {
        GtkWidget* menuItem = gtk_menu_item_new_with_label(title);
        GtkWidget* subMenu = gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), subMenu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
        return subMenu;
    };

    auto addItem = [&](GtkWidget* parentMenu, const char* label, ElectronicsComponentType type) {
        GtkWidget* item = gtk_menu_item_new_with_label(label);
        g_object_set_data(G_OBJECT(item), "comp_type", GINT_TO_POINTER(type));
        g_signal_connect(item, "activate", G_CALLBACK(onComponentSelected), this);
        gtk_menu_shell_append(GTK_MENU_SHELL(parentMenu), item);
    };

    // --- Categories ---
    GtkWidget* waveMenu = createSubMenu(_("Waveforms"));
    addItem(waveMenu, _("Sine Wave"), ELEC_WAVE_SINE);
    addItem(waveMenu, _("Square Wave"), ELEC_WAVE_SQUARE);
    addItem(waveMenu, _("Triangle Wave"), ELEC_WAVE_TRIANGLE);

    GtkWidget* passiveMenu = createSubMenu(_("Passives & Sources"));
    addItem(passiveMenu, _("Resistor (US)"), ELEC_RESISTOR_US);
    addItem(passiveMenu, _("Resistor (EU)"), ELEC_RESISTOR_EU);
    addItem(passiveMenu, _("Capacitor"), ELEC_CAPACITOR_NP);
    addItem(passiveMenu, _("Inductor"), ELEC_INDUCTOR);
    addItem(passiveMenu, _("Ground (Earth)"), ELEC_GND_EARTH);
    addItem(passiveMenu, _("Ground (Signal)"), ELEC_GND_SIGNAL);
    addItem(passiveMenu, _("DC Source"), ELEC_SOURCE_DC);
    addItem(passiveMenu, _("AC Source"), ELEC_SOURCE_AC);

    GtkWidget* semiMenu = createSubMenu(_("Semiconductors"));
    addItem(semiMenu, _("Diode"), ELEC_DIODE);
    addItem(semiMenu, _("LED"), ELEC_DIODE_LED);
    addItem(semiMenu, _("NPN Transistor"), ELEC_BJT_NPN);
    addItem(semiMenu, _("PNP Transistor"), ELEC_BJT_PNP);
    addItem(semiMenu, _("N-MOSFET"), ELEC_MOSFET_N);
    addItem(semiMenu, _("Op-Amp"), ELEC_OPAMP);

    GtkWidget* logicMenu = createSubMenu(_("Logic Gates"));
    addItem(logicMenu, _("AND Gate"), ELEC_GATE_AND);
    addItem(logicMenu, _("OR Gate"), ELEC_GATE_OR);
    addItem(logicMenu, _("NOT Gate"), ELEC_GATE_NOT);
    addItem(logicMenu, _("NAND Gate"), ELEC_GATE_NAND);
    addItem(logicMenu, _("NOR Gate"), ELEC_GATE_NOR);
    addItem(logicMenu, _("XOR Gate"), ELEC_GATE_XOR);
    addItem(logicMenu, _("D Flip-Flop"), ELEC_FF_D);

    gtk_widget_show_all(menu);
    gtk_menu_button_set_popup(GTK_MENU_BUTTON(menuButton), menu);

    gtk_box_pack_start(GTK_BOX(box), menuButton, FALSE, FALSE, 0);
    gtk_widget_show_all(box);

    return xoj::util::WidgetSPtr(box, xoj::util::adopt);
}
