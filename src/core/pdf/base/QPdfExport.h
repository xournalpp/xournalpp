/*
 * Xournal++
 *
 * PDF Document Export Abstraction Interface - use cairo for the annotations and overlay them on the original PDF using
 * QPDF to avoid information loss
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include "config-features.h"

#ifdef ENABLE_QPDF

#include <sstream>

#include "HybridPdfExport.h"  // for HybridPdfExport
#include "filesystem.h"       // for path

class Document;
class ProgressListener;

class QPdfExport: public HybridPdfExport {
public:
    QPdfExport(const Document* doc, ProgressListener* progressListener);
    ~QPdfExport() override;

protected:
    bool overlayAndSave(const fs::path& saveDestination, std::stringstream& overlaystream,
                        const std::vector<OutputPageInfo>& outputPageInfos) override;
};

#endif
