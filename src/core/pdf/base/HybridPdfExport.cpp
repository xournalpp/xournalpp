#include "HybridPdfExport.h"

#include <algorithm>  // for min
#include <ctime>      // for time_t
#include <sstream>    // for stringstream
#include <vector>     // for vector

#include <cairo-pdf.h>  // for cairo_pdf_surface_create_for_stream

#include "control/jobs/ProgressListener.h"  // for ProgressListener
#include "model/Document.h"                 // for Document
#include "model/PageRef.h"                  // for PageRef
#include "model/XojPage.h"                  // for XojPage
#include "util/Assert.h"                    // for xoj_assert
#include "util/i18n.h"                      // for _
#include "util/serdesstream.h"              // for serdes_stream

#include "filesystem.h"  // for path


HybridPdfExport::HybridPdfExport(const Document* doc, ProgressListener* progressListener):
        XojCairoPdfExport(doc, progressListener) {}

HybridPdfExport::~HybridPdfExport() = default;

static cairo_status_t writeFun(void* stream, const unsigned char* data, unsigned int length) {
    *static_cast<std::stringstream*>(stream) << std::string_view((char*)data, length);
    return CAIRO_STATUS_SUCCESS;
}

auto HybridPdfExport::startPdf(std::stringstream& stream) -> bool {
    this->surface = cairo_pdf_surface_create_for_stream(writeFun, &stream, 0, 0);
    this->cr = cairo_create(surface);

    configureCairoFontOptions();

    return cairo_surface_status(this->surface) == CAIRO_STATUS_SUCCESS;
}

auto HybridPdfExport::createPdf(fs::path const& file, const PageRangeVector& range, bool progressiveMode) -> bool {
    if (progressiveMode || exportBackground == EXPORT_BACKGROUND_NONE) {
        // For progressive mode or without any background, cairo export seems enough.
        return XojCairoPdfExport::createPdf(file, range, progressiveMode);
    }
    if (range.empty()) {
        this->lastError = _("No pages to export!");
        return false;
    }

    // Export the annotations to a PDF stream via cairo
    auto stream = serdes_stream<std::stringstream>();

    if (!startPdf(stream)) {
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

    size_t progress = 0;
    std::vector<OutputPageInfo> overlayToBackgroundIndex;
    overlayToBackgroundIndex.reserve(count);
    for (const auto& e: range) {
        auto max = std::min(e.last, doc->getPageCount() - 1);  // Should be e.last for parsed PageRangeVector
        for (size_t i = e.first; i <= max; i++) {

            PageRef p = doc->getPage(i);
            if (p->isAnnotated() || p->getPdfPageNr() == npos) {
                // Pages with only a PDF background and no annotations will be copied directly
                exportPage(i, false /* omit background if PDF */);
                overlayToBackgroundIndex.push_back({true, doc->getPage(i)->getPdfPageNr()});
            } else {
                overlayToBackgroundIndex.push_back({false, doc->getPage(i)->getPdfPageNr()});
            }

            if (this->progressListener) {
                this->progressListener->setCurrentState(++progress);
            }
        }
    }

    if (!endPdf()) {
        return false;
    }

    return overlayAndSave(file, stream, overlayToBackgroundIndex);
}

auto HybridPdfExport::createPdf(fs::path const& file, bool progressiveMode) -> bool {
    PageRangeVector range = {{0, doc->getPageCount() - 1}};
    return createPdf(file, range, progressiveMode);
}

auto HybridPdfExport::countOccurrences(size_t backgroundPageCount, const std::vector<OutputPageInfo>& outputPageInfos)
        -> std::vector<Occurrences> {
    std::vector<Occurrences> occurrences(backgroundPageCount);
    for (auto [hasOverlay, n]: outputPageInfos) {
        if (n != npos) {
            if (n >= backgroundPageCount) {
                throw std::out_of_range(_("PDF page number is out of range"));
            }
            occurrences[n].number++;
            occurrences[n].hasOverlay = occurrences[n].hasOverlay | hasOverlay;
        }
    }
    return occurrences;
}

auto HybridPdfExport::createPDFDateStringForNow() -> std::string {
    std::time_t now = std::time(nullptr);
    auto* time = std::gmtime(&now);
    std::string buf(30, 0);
    if (strftime(buf.data(), 30, "D:%Y%m%d%H%M%SZ", time)) {  // See PDF 1.7 specs - section 7.9.4
        return buf;
    }
    return std::string();
}
