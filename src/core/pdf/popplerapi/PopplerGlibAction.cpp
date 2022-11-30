#include "PopplerGlibAction.h"

#include <algorithm>  // for min
#include <cstddef>    // for size_t

#include <glib.h>              // for g_warning, gchar
#include <poppler-action.h>    // for poppler_dest_free, PopplerActi...
#include <poppler-document.h>  // for poppler_document_find_dest
#include <poppler-page.h>      // for poppler_page_get_size

#include "model/LinkDestination.h"     // for LinkDestination
#include "util/Util.h"                 // for npos
#include "util/raii/CLibrariesSPtr.h"  // for ref

using std::string;

PopplerGlibAction::PopplerGlibAction(PopplerAction* action, PopplerDocument* document):
        document(document, xoj::util::ref) {
    if (gchar* title_cstr = reinterpret_cast<PopplerActionAny*>(action)->title) {
        title = std::string{title_cstr};
    }

    destination = getDestination(action);
}

PopplerGlibAction::~PopplerGlibAction() = default;

auto PopplerGlibAction::getDestination(PopplerAction* action) -> std::shared_ptr<const LinkDestination> {
    auto dest = std::make_shared<LinkDestination>();
    dest->setName(getTitle());

    if (action->type == POPPLER_ACTION_URI && action->uri.uri) {
        dest->setURI(std::string{action->uri.uri});
    } else if (action->type == POPPLER_ACTION_GOTO_DEST) {
        auto* actionDest = reinterpret_cast<PopplerActionGotoDest*>(action);
        PopplerDest* pDest = actionDest->dest;

        if (pDest == nullptr) {
            return dest;
        }

        linkFromDest(*dest, pDest);
    } else {
        // Every other action is currently unsupported.
    }

    return dest;
}

auto PopplerGlibAction::getDestination() -> std::shared_ptr<const LinkDestination> { return destination; }

void PopplerGlibAction::linkFromDest(LinkDestination& link, PopplerDest* pDest) {
    switch (pDest->type) {
        case POPPLER_DEST_UNKNOWN:
            g_warning("PDF Contains unknown link destination");
            break;
        case POPPLER_DEST_XYZ: {
            PopplerPage* page = poppler_document_get_page(document.get(), pDest->page_num - 1);
            if (page == nullptr) {
                return;
            }

            double pageWidth = 0;
            double pageHeight = 0;
            poppler_page_get_size(page, &pageWidth, &pageHeight);

            if (pDest->left != 0) {
                link.setChangeLeft(pDest->left);
            } else if (pDest->right != 0) {
                link.setChangeLeft(pageWidth - pDest->right);
            }

            if (pDest->top != 0) {
                link.setChangeTop(pageHeight - std::min(pageHeight, pDest->top));
            } else if (pDest->bottom != 0) {
                link.setChangeTop(pageHeight - std::min(pageHeight, pageHeight - pDest->bottom));
            }

            if (pDest->zoom != 0) {
                link.setChangeZoom(pDest->zoom);
            }
            g_object_unref(page);
        } break;
        case POPPLER_DEST_NAMED: {
            PopplerDest* pDest2 = poppler_document_find_dest(document.get(), pDest->named_dest);
            if (pDest2 != nullptr) {
                linkFromDest(link, pDest2);
                poppler_dest_free(pDest2);
                return;
            }
        } break;

        default:
            break;
    }

    link.setPdfPage(pDest->page_num > 0 ? static_cast<size_t>(pDest->page_num - 1) : npos);
}

auto PopplerGlibAction::getTitle() -> std::string { return title; }
