#include "ExportDialog.h"

#include <algorithm>  // for max
#include <stdexcept>  // for invalid_argument
#include <string>     // for string, to_string, operator+
#include <vector>     // for allocator

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect
#include <glib.h>         // for GSList, TRUE, FALSE

#include "util/ElementRange.h"  // for parse, PageRangeVector

class GladeSearchpath;

ExportDialog::ExportDialog(GladeSearchpath* gladeSearchPath):
        GladeGui(gladeSearchPath, "exportSettings.glade", "exportDialog") {
    // rdRangePages toggled signal handler
    //
    // Sets and unsets the sensitivity of the text form, OK button and
    // displays an error when the text form is selected and contains
    // invalid user data. The error goes away when the text form is de-selected.
    auto toggledHandler = G_CALLBACK(+[](GtkToggleButton* togglebutton, ExportDialog* self) {
        auto active = gtk_toggle_button_get_active(togglebutton);
        auto btOk = self->get("btOk");
        auto txtPages = self->get("txtPages");
        auto context = gtk_widget_get_style_context(txtPages);
        gtk_widget_set_sensitive(txtPages, active);
        if (active) {
            const std::string text_form(gtk_editable_get_chars(GTK_EDITABLE(txtPages), 0, -1));
            try {
                ElementRange::parse(text_form, self->pageCount);
                gtk_style_context_remove_class(context, "error");
                gtk_widget_set_sensitive(btOk, TRUE);
            } catch (const std::invalid_argument&) {
                gtk_style_context_add_class(context, "error");
                gtk_widget_set_sensitive(btOk, FALSE);
            }
        } else {
            gtk_style_context_remove_class(context, "error");
            gtk_widget_set_sensitive(btOk, TRUE);
        }
    });
    // txtPages changed signal handler
    //
    // Displays an error when the user inputs invalid page ranges in the text form,
    // and sets the sensitivity of the OK button to FALSE. These actions are
    // reversed when the text form contains a valid page range.
    auto changedHandler = G_CALLBACK(+[](GtkEditable* txtPages, ExportDialog* self) {
        auto context = gtk_widget_get_style_context(GTK_WIDGET(txtPages));
        const std::string text_form(gtk_editable_get_chars(txtPages, 0, -1));
        auto btOk = self->get("btOk");
        try {
            ElementRange::parse(text_form, self->pageCount);
            gtk_style_context_remove_class(context, "error");
            gtk_widget_set_sensitive(btOk, TRUE);
        } catch (const std::invalid_argument& e) {
            gtk_style_context_add_class(context, "error");
            gtk_widget_set_sensitive(btOk, FALSE);
        }
    });

    gtk_widget_hide(get("cbProgressiveMode"));
    g_signal_connect(get("rdRangePages"), "toggled", toggledHandler, this);
    g_signal_connect(get("cbQuality"), "changed", G_CALLBACK(ExportDialog::selectQualityCriterion), this);
    GSList* radios = gtk_radio_button_get_group(GTK_RADIO_BUTTON(get("rdRangeAll")));
    for (GSList* head = radios; head != nullptr; head = head->next) {
        g_signal_connect(reinterpret_cast<GtkRadioButton*>(head->data), "activate",
                         G_CALLBACK(+[](GtkButton*, ExportDialog* self) {
                             gtk_dialog_response(GTK_DIALOG(self->window), GTK_RESPONSE_OK);
                         }),
                         this);
    }
    g_signal_connect(get("txtPages"), "changed", changedHandler, this);
}

void ExportDialog::initPages(size_t current, size_t count) {
    std::string allPages = "1 - " + std::to_string(count);
    gtk_label_set_text(GTK_LABEL(get("lbAllPagesInfo")), allPages.c_str());
    std::string currentPages = std::to_string(current);
    gtk_label_set_text(GTK_LABEL(get("lbCurrentPage")), currentPages.c_str());

    this->currentPage = current;
    this->pageCount = count;
}

void ExportDialog::removeQualitySetting() {
    gtk_widget_hide(get("lbQuality"));
    gtk_widget_hide(get("boxQuality"));
    gtk_widget_hide(get("cbQuality"));
}

void ExportDialog::showProgressiveMode() {
    gtk_widget_show(get("cbProgressiveMode"));
    removeQualitySetting();
}

void ExportDialog::selectQualityCriterion(GtkComboBox* comboBox, ExportDialog* self) {
    int activeCriterion = gtk_combo_box_get_active(comboBox);
    switch (activeCriterion) {
        case EXPORT_QUALITY_DPI:
            gtk_label_set_text(GTK_LABEL(self->get("lbQualityUnit")), "dpi");
            gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(self->get("sbQualityValue")),
                                           GTK_ADJUSTMENT(gtk_builder_get_object(self->getBuilder(), "adjustmentDpi")));
            break;
        case EXPORT_QUALITY_WIDTH:
        case EXPORT_QUALITY_HEIGHT:
            gtk_label_set_text(GTK_LABEL(self->get("lbQualityUnit")), "px");
            gtk_spin_button_set_adjustment(
                    GTK_SPIN_BUTTON(self->get("sbQualityValue")),
                    GTK_ADJUSTMENT(gtk_builder_get_object(self->getBuilder(), "adjustmentHeightWidth")));
            break;
    }
}

auto ExportDialog::getPngQualityParameter() -> RasterImageQualityParameter {
    return RasterImageQualityParameter(
            (ExportQualityCriterion)gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbQuality"))),
            gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get("sbQualityValue"))));
}

auto ExportDialog::isConfirmed() const -> bool { return this->confirmed; }

auto ExportDialog::progressiveMode() -> bool {
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get("cbProgressiveMode")));
}

auto ExportDialog::getBackgroundType() -> ExportBackgroundType {
    return (ExportBackgroundType)gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbBackgroundType")));
}

auto ExportDialog::getRange() -> PageRangeVector {
    GtkWidget* rdRangeCurrent = get("rdRangeCurrent");
    GtkWidget* rdRangePages = get("rdRangePages");

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangePages))) {
        return ElementRange::parse(gtk_entry_get_text(GTK_ENTRY(get("txtPages"))), this->pageCount);
    }
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangeCurrent))) {
        PageRangeVector range;
        range.emplace_back(this->currentPage - 1, this->currentPage - 1);
        return range;
    }

    PageRangeVector range;
    range.emplace_back(0, this->pageCount - 1);
    return range;
}

void ExportDialog::show(GtkWindow* parent) {
    confirmed = false;

    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);

    int res = gtk_dialog_run(GTK_DIALOG(this->window));

    if (res == GTK_RESPONSE_OK) {
        confirmed = true;
    }

    gtk_widget_hide(this->window);
}
