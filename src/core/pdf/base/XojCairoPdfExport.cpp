#include "XojCairoPdfExport.h"

#include <algorithm>  // for copy, min
#include <map>        // for map
#include <memory>     // for __shared_ptr_access
#include <sstream>    // for ostringstream, operator<<
#include <stack>      // for stack
#include <utility>    // for pair, make_pair
#include <vector>     // for vector

#include <cairo-pdf.h>    // for cairo_pdf_surface_set_met...
#include <glib-object.h>  // for g_object_unref

#include "control/jobs/ProgressListener.h"  // for ProgressListener
#include "model/Document.h"                 // for Document
#include "model/Layer.h"                    // for Layer
#include "model/LinkDestination.h"          // for LinkDestination, XojLinkDest
#include "model/PageRef.h"                  // for PageRef
#include "model/PageType.h"                 // for PageType
#include "model/XojPage.h"                  // for XojPage
#include "pdf/base/XojPdfPage.h"            // for XojPdfPageSPtr, XojPdfPage
#include "util/Assert.h"                    // for xoj_assert
#include "util/Util.h"                      // for npos
#include "util/i18n.h"                      // for _
#include "util/serdesstream.h"              // for serdes_stream
#include "view/DocumentView.h"              // for DocumentView

#include "config.h"      // for PROJECT_STRING
#include "filesystem.h"  // for path

XojCairoPdfExport::XojCairoPdfExport(Document* doc, ProgressListener* progressListener):
        doc(doc), progressListener(progressListener) {}

XojCairoPdfExport::~XojCairoPdfExport() {
    if (this->surface != nullptr) {
        endPdf();
    }
}

/**
 * Export without background
 */
void XojCairoPdfExport::setExportBackground(ExportBackgroundType exportBackground) {
    this->exportBackground = exportBackground;
}

auto XojCairoPdfExport::startPdf(const fs::path& file, bool exportOutline) -> bool {
    this->surface = cairo_pdf_surface_create(file.u8string().c_str(), 0, 0);
    this->cr = cairo_create(surface);

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 16, 0)
    cairo_pdf_surface_set_metadata(surface, CAIRO_PDF_METADATA_TITLE, doc->getFilepath().filename().u8string().c_str());
    cairo_pdf_surface_set_metadata(surface, CAIRO_PDF_METADATA_CREATOR, PROJECT_STRING);
    if (exportOutline) {
        this->populatePdfOutline();
    }
#endif

    // Turn on font hint metrics, for consistency with text display in the app
    cairo_font_options_t* fontOptions = cairo_font_options_create();
    cairo_font_options_set_hint_metrics(fontOptions, CAIRO_HINT_METRICS_ON);
    cairo_set_font_options(cr, fontOptions);
    cairo_font_options_destroy(fontOptions);

    return cairo_surface_status(this->surface) == CAIRO_STATUS_SUCCESS;
}

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 16, 0)
void XojCairoPdfExport::populatePdfOutline() {
    auto tocModel = doc->getContentsModel();
    if (tocModel == nullptr)
        return;

    int idCounter = CAIRO_PDF_OUTLINE_ROOT;
    std::stack<std::pair<GtkTreeIter, int>> nodeStack;

    GtkTreeIter firstIter = {0};
    if (!gtk_tree_model_get_iter_first(tocModel, &firstIter)) {
        // Outline is empty, so do nothing.
        return;
    }

    nodeStack.push(std::make_pair(firstIter, idCounter));
    while (!nodeStack.empty()) {
        auto [iter, parentId] = nodeStack.top();
        nodeStack.pop();
        const int currentId = ++idCounter;
        XojLinkDest* link = nullptr;

        gtk_tree_model_get(tocModel, &iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);
        auto dest = link->dest;
        auto pdfBgPage = dest->getPdfPage();  // Link destination in original background PDF
        auto pageDest = pdfBgPage == npos ? npos : doc->findPdfPage(pdfBgPage);  // Destination in document
        if (pageDest != npos) {
            auto linkAttrBuf = serdes_stream<std::ostringstream>();
            linkAttrBuf << "page=" << pageDest + 1;
            if (dest->shouldChangeLeft() && dest->shouldChangeTop()) {
                linkAttrBuf << " pos=[" << dest->getLeft() << " " << dest->getTop() << "]";
            }
            const auto linkAttr = linkAttrBuf.str();
            auto outlineFlags = dest->getExpand() ? CAIRO_PDF_OUTLINE_FLAG_OPEN : 0;
            cairo_pdf_surface_add_outline(this->surface, parentId, link->dest->getName().data(), linkAttr.data(),
                                          static_cast<cairo_pdf_outline_flags_t>(outlineFlags));
        }
        g_object_unref(link);

        GtkTreeIter childIter;
        if (gtk_tree_model_iter_children(tocModel, &childIter, &iter)) {
            nodeStack.push(std::make_pair(childIter, currentId));
        }

        if (gtk_tree_model_iter_next(tocModel, &iter)) {
            nodeStack.push(std::make_pair(iter, parentId));
        }
    }
}
#endif

bool XojCairoPdfExport::endPdf() {
    cairo_surface_finish(this->surface);
    bool success = cairo_surface_status(this->surface) == CAIRO_STATUS_SUCCESS;
    if (!success) {
        this->lastError = _("Error while finalizing the PDF Cairo surface");
        this->lastError += "\nCairo error: ";
        this->lastError += cairo_status_to_string(cairo_surface_status(this->surface));
    }
    cairo_destroy(this->cr);
    this->cr = nullptr;
    cairo_surface_destroy(this->surface);
    this->surface = nullptr;
    return success;
}

void XojCairoPdfExport::exportPage(size_t page) {
    PageRef p = doc->getPage(page);

    cairo_pdf_surface_set_size(this->surface, p->getWidth(), p->getHeight());

    DocumentView view;

    cairo_save(this->cr);

    // For a better pdf quality, we use a dedicated pdf rendering
    if (p->getBackgroundType().isPdfPage() && (exportBackground != EXPORT_BACKGROUND_NONE)) {
        auto pgNo = p->getPdfPageNr();
        XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);

        popplerPage->renderForPrinting(cr);
    }

    xoj::view::BackgroundFlags flags;
    flags.showPDF = xoj::view::HIDE_PDF_BACKGROUND;  // Already exported (if any)
    flags.showImage = exportBackground == EXPORT_BACKGROUND_NONE ? xoj::view::HIDE_IMAGE_BACKGROUND :
                                                                   xoj::view::SHOW_IMAGE_BACKGROUND;
    flags.showRuling = exportBackground <= EXPORT_BACKGROUND_UNRULED ? xoj::view::HIDE_RULING_BACKGROUND :
                                                                       xoj::view::SHOW_RULING_BACKGROUND;

    if (layerRange) {
        view.drawLayersOfPage(*layerRange, p, this->cr, true /* dont render eraseable */, flags);
    } else {
        view.drawPage(p, this->cr, true /* dont render eraseable */, flags);
    }

    // next page
    cairo_show_page(this->cr);
    cairo_restore(this->cr);
}

// export layers one by one to produce as many PDF pages as there are layers.
void XojCairoPdfExport::exportPageLayers(size_t page) {
    PageRef p = doc->getPage(page);

    // We keep a copy of the layers initial Visible state
    std::map<Layer*, bool> initialVisibility;
    for (const auto& layer: *p->getLayers()) {
        initialVisibility[layer] = layer->isVisible();
        layer->setVisible(false);
    }

    // We draw as many pages as there are layers. The first page has
    // only Layer 1 visible, the last has all layers visible.
    for (const auto& layer: *p->getLayers()) {
        layer->setVisible(true);
        exportPage(page);
    }

    // We restore the initial visibilities
    for (const auto& layer: *p->getLayers()) layer->setVisible(initialVisibility[layer]);
}

auto XojCairoPdfExport::createPdf(fs::path const& file, const PageRangeVector& range, bool progressiveMode) -> bool {
    if (range.empty()) {
        this->lastError = _("No pages to export!");
        return false;
    }

    // Only export the outline if we are exporting the entire document. otherwise, links may point to page numbers
    // greater than the total number of pages, leading to corrupt pdf files
    bool outline = range.size() == 1 && range.front().first == 0 && range.front().last >= doc->getPageCount() - 1;

    if (!startPdf(file, outline)) {
        this->lastError = _("Failed to initialize PDF Cairo surface");
        this->lastError += "\nCairo error: ";
        this->lastError += cairo_status_to_string(cairo_surface_status(this->surface));
        return false;
    }

    size_t count = 0;
    for (const auto& e: range) {
        xoj_assert(e.last >= e.first);  // Ok, when the PageRangeVector was the result of parsing
        count += e.last - e.first + 1;  // Not accurate, if e.last is > doc->getPageCount()
    }

    if (this->progressListener) {
        this->progressListener->setMaximumState(count);
    }

    size_t c = 0;
    for (const auto& e: range) {
        auto max = std::min(e.last, doc->getPageCount());  // Should be e.last for parsed PageRangeVector
        for (size_t i = e.first; i <= max; i++) {
            if (progressiveMode) {
                exportPageLayers(i);
            } else {
                exportPage(i);
            }

            if (this->progressListener) {
                this->progressListener->setCurrentState(++c);
            }
        }
    }

    return endPdf();
}

auto XojCairoPdfExport::createPdf(fs::path const& file, bool progressiveMode) -> bool {
    if (doc->getPageCount() < 1) {
        lastError = _("No pages to export!");
        return false;
    }

    if (!startPdf(file, /* exportOutline */ true)) {
        this->lastError = _("Failed to initialize PDF Cairo surface");
        this->lastError += "\nCairo error: ";
        this->lastError += cairo_status_to_string(cairo_surface_status(this->surface));
        return false;
    }

    auto count = doc->getPageCount();
    if (this->progressListener) {
        this->progressListener->setMaximumState(count);
    }

    for (decltype(count) i = 0; i < count; i++) {
        if (progressiveMode) {
            exportPageLayers(i);
        } else {
            exportPage(i);
        }

        if (this->progressListener) {
            this->progressListener->setCurrentState(i + 1);
        }
    }

    return endPdf();
}

auto XojCairoPdfExport::getLastError() -> std::string { return lastError; }

void XojCairoPdfExport::setLayerRange(const char* rangeStr) {
    if (rangeStr) {
        // Use no upper bound for layer indices, as the maximal value can vary between pages
        layerRange =
                std::make_unique<LayerRangeVector>(ElementRange::parse(rangeStr, std::numeric_limits<size_t>::max()));
    }
}
