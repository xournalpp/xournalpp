#include "FormatDialog.h"

#include <string>  // for operator==, string, basic_string

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect
#include <glib.h>         // for g_list_free, GList

#include "control/settings/Settings.h"  // for Settings
#include "model/FormatDefinitions.h"    // for FormatUnits, XOJ_UNITS, XOJ_U...
#include "util/GListView.h"             // for GListView, GListView<>::GList...
#include "util/StringUtils.h"           // for StringUtils
#include "util/i18n.h"                  // for _

class GladeSearchpath;


FormatDialog::FormatDialog(GladeSearchpath* gladeSearchPath, Settings* settings, double width, double height):
        GladeGui(gladeSearchPath, "pagesize.glade", "pagesizeDialog"), settings(settings) {
    this->selectedScale = settings->getSizeUnitIndex();

    this->scale = XOJ_UNITS[this->selectedScale].scale;
    this->origHeight = height;
    this->origWidth = width;

    setSpinValues(this->origWidth / this->scale, this->origHeight / this->scale);

    GtkWidget* cbUnit = get("cbUnit");

    for (int i = 0; i < XOJ_UNIT_COUNT; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cbUnit), XOJ_UNITS[i].name);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(cbUnit), this->selectedScale);

    GtkWidget* cbTemplate = get("cbTemplate");
    GtkListStore* store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
    gtk_combo_box_set_model(GTK_COMBO_BOX(cbTemplate), GTK_TREE_MODEL(store));
    g_object_unref(store);

    GtkCellRenderer* cell = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cbTemplate), cell, true);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cbTemplate), cell, "text", 0, nullptr);

    int selectedFormat = -1;

    // Todo(fabian) why does this nothing and is still here or is it a bug? (Commented it out)
    //		if (height < width)
    //		{
    //		double tmp = width;
    //		width = height;
    //		height = tmp;
    //	}

    loadPageFormats();

    int i = 0;
    for (auto& size: paperSizes) {
        std::string displayName = gtk_paper_size_get_display_name(size.get());
        if (StringUtils::startsWith(displayName, "custom_")) {
            displayName = displayName.substr(7);
        }

        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, displayName.c_str(), -1);
        gtk_list_store_set(store, &iter, 1, size.get(), -1);
        i++;
    }

    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, _("Custom"), -1);
    gtk_list_store_set(store, &iter, 1, nullptr, -1);

    // not found, select custom format
    if (selectedFormat == -1) {
        selectedFormat = i;
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(cbTemplate), selectedFormat);

    spinValueChangedCb(nullptr, this);

    g_signal_connect(get("btLandscape"), "toggled", G_CALLBACK(landscapeSelectedCb), this);
    g_signal_connect(get("btPortrait"), "toggled", G_CALLBACK(portraitSelectedCb), this);
    g_signal_connect(cbTemplate, "changed", G_CALLBACK(cbFormatChangedCb), this);
    g_signal_connect(cbUnit, "changed", G_CALLBACK(cbUnitChanged), this);

    g_signal_connect(get("spinWidth"), "value-changed", G_CALLBACK(spinValueChangedCb), this);
    g_signal_connect(get("spinHeight"), "value-changed", G_CALLBACK(spinValueChangedCb), this);
}

void FormatDialog::loadPageFormats() {
    GList* default_sizes = gtk_paper_size_get_paper_sizes(false);
    for (auto& s: GListView<GtkPaperSize>(default_sizes)) {
        std::string name = gtk_paper_size_get_name(&s);
        if (name == GTK_PAPER_NAME_A3 || name == GTK_PAPER_NAME_A4 || name == GTK_PAPER_NAME_A5 ||
            name == GTK_PAPER_NAME_LETTER || name == GTK_PAPER_NAME_LEGAL) {
            paperSizes.emplace_back(&s, gtk_paper_size_free);
            continue;
        }
        gtk_paper_size_free(&s);
    }
    g_list_free(default_sizes);
    // Name format: ftp://ftp.pwg.org/pub/pwg/candidates/cs-pwgmsn10-20020226-5101.1.pdf
    paperSizes.emplace_back(gtk_paper_size_new("custom_16x9_320x180mm"), gtk_paper_size_free);
    paperSizes.emplace_back(gtk_paper_size_new("custom_4x3_320x240mm"), gtk_paper_size_free);
}

auto FormatDialog::getWidth() const -> double { return this->width; }

auto FormatDialog::getHeight() const -> double { return this->height; }

void FormatDialog::setOrientation(Orientation orientation) {
    if (this->orientation == orientation) {
        return;
    }
    this->orientation = orientation;

    GtkWidget* btPortrait = get("btPortrait");
    GtkWidget* btLandscape = get("btLandscape");

    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(btPortrait), orientation == ORIENTATION_PORTRAIT);
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(btLandscape), orientation == ORIENTATION_LANDSCAPE);
}

void FormatDialog::spinValueChangedCb(GtkSpinButton* spinbutton, FormatDialog* dlg) {
    if (dlg->ignoreSpinChange) {
        return;
    }

    double width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinWidth"))) * dlg->scale;
    double height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinHeight"))) * dlg->scale;

    if (width < height) {
        dlg->setOrientation(ORIENTATION_PORTRAIT);
    } else if (width > height) {
        dlg->setOrientation(ORIENTATION_LANDSCAPE);
    } else {
        dlg->setOrientation(ORIENTATION_NOT_DEFINED);
    }

    int i = 0;
    for (auto& size: dlg->paperSizes) {
        double w = gtk_paper_size_get_width(size.get(), GTK_UNIT_POINTS);
        double h = gtk_paper_size_get_height(size.get(), GTK_UNIT_POINTS);

        if ((static_cast<int>(w - width) == 0 && static_cast<int>(h - height) == 0) ||
            (static_cast<int>(h - width) == 0 && static_cast<int>(w - height) == 0)) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(dlg->get("cbTemplate")), i);
            return;
        }
        i++;
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(dlg->get("cbTemplate")), i);
}

void FormatDialog::cbUnitChanged(GtkComboBox* widget, FormatDialog* dlg) {
    int selectd = gtk_combo_box_get_active(widget);
    if (dlg->selectedScale == selectd) {
        return;
    }

    double width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinWidth")));
    double height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinHeight")));

    width *= dlg->scale;
    height *= dlg->scale;

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
            double tmp = width;
            width = height;
            height = tmp;
        }
    } else {
        if (width > height) {
            double tmp = width;
            width = height;
            height = tmp;
        }

        dlg->setOrientation(ORIENTATION_PORTRAIT);
    }

    dlg->setSpinValues(width, height);
}

void FormatDialog::portraitSelectedCb(GtkToggleToolButton* bt, FormatDialog* dlg) {
    bool activated = gtk_toggle_tool_button_get_active(bt);

    if (activated) {
        gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(dlg->get("btLandscape")), false);
        dlg->orientation = ORIENTATION_PORTRAIT;

        double width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinWidth")));
        double height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinHeight")));

        if (width > height) {
            // Exchange width and height
            dlg->setSpinValues(height, width);
        }
    }
}

void FormatDialog::landscapeSelectedCb(GtkToggleToolButton* bt, FormatDialog* dlg) {
    bool activated = gtk_toggle_tool_button_get_active(bt);

    if (activated) {
        gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(dlg->get("btPortrait")), false);
        dlg->orientation = ORIENTATION_LANDSCAPE;

        double width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinWidth")));
        double height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->get("spinHeight")));

        if (width < height) {
            // Exchange width and height
            dlg->setSpinValues(height, width);
        }
    }
}

void FormatDialog::setSpinValues(double width, double heigth) {
    ignoreSpinChange = true;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spinWidth")), width);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spinHeight")), heigth);
    ignoreSpinChange = false;
}

void FormatDialog::show(GtkWindow* parent) {
    int ret = 0;
    while (ret == 0) {
        gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
        ret = gtk_dialog_run(GTK_DIALOG(this->window));
        if (ret == 0) {
            setSpinValues(this->origWidth / this->scale, this->origHeight / this->scale);
        }
    }

    if (ret == 1)  // OK
    {
        settings->setSizeUnitIndex(this->selectedScale);

        this->width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spinWidth"))) * this->scale;
        this->height = gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spinHeight"))) * this->scale;
    }

    gtk_widget_hide(this->window);
}
