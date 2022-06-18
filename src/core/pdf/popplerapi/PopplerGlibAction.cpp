#include "PopplerGlibAction.h"

#include <algorithm>  // for min

#include <glib.h>              // for g_warning
#include <poppler-action.h>    // for poppler_action_free, poppler_dest_free
#include <poppler-document.h>  // for poppler_document_find_dest, poppler_do...
#include <poppler-page.h>      // for poppler_page_get_size

using std::string;

PopplerGlibAction::PopplerGlibAction(PopplerAction* action, PopplerDocument* document):
        action(action), document(document) {
    g_object_ref(document);
}

PopplerGlibAction::~PopplerGlibAction() {
    poppler_action_free(action);
    action = nullptr;

    if (document) {
        g_object_unref(document);
        document = nullptr;
    }
}

auto PopplerGlibAction::getDestination() -> XojLinkDest* {
    XojLinkDest* dest = link_dest_new();
    dest->dest = new LinkDestination();
    dest->dest->setName(getTitle());

    // every other action is not supported in Xournal
    if (action->type == POPPLER_ACTION_GOTO_DEST) {
        auto* actionDest = reinterpret_cast<PopplerActionGotoDest*>(action);
        PopplerDest* pDest = actionDest->dest;

        if (pDest == nullptr) {
            return dest;
        }

        linkFromDest(dest->dest, pDest);
    }

    return dest;
}

void PopplerGlibAction::linkFromDest(LinkDestination* link, PopplerDest* pDest) {
    switch (pDest->type) {
        case POPPLER_DEST_UNKNOWN:
            g_warning("PDF Contains unknown link destination");
            break;
        case POPPLER_DEST_XYZ: {
            PopplerPage* page = poppler_document_get_page(document, pDest->page_num - 1);
            if (page == nullptr) {
                return;
            }

            double pageWidth = 0;
            double pageHeight = 0;
            poppler_page_get_size(page, &pageWidth, &pageHeight);

            if (pDest->left) {
                link->setChangeLeft(pDest->left);
            } else if (pDest->right) {
                link->setChangeLeft(pageWidth - pDest->right);
            }

            if (pDest->top) {
                link->setChangeTop(pageHeight - std::min(pageHeight, pDest->top));
            } else if (pDest->bottom) {
                link->setChangeTop(pageHeight - std::min(pageHeight, pageHeight - pDest->bottom));
            }

            if (pDest->zoom != 0) {
                link->setChangeZoom(pDest->zoom);
            }
            g_object_unref(page);
        } break;
        case POPPLER_DEST_NAMED: {
            PopplerDest* pDest2 = poppler_document_find_dest(document, pDest->named_dest);
            if (pDest2 != nullptr) {
                linkFromDest(link, pDest2);
                poppler_dest_free(pDest2);
                return;
            }
        } break;

        default:
            break;
    }

    link->setPdfPage(pDest->page_num - 1);
}

auto PopplerGlibAction::getTitle() -> std::string { return (reinterpret_cast<PopplerActionAny*>(action))->title; }
