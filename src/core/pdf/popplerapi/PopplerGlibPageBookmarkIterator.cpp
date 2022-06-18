#include "PopplerGlibPageBookmarkIterator.h"

#include <poppler-document.h>  // for poppler_index_iter_free

#include "pdf/popplerapi/PopplerGlibAction.h"  // for PopplerGlibAction

class XojPdfAction;

PopplerGlibPageBookmarkIterator::PopplerGlibPageBookmarkIterator(PopplerIndexIter* iter, PopplerDocument* document):
        iter(iter), document(document) {
    g_object_ref(document);
}

PopplerGlibPageBookmarkIterator::~PopplerGlibPageBookmarkIterator() {
    poppler_index_iter_free(iter);
    iter = nullptr;

    if (document) {
        g_object_unref(document);
        document = nullptr;
    }
}

auto PopplerGlibPageBookmarkIterator::next() -> bool { return poppler_index_iter_next(iter); }

auto PopplerGlibPageBookmarkIterator::isOpen() -> bool { return poppler_index_iter_is_open(iter); }

auto PopplerGlibPageBookmarkIterator::getChildIter() -> XojPdfBookmarkIterator* {
    PopplerIndexIter* child = poppler_index_iter_get_child(iter);
    if (child == nullptr) {
        return nullptr;
    }

    return new PopplerGlibPageBookmarkIterator(child, document);
}

auto PopplerGlibPageBookmarkIterator::getAction() -> XojPdfAction* {
    PopplerAction* action = poppler_index_iter_get_action(iter);

    if (action == nullptr) {
        return nullptr;
    }

    return new PopplerGlibAction(action, document);
}
