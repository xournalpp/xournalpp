#include "FormatDialog.h"

#include <string>  // for operator==, string, basic_string

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect
#include <glib.h>         // for g_list_free, GList

#include "control/settings/Settings.h"  // for Settings
#include "gui/PaperFormatUtils.h"       // for PaperFormatUtils
#include "model/FormatDefinitions.h"    // for FormatUnits, XOJ_UNITS, XOJ_U...
#include "util/GListView.h"             // for GListView, GListView<>::GList...
#include "util/StringUtils.h"           // for StringUtils
#include "util/i18n.h"                  // for _

class GladeSearchpath;

constexpr auto UI_FILE = "pageFormat.ui";
constexpr auto UI_DIALOG_NAME = "pageFormatDialog";

using namespace xoj::popup;
using xoj::util::GtkPaperSizeUPtr;

FormatDialog::FormatDialog(GladeSearchpath* gladeSearchPath, Settings* settings, double width, double height,
                           std::function<void(double, double)> callback):
        settings(settings),
        selectedScale(settings->getSizeUnitIndex()),
        scale(XOJ_UNITS[selectedScale].scale),
        origWidth(width),
        origHeight(height),
        callbackFun(callback) {

    Builder builder(gladeSearchPath, UI_FILE);
    window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));

    paperTemplatesCombo = GTK_COMBO_BOX(builder.get("cbTemplate"));
    landscapeButton = GTK_TOGGLE_BUTTON(builder.get("btLandscape"));
    portraitButton = GTK_TOGGLE_BUTTON(builder.get("btPortrait"));
    widthSpin = GTK_SPIN_BUTTON(builder.get("spinWidth"));
    heightSpin = GTK_SPIN_BUTTON(builder.get("spinHeight"));

    GtkWidget* cbUnit = builder.get("cbUnit");

    for (int i = 0; i < XOJ_UNIT_COUNT; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cbUnit), XOJ_UNITS[i].name);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(cbUnit), this->selectedScale);


    PaperFormatUtils::loadDefaultPaperSizes(paperSizes);
    paperSizes.emplace_back(_("Custom"));
    PaperFormatUtils::fillPaperFormatDropDown(paperSizes, paperTemplatesCombo);
    paperSizes.pop_back();  // Remove the custom option to prevent it from being matched in the
                            // spinValueChangedCb function, as its presence would lead to errors

    reset(this);

    g_signal_connect(landscapeButton, "toggled", G_CALLBACK(landscapeSelectedCb), this);
    g_signal_connect(portraitButton, "toggled", G_CALLBACK(portraitSelectedCb), this);
    g_signal_connect(paperTemplatesCombo, "changed", G_CALLBACK(cbFormatChangedCb), this);
    g_signal_connect(cbUnit, "changed", G_CALLBACK(cbUnitChanged), this);

    g_signal_connect_swapped(widthSpin, "value-changed", G_CALLBACK(spinValueChangedCb), this);
    g_signal_connect_swapped(heightSpin, "value-changed", G_CALLBACK(spinValueChangedCb), this);

    g_signal_connect_swapped(builder.get("btReset"), "clicked", G_CALLBACK(reset), this);

    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());
    g_signal_connect_swapped(builder.get("btOk"), "clicked", G_CALLBACK(+[](FormatDialog* self) {
                                 self->settings->setSizeUnitIndex(self->selectedScale);
                                 self->callbackFun(gtk_spin_button_get_value(self->widthSpin) * self->scale,
                                                   gtk_spin_button_get_value(self->heightSpin) * self->scale);
                                 gtk_window_close(self->window.get());
                             }),
                             this);
}

void FormatDialog::setOrientation(Orientation orientation) {
    if (this->orientation == orientation) {
        return;
    }
    this->orientation = orientation;

    gtk_toggle_button_set_active(portraitButton, orientation == ORIENTATION_PORTRAIT);
    gtk_toggle_button_set_active(landscapeButton, orientation == ORIENTATION_LANDSCAPE);
}

void FormatDialog::spinValueChangedCb(FormatDialog* dlg) {
    if (dlg->ignoreSpinChange) {
        return;
    }

    double width = gtk_spin_button_get_value(dlg->widthSpin) * dlg->scale;
    double height = gtk_spin_button_get_value(dlg->heightSpin) * dlg->scale;

    if (width < height) {
        dlg->setOrientation(ORIENTATION_PORTRAIT);
    } else if (width > height) {
        dlg->setOrientation(ORIENTATION_LANDSCAPE);
    } else {
        dlg->setOrientation(ORIENTATION_NOT_DEFINED);
    }

    int i = 0;
    for (auto& size: dlg->paperSizes) {
        xoj_assert(std::holds_alternative<GtkPaperSizeUPtr>(size));
        double w = gtk_paper_size_get_width(std::get<GtkPaperSizeUPtr>(size).get(), GTK_UNIT_POINTS);
        double h = gtk_paper_size_get_height(std::get<GtkPaperSizeUPtr>(size).get(), GTK_UNIT_POINTS);

        if ((static_cast<int>(w - width) == 0 && static_cast<int>(h - height) == 0) ||
            (static_cast<int>(h - width) == 0 && static_cast<int>(w - height) == 0)) {
            break;
        }
        i++;
    }

    gtk_combo_box_set_active(dlg->paperTemplatesCombo, i);
}

void FormatDialog::cbUnitChanged(GtkComboBox* widget, FormatDialog* dlg) {
    int selectd = gtk_combo_box_get_active(widget);
    if (dlg->selectedScale == selectd) {
        return;
    }

    double width = gtk_spin_button_get_value(dlg->widthSpin) * dlg->scale;
    double height = gtk_spin_button_get_value(dlg->heightSpin) * dlg->scale;

    dlg->selectedScale = selectd;
    dlg->scale = XOJ_UNITS[dlg->selectedScale].scale;

    dlg->setSpinValues(width / dlg->scale, height / dlg->scale);
}

void FormatDialog::cbFormatChangedCb(GtkComboBox* widget, FormatDialog* dlg) {
    GtkTreeIter iter;

    if (!gtk_combo_box_get_active_iter(widget, &iter)) {
        return;
    }
    GtkTreeModel* model = gtk_combo_box_get_model(widget);

    GValue value = {0};
    gtk_tree_model_get_value(model, &iter, 1, &value);

    if (!G_VALUE_HOLDS_POINTER(&value)) {
        return;
    }
    auto* s = static_cast<GtkPaperSize*>(g_value_get_pointer(&value));

    if (s == nullptr) {
        return;
    }

    double width = gtk_paper_size_get_width(s, GTK_UNIT_POINTS) / dlg->scale;
    double height = gtk_paper_size_get_height(s, GTK_UNIT_POINTS) / dlg->scale;

    if (dlg->orientation == ORIENTATION_LANDSCAPE) {
        if (width < height) {
            std::swap(width, height);
        }
    } else {
        if (width > height) {
            std::swap(width, height);
        }

        dlg->setOrientation(ORIENTATION_PORTRAIT);
    }

    dlg->setSpinValues(width, height);
}

void FormatDialog::portraitSelectedCb(GtkToggleButton* bt, FormatDialog* dlg) {
    bool activated = gtk_toggle_button_get_active(bt);

    if (activated) {
        gtk_toggle_button_set_active(dlg->landscapeButton, false);
        dlg->orientation = ORIENTATION_PORTRAIT;

        double width = gtk_spin_button_get_value(dlg->widthSpin);
        double height = gtk_spin_button_get_value(dlg->heightSpin);

        if (width > height) {
            // Exchange width and height
            dlg->setSpinValues(height, width);
        }
    }
}

void FormatDialog::landscapeSelectedCb(GtkToggleButton* bt, FormatDialog* dlg) {
    bool activated = gtk_toggle_button_get_active(bt);

    if (activated) {
        gtk_toggle_button_set_active(dlg->portraitButton, false);
        dlg->orientation = ORIENTATION_LANDSCAPE;

        double width = gtk_spin_button_get_value(dlg->widthSpin);
        double height = gtk_spin_button_get_value(dlg->heightSpin);

        if (width < height) {
            // Exchange width and height
            dlg->setSpinValues(height, width);
        }
    }
}

void FormatDialog::reset(FormatDialog* self) {
    self->setSpinValues(self->origWidth / self->scale, self->origHeight / self->scale);
    spinValueChangedCb(self);
}

void FormatDialog::setSpinValues(double width, double height) {
    ignoreSpinChange = true;
    gtk_spin_button_set_value(widthSpin, width);
    gtk_spin_button_set_value(heightSpin, height);
    ignoreSpinChange = false;
}
