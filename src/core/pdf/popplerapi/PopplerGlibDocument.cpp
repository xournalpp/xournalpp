#include "PopplerGlibDocument.h"

#include <memory>    // for make_shared
#include <optional>  // for optional

#include <poppler-document.h>  // for poppler_document_get_n_...

#include "util/PathUtil.h"  // for toUri

#include "PopplerGlibPage.h"                  // for PopplerGlibPage
#include "PopplerGlibPageBookmarkIterator.h"  // for PopplerGlibPageBookmark...
#include "filesystem.h"                       // for path

class XojPdfBookmarkIterator;

using std::string;

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

auto PopplerGlibDocument::equals(XojPdfDocumentInterface* doc) const -> bool {
    return document == (dynamic_cast<PopplerGlibDocument*>(doc))->document;
}

auto PopplerGlibDocument::save(fs::path const& file, GError** error) const -> bool {
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
        document = nullptr;
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

auto PopplerGlibDocument::isLoaded() const -> bool { return this->document != nullptr; }

auto PopplerGlibDocument::getPage(size_t page) const -> XojPdfPageSPtr {
    if (document == nullptr) {
        return nullptr;
    }

    PopplerPage* pg = poppler_document_get_page(document, int(page));
    XojPdfPageSPtr pageptr = std::make_shared<PopplerGlibPage>(pg, document);
    g_object_unref(pg);

    return pageptr;
}

auto PopplerGlibDocument::getPageCount() const -> size_t {
    if (document == nullptr) {
        return 0;
    }

    return size_t(poppler_document_get_n_pages(document));
}

auto PopplerGlibDocument::getContentsIter() const -> XojPdfBookmarkIterator* {
    if (document == nullptr) {
        return nullptr;
    }

    PopplerIndexIter* iter = poppler_index_iter_new(document);

    if (iter == nullptr) {
        return nullptr;
    }

    return new PopplerGlibPageBookmarkIterator(iter, document);
}
