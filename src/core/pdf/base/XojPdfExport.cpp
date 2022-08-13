#include "XojPdfExport.h"

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
#include "util/Util.h"                      // for npos
#include "util/i18n.h"                      // for _
#include "util/serdesstream.h"              // for serdes_stream
#include "view/DocumentView.h"              // for DocumentView

#include "config.h"      // for PROJECT_STRING
#include "filesystem.h"  // for path

XojPdfExport::XojPdfExport(Document* doc, fs::path filePath): ExportTemplate{doc, std::move(filePath)} {
    createCairoCr(0.0, 0.0);
}

XojPdfExport::~XojPdfExport() {
    if (surface) {
        freeCairoResources();
    }
}

auto XojPdfExport::createCairoCr(double width = 0.0, double height = 0.0) -> bool {
    surface = cairo_pdf_surface_create(filePath.u8string().c_str(), width, height);
    cr = cairo_create(surface);

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 16, 0)
    cairo_pdf_surface_set_metadata(surface, CAIRO_PDF_METADATA_TITLE, doc->getFilepath().filename().u8string().c_str());
    cairo_pdf_surface_set_metadata(surface, CAIRO_PDF_METADATA_CREATOR, PROJECT_STRING);
    GtkTreeModel* tocModel = doc->getContentsModel();
    populatePdfOutline(tocModel);
#endif

    return cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS;
}

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 16, 0)
void XojPdfExport::populatePdfOutline(GtkTreeModel* tocModel) {
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

auto XojPdfExport::configureCairoResourcesForPage(const PageRef page) -> bool {
    cairo_pdf_surface_set_size(surface, page->getWidth(), page->getHeight());
    cairo_save(cr);
    return true;
}

auto XojPdfExport::clearCairoConfig() -> bool {
    cairo_show_page(cr);
    cairo_restore(cr);
    return true;
}
