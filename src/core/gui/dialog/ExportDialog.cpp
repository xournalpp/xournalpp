#include "ExportDialog.h"

#include <algorithm>  // for max
#include <stdexcept>  // for invalid_argument
#include <string>     // for string, to_string, operator+
#include <vector>     // for allocator

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect

#include "control/jobs/ExportParameters.h"
#include "gui/Builder.h"
#include "util/ElementRange.h"  // for parse, PageRangeVector
#include "util/gtk4_helper.h"

class GladeSearchpath;

constexpr auto UI_FILE = "exportSettings.glade";
constexpr auto UI_DIALOG_NAME = "exportDialog";

using namespace xoj::popup;

static ExportFormat readFormat(GtkComboBox* cb) {
    const char* id = gtk_combo_box_get_active_id(cb);
    if (!id) {
        return EXPORT_GRAPHICS_UNDEFINED;
    } else if (std::string_view sv(id); sv == "svg") {
        return EXPORT_GRAPHICS_SVG;
    } else if (sv == "png") {
        return EXPORT_GRAPHICS_PNG;
    } else if (sv == "pdf") {
        return EXPORT_GRAPHICS_PDF;
    } else if (sv == "xoj") {
        return EXPORT_XOJ;
    }
    g_warning("Unable to convert id to export format: \"%s\"", id);
    return EXPORT_GRAPHICS_UNDEFINED;
}

ExportDialog::ExportDialog(GladeSearchpath* gladeSearchPath, size_t currentPage, size_t pageCount,
                           bool hasPdfBackground, std::function<void(std::unique_ptr<ExportParameters>)> callbackFun):
        currentPage(currentPage), pageCount(pageCount), builder(gladeSearchPath, UI_FILE), callbackFun(callbackFun) {
    window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));

    gtk_label_set_text(GTK_LABEL(builder.get("lbAllPagesInfo")), ("1 - " + std::to_string(pageCount)).c_str());
    gtk_label_set_text(GTK_LABEL(builder.get("lbCurrentPage")), std::to_string(currentPage).c_str());

#if GTK_MAJOR_VERSION == 3
    // Widgets are visible by default in gtk4
    gtk_widget_show_all(builder.get("dialog-main-box"));
#endif

    auto visibilityCb = +[](GtkComboBox* cb, ExportDialog* self) {
        auto fmt = readFormat(cb);
        bool isPNG = fmt == EXPORT_GRAPHICS_PNG;
        bool isPDF = fmt == EXPORT_GRAPHICS_PDF;
        gtk_widget_set_visible(self->builder.get("lbQuality"), isPNG);
        gtk_widget_set_visible(self->builder.get("boxQuality"), isPNG);
        gtk_widget_set_visible(self->builder.get("cbQuality"), isPNG);
        gtk_widget_set_visible(self->builder.get("cbProgressiveMode"), isPDF);
        gtk_widget_set_visible(self->builder.get("boxPdfBackend"), isPDF);
    };
    visibilityCb(GTK_COMBO_BOX(builder.get("cbtExportFormat")), this);
    g_signal_connect(builder.get("cbtExportFormat"), "changed", G_CALLBACK(visibilityCb), this);

    //  Populate the PDF backend menu
    auto* cbt = GTK_COMBO_BOX_TEXT(builder.get("cbtPdfExportBackend"));
    for (auto&& [id, name]: PdfExportBackend::getPrettyNamesOfAvailableBackends()) {
        gtk_combo_box_text_append(cbt, id, name);
    }
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(cbt), PdfExportBackend::DEFAULT_ID_STRING);
    if (!hasPdfBackground) {
        // No need for backend selection if no pdf background is present.
        gtk_widget_set_sensitive(builder.get("boxPdfBackend"), false);
    }

    // rdRangePages toggled signal handler
    //
    // Sets and unsets the sensitivity of the text form, OK button and
    // displays an error when the text form is selected and contains
    // invalid user data. The error goes away when the text form is de-selected.
    auto toggledHandler = G_CALLBACK(+[](GtkCheckButton* button, ExportDialog* self) {
        auto active = gtk_check_button_get_active(button);
        auto btOk = self->builder.get("btOk");
        auto txtPages = self->builder.get("txtPages");
        gtk_widget_set_sensitive(txtPages, active);
        if (active) {
            const std::string text_form(gtk_editable_get_text(GTK_EDITABLE(txtPages)));
            try {
                ElementRange::parse(text_form, self->pageCount);
                gtk_widget_remove_css_class(txtPages, "error");
                gtk_widget_set_sensitive(btOk, true);
            } catch (const std::invalid_argument&) {
                gtk_widget_add_css_class(txtPages, "error");
                gtk_widget_set_sensitive(btOk, false);
            }
        } else {
            gtk_widget_remove_css_class(txtPages, "error");
            gtk_widget_set_sensitive(btOk, true);
        }
    });
    // txtPages changed signal handler
    //
    // Displays an error when the user inputs invalid page ranges in the text form,
    // and sets the sensitivity of the OK button to FALSE. These actions are
    // reversed when the text form contains a valid page range.
    auto changedHandler = G_CALLBACK(+[](GtkEditable* txtPages, ExportDialog* self) {
        const std::string text_form(gtk_editable_get_text(txtPages));
        auto btOk = self->builder.get("btOk");
        try {
            ElementRange::parse(text_form, self->pageCount);
            gtk_widget_remove_css_class(GTK_WIDGET(txtPages), "error");
            gtk_widget_set_sensitive(btOk, true);
        } catch (const std::invalid_argument& e) {
            gtk_widget_add_css_class(GTK_WIDGET(txtPages), "error");
            gtk_widget_set_sensitive(btOk, false);
        }
    });

    g_signal_connect(builder.get("rdRangePages"), "toggled", toggledHandler, this);
    g_signal_connect(builder.get("cbQuality"), "changed", G_CALLBACK(ExportDialog::selectQualityCriterion), this);
    g_signal_connect(builder.get("txtPages"), "changed", changedHandler, this);


    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());
    g_signal_connect_swapped(builder.get("btOk"), "clicked", G_CALLBACK(ExportDialog::onSuccessCallback), this);

    /**
     * The callback returns `false` so that the PopupWindowManager callback deleting `this` gets called as well.
     */
#if GTK_MAJOR_VERSION == 3
    g_signal_connect_swapped(window.get(), "delete-event", G_CALLBACK(+[](ExportDialog* self) {
                                 if (self->confirmed && self->parameters) {
                                     self->callbackFun(std::move(self->parameters));
                                 }
                                 return false;
                             }),
                             this);
#else
    g_signal_connect_swapped(window.get(), "close-request", G_CALLBACK(+[](ExportDialog* self) {
                                 if (self->confirmed && self->parameters) {
                                     self->callbackFun(std::move(self->parameters));
                                 }
                                 return false;
                             }),
                             this);
#endif
}

ExportDialog::~ExportDialog() = default;

void ExportDialog::selectQualityCriterion(GtkComboBox* comboBox, ExportDialog* self) {
    int activeCriterion = gtk_combo_box_get_active(comboBox);
    switch (activeCriterion) {
        case EXPORT_QUALITY_DPI:
            gtk_label_set_text(GTK_LABEL(self->builder.get("lbQualityUnit")), "dpi");
            gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(self->builder.get("sbQualityValue")),
                                           GTK_ADJUSTMENT(self->builder.get<GObject>("adjustmentDpi")));
            break;
        case EXPORT_QUALITY_WIDTH:
        case EXPORT_QUALITY_HEIGHT:
            gtk_label_set_text(GTK_LABEL(self->builder.get("lbQualityUnit")), "px");
            gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(self->builder.get("sbQualityValue")),
                                           GTK_ADJUSTMENT(self->builder.get<GObject>("adjustmentHeightWidth")));
            break;
    }
}

void ExportDialog::onSuccessCallback(ExportDialog* self) {
    self->confirmed = true;
    self->parameters = std::make_unique<ExportParameters>();
    self->parameters->progressiveMode =
            gtk_check_button_get_active(GTK_CHECK_BUTTON(self->builder.get("cbProgressiveMode")));
    self->parameters->backgroundType = static_cast<ExportBackgroundType>(
            gtk_combo_box_get_active(GTK_COMBO_BOX(self->builder.get("cbBackgroundType"))));
    self->parameters->pageRanges = [self]() {
        GtkWidget* rdRangeCurrent = self->builder.get("rdRangeCurrent");
        GtkWidget* rdRangePages = self->builder.get("rdRangePages");

        if (gtk_check_button_get_active(GTK_CHECK_BUTTON(rdRangePages))) {
            return ElementRange::parse(gtk_editable_get_text(GTK_EDITABLE(self->builder.get("txtPages"))),
                                       self->pageCount);
        }
        if (gtk_check_button_get_active(GTK_CHECK_BUTTON(rdRangeCurrent))) {
            PageRangeVector range;
            range.emplace_back(self->currentPage - 1, self->currentPage - 1);
            return range;
        }

        PageRangeVector range;
        range.emplace_back(0, self->pageCount - 1);
        return range;
    }();
    self->parameters->pdfExportBackend = PdfExportBackend::fromString(
            gtk_combo_box_get_active_id(GTK_COMBO_BOX(self->builder.get("cbtPdfExportBackend"))));
    self->parameters->qualityParameter = RasterImageQualityParameter(
            static_cast<ExportQualityCriterion>(
                    gtk_combo_box_get_active(GTK_COMBO_BOX(self->builder.get("cbQuality")))),
            gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(self->builder.get("sbQualityValue"))));
    self->parameters->format = readFormat(GTK_COMBO_BOX(self->builder.get("cbtExportFormat")));
    gtk_window_close(self->window.get());
}
