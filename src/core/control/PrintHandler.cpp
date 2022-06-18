#include "PrintHandler.h"

#include <cmath>   // for M_PI_2
#include <memory>  // for __shared_ptr_access
#include <string>  // for string

#include <cairo.h>        // for cairo_rotate, cairo_translate, cairo_t
#include <config-dev.h>   // for PRINT_CONFIG_FILE
#include <glib-object.h>  // for g_object_unref, G_CALLBACK, g_signa...
#include <glib.h>         // for GError, g_error_free, g_warning

#include "model/Document.h"       // for Document
#include "model/PageRef.h"        // for PageRef
#include "model/PageType.h"       // for PageType
#include "model/XojPage.h"        // for XojPage
#include "pdf/base/XojPdfPage.h"  // for XojPdfPageSPtr, XojPdfPage
#include "util/PathUtil.h"        // for getConfigFile
#include "util/XojMsgBox.h"       // for XojMsgBox
#include "util/i18n.h"            // for _
#include "util/safe_casts.h"      // for strict_cast
#include "view/DocumentView.h"    // for DocumentView

#include "filesystem.h"  // for exists, remove, path

namespace {
void drawPage(GtkPrintOperation* /*operation*/, GtkPrintContext* context, int pageNr, Document* doc) {
    cairo_t* cr = gtk_print_context_get_cairo_context(context);

    PageRef page = doc->getPage(static_cast<size_t>(pageNr));
    if (!page) {
        return;
    }

    double width = page->getWidth();
    double height = page->getHeight();

    if (width > height) {
        cairo_rotate(cr, M_PI_2);
        cairo_translate(cr, 0, -height);
    }

    // For better quality printing, we use a dedicated pdf-renderer in this case
    if (page->getBackgroundType().isPdfPage()) {
        XojPdfPageSPtr popplerPage = doc->getPdfPage(page->getPdfPageNr());
        if (popplerPage) {
            popplerPage->renderForPrinting(cr);
        }
    }

    DocumentView view;
    view.drawPage(page, cr, true /* dont render eraseable */, true /* dont show pdf background*/);
}

void requestPageSetup(GtkPrintOperation* /*op*/, GtkPrintContext* /*ctx*/, int pageNr, GtkPageSetup* setup,
                      Document* doc) {
    PageRef page = doc->getPage(static_cast<size_t>(pageNr));  // Can't be negative
    if (!page) {
        return;
    }

    double width = page->getWidth();
    double height = page->getHeight();

    if (width > height) {
        gtk_page_setup_set_orientation(setup, GTK_PAGE_ORIENTATION_LANDSCAPE);
    } else {
        gtk_page_setup_set_orientation(setup, GTK_PAGE_ORIENTATION_PORTRAIT);
    }

    GtkPaperSize* size = gtk_paper_size_new_custom("xoj-internal", "xoj-internal", width, height, GTK_UNIT_POINTS);
    gtk_page_setup_set_paper_size(setup, size);
    gtk_paper_size_free(size);
}

inline void handlePrintError(GError*& error, const char* message) {
    if (error) {
        g_warning(message, error->message);
        g_error_free(error);
        error = nullptr;
    }
}
}  // namespace

// Todo: maybe loop over this twice,
void PrintHandler::print(Document* doc, size_t currentPage, GtkWindow* parent) {
    GtkPrintSettings* settings{};
    auto filepath = Util::getConfigFile(PRINT_CONFIG_FILE);
    if (fs::exists(filepath)) {
        GError* error{};
        settings = gtk_print_settings_new_from_file(filepath.u8string().c_str(), &error);
        handlePrintError(error, "Loading print settings failed with: %s");
        fs::remove(filepath);
    }
    if (settings == nullptr) {
        settings = gtk_print_settings_new();
    }

    GtkPrintOperation* op = gtk_print_operation_new();
    gtk_print_operation_set_print_settings(op, settings);
    gtk_print_operation_set_n_pages(op, strict_cast<int>(doc->getPageCount()));
    gtk_print_operation_set_current_page(op, strict_cast<int>(currentPage));
    gtk_print_operation_set_job_name(op, "Xournal++");
    gtk_print_operation_set_unit(op, GTK_UNIT_POINTS);
    gtk_print_operation_set_use_full_page(op, true);
    g_signal_connect(op, "draw_page", G_CALLBACK(drawPage), doc);
    g_signal_connect(op, "request-page-setup", G_CALLBACK(requestPageSetup), doc);

    GError* error{};
    GtkPrintOperationResult res = gtk_print_operation_run(op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, parent, &error);
    g_object_unref(settings);
    if (GTK_PRINT_OPERATION_RESULT_APPLY == res) {
        settings = gtk_print_operation_get_print_settings(op);
        gtk_print_settings_to_file(settings, filepath.u8string().c_str(), nullptr);
    } else if (GTK_PRINT_OPERATION_RESULT_ERROR == res) {
        constexpr auto msg = "Running print operation failed with %s";
        XojMsgBox::showErrorToUser(nullptr, _(msg));
        handlePrintError(error, msg);
    }

    g_object_unref(op);
}
