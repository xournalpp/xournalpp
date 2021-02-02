#include "PopplerGlibAction.h"

using std::string;

PopplerGlibAction::PopplerGlibAction(PopplerAction* action, PopplerDocument* document): document(document) {
    g_object_ref(document);

    gchar* title_cstr = (reinterpret_cast<PopplerActionAny*>(action))->title;

    if (title_cstr != nullptr) {
        title = std::string{title_cstr};
    }

    destination = getDestination(action);
}

PopplerGlibAction::~PopplerGlibAction() {
    if (document) {
        g_object_unref(document);
        document = nullptr;
    }
}

auto PopplerGlibAction::getDestination(PopplerAction* action) -> std::shared_ptr<const LinkDestination> {
    std::shared_ptr<LinkDestination> dest = std::make_shared<LinkDestination>();
    dest->setName(getTitle());

    // every other action is not supported in Xournal
    if (action->type == POPPLER_ACTION_URI && action->uri.uri) {
        dest->setURI(std::string{action->uri.uri});
    } else if (action->type == POPPLER_ACTION_GOTO_DEST) {
        auto* actionDest = reinterpret_cast<PopplerActionGotoDest*>(action);
        PopplerDest* pDest = actionDest->dest;

        if (pDest == nullptr) {
            return dest;
        }

        linkFromDest(*dest, pDest);
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
            PopplerPage* page = poppler_document_get_page(document, pDest->page_num - 1);
            if (page == nullptr) {
                return;
            }

            double pageWidth = 0;
            double pageHeight = 0;
            poppler_page_get_size(page, &pageWidth, &pageHeight);

            if (pDest->left) {
                link.setChangeLeft(pDest->left);
            } else if (pDest->right) {
                link.setChangeLeft(pageWidth - pDest->right);
            }

            if (pDest->top) {
                link.setChangeTop(pageHeight - std::min(pageHeight, pDest->top));
            } else if (pDest->bottom) {
                link.setChangeTop(pageHeight - std::min(pageHeight, pageHeight - pDest->bottom));
            }

            if (pDest->zoom != 0) {
                link.setChangeZoom(pDest->zoom);
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

    link.setPdfPage(pDest->page_num - 1);
}

auto PopplerGlibAction::getTitle() -> std::string { return title; }
