#include "PopplerGlibDocument.h"

#include <memory>

#include "PathUtil.h"
#include "PopplerGlibPage.h"
#include "PopplerGlibPageBookmarkIterator.h"
#include "Util.h"
#include "filesystem.h"


PopplerGlibDocument::PopplerGlibDocument() = default;

PopplerGlibDocument::PopplerGlibDocument(const PopplerGlibDocument& doc): document(doc.document) {
    if (document) {
        g_object_ref(document);
    }
}

PopplerGlibDocument::~PopplerGlibDocument() {
    if (document) {
        g_object_unref(document);
        document = nullptr;
    }
}

void PopplerGlibDocument::assign(XojPdfDocumentInterface* doc) {
    if (document) {
        g_object_unref(document);
    }

    document = (dynamic_cast<PopplerGlibDocument*>(doc))->document;
    if (document) {
        g_object_ref(document);
    }
}

auto PopplerGlibDocument::equals(XojPdfDocumentInterface* doc) -> bool {
    return document == (dynamic_cast<PopplerGlibDocument*>(doc))->document;
}

auto PopplerGlibDocument::save(fs::path const& file, GError** error) -> bool {
    if (document == nullptr) {
        return false;
    }

    auto uri = Util::toUri(file);
    if (!uri) {
        return false;
    }
    return poppler_document_save(document, uri->c_str(), error);
}

auto PopplerGlibDocument::load(fs::path const& file, string password, GError** error) -> bool {
    auto uri = Util::toUri(file);
    if (!uri) {
        return false;
    }

    if (document) {
        g_object_unref(document);
    }

    this->document = poppler_document_new_from_file(uri->c_str(), password.c_str(), error);
    return this->document != nullptr;
}

auto PopplerGlibDocument::load(gpointer data, gsize length, string password, GError** error) -> bool {
    if (document) {
        g_object_unref(document);
    }

    this->document =
            poppler_document_new_from_data(static_cast<char*>(data), static_cast<int>(length), password.c_str(), error);
    return this->document != nullptr;
}

auto PopplerGlibDocument::isLoaded() -> bool { return this->document != nullptr; }

auto PopplerGlibDocument::getPage(size_t page) -> XojPdfPageSPtr {
    if (document == nullptr) {
        return nullptr;
    }

    PopplerPage* pg = poppler_document_get_page(document, page);
    XojPdfPageSPtr pageptr = std::make_shared<PopplerGlibPage>(pg);
    g_object_unref(pg);

    return pageptr;
}

auto PopplerGlibDocument::getPageCount() -> size_t {
    if (document == nullptr) {
        return 0;
    }

    return poppler_document_get_n_pages(document);
}

auto PopplerGlibDocument::getContentsIter() -> XojPdfBookmarkIterator* {
    if (document == nullptr) {
        return nullptr;
    }

    PopplerIndexIter* iter = poppler_index_iter_new(document);

    if (iter == nullptr) {
        return nullptr;
    }

    return new PopplerGlibPageBookmarkIterator(iter, document);
}
