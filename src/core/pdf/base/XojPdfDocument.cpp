#include "XojPdfDocument.h"

#include "pdf/base/XojPdfDocumentInterface.h"    // for XojPdfDocumentInterface
#include "pdf/base/XojPdfPage.h"                 // for XojPdfPageSPtr
#include "pdf/popplerapi/PopplerGlibDocument.h"  // for PopplerGlibDocument

#include "filesystem.h"  // for path

class XojPdfBookmarkIterator;

XojPdfDocument::XojPdfDocument(): doc(new PopplerGlibDocument()) {}

XojPdfDocument::XojPdfDocument(const XojPdfDocument& doc): doc(new PopplerGlibDocument()) {
    this->doc->assign(doc.doc);
}

XojPdfDocument::~XojPdfDocument() {
    delete doc;
    doc = nullptr;
}

auto XojPdfDocument::operator=(const XojPdfDocument& doc) -> XojPdfDocument& {
    this->doc->assign(doc.doc);
    return *this;
}

auto XojPdfDocument::operator==(XojPdfDocument& doc) const -> bool { return this->doc->equals(doc.doc); }

void XojPdfDocument::assign(XojPdfDocumentInterface* doc) { this->doc->assign(doc); }

auto XojPdfDocument::equals(XojPdfDocumentInterface* doc) const -> bool { return this->doc->equals(doc); }

auto XojPdfDocument::save(fs::path const& file, GError** error) const -> bool { return doc->save(file, error); }

auto XojPdfDocument::load(fs::path const& file, std::string password, GError** error) -> bool {
    return doc->load(file, password, error);
}

auto XojPdfDocument::load(gpointer data, gsize length, std::string password, GError** error) -> bool {
    return doc->load(data, length, password, error);
}

auto XojPdfDocument::isLoaded() const -> bool { return doc->isLoaded(); }

auto XojPdfDocument::getPage(size_t page) const -> XojPdfPageSPtr { return doc->getPage(page); }

auto XojPdfDocument::getPageCount() const -> size_t { return doc->getPageCount(); }

auto XojPdfDocument::getContentsIter() const -> XojPdfBookmarkIterator* { return doc->getContentsIter(); }
