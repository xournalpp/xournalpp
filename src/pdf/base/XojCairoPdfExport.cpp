#include "XojCairoPdfExport.h"

#include <sstream>
#include <stack>

#include <cairo/cairo-pdf.h>

#include "view/DocumentView.h"

#include "Util.h"
#include "filesystem.h"
#include "i18n.h"

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
void XojCairoPdfExport::setNoBackgroundExport(bool noBackgroundExport) {
    this->noBackgroundExport = noBackgroundExport;
}

auto XojCairoPdfExport::startPdf(const fs::path& file) -> bool {
    this->surface = cairo_pdf_surface_create(file.u8string().c_str(), 0, 0);
    this->cr = cairo_create(surface);

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 16, 0)
    cairo_pdf_surface_set_metadata(surface, CAIRO_PDF_METADATA_TITLE, doc->getFilepath().filename().u8string().c_str());
    GtkTreeModel* tocModel = doc->getContentsModel();
    this->populatePdfOutline(tocModel);
#endif

    return true;
}

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 16, 0)
void XojCairoPdfExport::populatePdfOutline(GtkTreeModel* tocModel) {
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
            std::ostringstream linkAttrBuf;
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

void XojCairoPdfExport::endPdf() {
    cairo_destroy(this->cr);
    this->cr = nullptr;
    cairo_surface_destroy(this->surface);
    this->surface = nullptr;
}

void XojCairoPdfExport::exportPage(size_t page) {
    PageRef p = doc->getPage(page);

    cairo_pdf_surface_set_size(this->surface, p->getWidth(), p->getHeight());

    DocumentView view;

    cairo_save(this->cr);
    if (p->getBackgroundType().isPdfPage() && !noBackgroundExport) {
        int pgNo = p->getPdfPageNr();
        XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);

        popplerPage->render(cr, true);
    }

    view.drawPage(p, this->cr, true /* dont render eraseable */, noBackgroundExport);

    // next page
    cairo_show_page(this->cr);
    cairo_restore(this->cr);
}

auto XojCairoPdfExport::createPdf(fs::path const& file, PageRangeVector& range) -> bool {
    if (range.empty()) {
        this->lastError = _("No pages to export!");
        return false;
    }

    if (!startPdf(file)) {
        return false;
    }

    int count = 0;
    for (PageRangeEntry* e: range) {
        count += e->getLast() - e->getFirst() + 1;
    }

    if (this->progressListener) {
        this->progressListener->setMaximumState(count);
    }

    int c = 0;
    for (PageRangeEntry* e: range) {
        for (int i = e->getFirst(); i <= e->getLast(); i++) {
            if (i < 0 || i >= static_cast<int>(doc->getPageCount())) {
                continue;
            }

            exportPage(i);

            if (this->progressListener) {
                this->progressListener->setCurrentState(c++);
            }
        }
    }

    endPdf();
    return true;
}

auto XojCairoPdfExport::createPdf(fs::path const& file) -> bool {
    if (doc->getPageCount() < 1) {
        lastError = _("No pages to export!");
        return false;
    }

    if (!startPdf(file)) {
        return false;
    }

    int count = doc->getPageCount();
    if (this->progressListener) {
        this->progressListener->setMaximumState(count);
    }

    for (int i = 0; i < count; i++) {
        exportPage(i);

        if (this->progressListener) {
            this->progressListener->setCurrentState(i);
        }
    }

    endPdf();
    return true;
}

auto XojCairoPdfExport::getLastError() -> string { return lastError; }
