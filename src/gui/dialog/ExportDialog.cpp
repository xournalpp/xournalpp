#include "ExportDialog.h"

#include <config.h>

#include "PageRange.h"
#include "i18n.h"

ExportDialog::ExportDialog(GladeSearchpath* gladeSearchPath):
        GladeGui(gladeSearchPath, "exportSettings.glade", "exportDialog") {

    g_signal_connect(get("rdRangePages"), "toggled", G_CALLBACK(+[](GtkToggleButton* togglebutton, ExportDialog* self) {
                         gtk_widget_set_sensitive(self->get("txtPages"), gtk_toggle_button_get_active(togglebutton));
                     }),
                     this);

    g_signal_connect(get("cbQuality"), "changed", G_CALLBACK(ExportDialog::selectQualityCriterion), this);


    GSList* radios = gtk_radio_button_get_group(GTK_RADIO_BUTTON(get("rdRangeAll")));
    for (GSList* head = radios; head != nullptr; head = head->next) {
        g_signal_connect(reinterpret_cast<GtkRadioButton*>(head->data), "activate",
                         G_CALLBACK(+[](GtkButton*, ExportDialog* self) {
                             gtk_dialog_response(GTK_DIALOG(self->window), GTK_RESPONSE_OK);
                         }),
                         this);
    }
}

ExportDialog::~ExportDialog() = default;

void ExportDialog::initPages(int current, int count) {
    string allPages = "1 - " + std::to_string(count);
    gtk_label_set_text(GTK_LABEL(get("lbAllPagesInfo")), allPages.c_str());
    string currentPages = std::to_string(current);
    gtk_label_set_text(GTK_LABEL(get("lbCurrentPage")), currentPages.c_str());

    this->currentPage = current;
    this->pageCount = count;
}

void ExportDialog::removeQualitySetting() {
    gtk_widget_hide(get("lbQuality"));
    gtk_widget_hide(get("boxQuality"));
    gtk_widget_hide(get("cbQuality"));
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
            gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("sbQualityValue"))));
}

auto ExportDialog::isConfirmed() const -> bool { return this->confirmed; }

auto ExportDialog::getRange() -> PageRangeVector {
    GtkWidget* rdRangeCurrent = get("rdRangeCurrent");
    GtkWidget* rdRangePages = get("rdRangePages");

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangePages))) {
        return PageRange::parse(gtk_entry_get_text(GTK_ENTRY(get("txtPages"))), this->pageCount);
    }
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangeCurrent))) {
        PageRangeVector range;
        range.push_back(new PageRangeEntry(this->currentPage - 1, this->currentPage - 1));
        return range;
    }


    PageRangeVector range;
    range.push_back(new PageRangeEntry(0, this->pageCount - 1));
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
