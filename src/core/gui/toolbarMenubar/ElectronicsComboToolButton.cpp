#include "ElectronicsComboToolButton.h"

#include <glib/gi18n.h>

#include "control/Control.h"
#include "control/ToolHandler.h"
#include "control/actions/ActionDatabase.h"
#include "control/Tool.h"

ElectronicsComboToolButton::Entry::Entry(std::string name, ElectronicsComponentType type)
    : name(std::move(name)), type(type) {}

ElectronicsComboToolButton::ElectronicsComboToolButton(std::string id, ActionDatabase* db)
    : AbstractToolItem(std::move(id), Category::TOOLS), db(db) {

    entries.emplace_back(_("Sine Wave"), ELEC_WAVE_SINE);
    entries.emplace_back(_("Square Wave"), ELEC_WAVE_SQUARE);
    entries.emplace_back(_("Triangle Wave"), ELEC_WAVE_TRIANGLE);
    entries.emplace_back(_("Resistor (US)"), ELEC_RESISTOR_US);
    entries.emplace_back(_("Resistor (EU)"), ELEC_RESISTOR_EU);
    entries.emplace_back(_("Capacitor"), ELEC_CAPACITOR_NP);
    entries.emplace_back(_("Inductor"), ELEC_INDUCTOR);
    entries.emplace_back(_("Diode"), ELEC_DIODE);
    entries.emplace_back(_("LED"), ELEC_DIODE_LED);
    entries.emplace_back(_("NPN Transistor"), ELEC_BJT_NPN);
    entries.emplace_back(_("PNP Transistor"), ELEC_BJT_PNP);
    entries.emplace_back(_("N-MOSFET"), ELEC_MOSFET_N);
    entries.emplace_back(_("Op-Amp"), ELEC_OPAMP);
    entries.emplace_back(_("AND Gate"), ELEC_GATE_AND);
    entries.emplace_back(_("OR Gate"), ELEC_GATE_OR);
    entries.emplace_back(_("NOT Gate"), ELEC_GATE_NOT);
    entries.emplace_back(_("NAND Gate"), ELEC_GATE_NAND);
    entries.emplace_back(_("NOR Gate"), ELEC_GATE_NOR);
    entries.emplace_back(_("XOR Gate"), ELEC_GATE_XOR);
    entries.emplace_back(_("D Flip-Flop"), ELEC_FF_D);
    entries.emplace_back(_("Ground (Earth)"), ELEC_GND_EARTH);
    entries.emplace_back(_("Ground (Signal)"), ELEC_GND_SIGNAL);
    entries.emplace_back(_("DC Source"), ELEC_SOURCE_DC);
    entries.emplace_back(_("AC Source"), ELEC_SOURCE_AC);
}

ElectronicsComboToolButton::~ElectronicsComboToolButton() = default;

std::string ElectronicsComboToolButton::getToolDisplayName() const {
    return _("Electronics Components");
}

GtkWidget* ElectronicsComboToolButton::getNewToolIcon() const {
    return gtk_image_new_from_icon_name("draw-spline", GTK_ICON_SIZE_MENU);
}

void ElectronicsComboToolButton::onComponentSelected(GtkWidget* widget, gpointer data) {
    auto* self = static_cast<ElectronicsComboToolButton*>(data);

    // GtkComboBoxText handling
    int active = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    if (active >= 0 && active < static_cast<int>(self->entries.size())) {
        auto type = self->entries[active].type;
        self->db->setActionState(Action::TOOL_DRAW_ELECTRONICS, true);

        // Expose method natively
        if (auto* t = self->db->control->getToolHandler()->getActiveTool()) {
            t->setElectronicsComponentType(type);
            self->db->control->getToolHandler()->setDrawingType(DRAWING_TYPE_ELECTRONICS);
        }
    }
}

xoj::util::WidgetSPtr ElectronicsComboToolButton::createItem(bool horizontal) {
    GtkWidget* box = gtk_box_new(horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0);

    // Primary action button (activates the tool)
    GtkWidget* button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), getNewToolIcon());
    gtk_widget_set_tooltip_text(button, _("Draw Electronics Component"));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button), "win.tool-draw-electronics");
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

    // Combo box for component selection
    GtkWidget* combo = gtk_combo_box_text_new();
    for (const auto& entry : entries) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), entry.name.c_str());
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0); // Default to first item

    g_signal_connect(combo, "changed", G_CALLBACK(onComponentSelected), this);

    gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 0);

    gtk_widget_show_all(box);
    return xoj::util::WidgetSPtr(box, xoj::util::adopt);
}
