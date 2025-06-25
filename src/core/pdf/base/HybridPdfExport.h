/*
 * Xournal++
 *
 * PDF Document Export Abstraction Interface - uses cairo for the annotations and overlay them on the original PDF using
 * another PDF library
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cstddef>  // for size_t
#include <sstream>

#include "util/ElementRange.h"  // for PageRangeVector

#include "XojCairoPdfExport.h"  // for XojPdfExport
#include "filesystem.h"         // for path

class Document;
class ProgressListener;

class HybridPdfExport: public XojCairoPdfExport {
public:
    HybridPdfExport(const Document* doc, ProgressListener* progressListener);
    ~HybridPdfExport() override;

public:
    bool createPdf(fs::path const& file, bool progressiveMode) override;
    bool createPdf(fs::path const& file, const PageRangeVector& range, bool progressiveMode) override;

    struct OutputPageInfo {
        bool hasOverlay;
        size_t pdfBackgroundPageNumber;
    };
    /// Occurrences of a given background page
    struct Occurrences {
        size_t number = 0;
        bool hasOverlay = false;
    };
    // Count occurrences of background pages: the user might have duplicated or removed pages
    static std::vector<Occurrences> countOccurrences(size_t backgroundPageCount,
                                                     const std::vector<OutputPageInfo>& outputPageInfos);

protected:
    bool startPdf(std::stringstream& stream);
    virtual bool overlayAndSave(const fs::path& saveDestination, std::stringstream& overlaystream,
                                const std::vector<OutputPageInfo>& outputPageInfos) = 0;
    static std::string createPDFDateStringForNow();  // See PDF 1.7 specs - section 7.9.4
};
