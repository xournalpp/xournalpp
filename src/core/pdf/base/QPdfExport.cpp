#include "QPdfExport.h"

#ifdef ENABLE_QPDF

#include <algorithm>
#include <sstream>  // for ostringstream
#include <vector>   // for vector

#include <qpdf/DLL.h>
#if QPDF_MAJOR_VERSION == 11
#define POINTERHOLDER_TRANSITION 4  // Only used for QPDF 11
#endif
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>
#include <qpdf/QPDFWriter.hh>

#include "model/Document.h"   // for Document
#include "util/Assert.h"      // for xoj_assert
#include "util/Util.h"        // for npos
#include "util/i18n.h"        // for _
#include "util/safe_casts.h"  // for strict_cast

#include "config.h"      // for PROJECT_STRING
#include "filesystem.h"  // for path

QPdfExport::QPdfExport(const Document* doc, ProgressListener* progressListener):
        HybridPdfExport(doc, progressListener) {}

QPdfExport::~QPdfExport() = default;

static void reorderBackgrounds(QPDF& background, const std::vector<HybridPdfExport::OutputPageInfo>& outputPageInfos) {
    auto pageHelper = QPDFPageDocumentHelper(background);
    // We may need to shuffle the pages around: Remember the original pages addresses.
    auto backgroundPages = pageHelper.getAllPages();
    auto count = backgroundPages.size();
    // Count occurrences of background pages: the user might have duplicated or removed pages
    auto occurrences = HybridPdfExport::countOccurrences(count, outputPageInfos);

    size_t nbValidPages = 0;
    std::optional<QPDFPageObjectHelper> lastPageSet;
    for (size_t i = 0; i < outputPageInfos.size(); i++) {
        auto n = outputPageInfos[i].pdfBackgroundPageNumber;
        if (n != npos) {
#if QPDF_MAJOR_VERSION >= 11
            if (const auto& ps = background.getAllPages();
                !ps[nbValidPages].isSameObjectAs(backgroundPages[n].getObjectHandle())) {
#else
            {
                const auto& ps = background.getAllPages();
#endif
                if (nbValidPages > 0) {
                    auto prevPage = ps[nbValidPages - 1];
                    pageHelper.removePage(backgroundPages[n]);
                    pageHelper.addPageAt(backgroundPages[n], false, prevPage);
                } else {
                    pageHelper.removePage(backgroundPages[n]);
                    pageHelper.addPage(backgroundPages[n], true);
                }
            }
            if (occurrences[n].number >= 2) {
                // Keep a copy for next time
                backgroundPages[n].getObjectHandle().makeResourcesIndirect(background);
                backgroundPages[n] = backgroundPages[n].shallowCopyPage();
                pageHelper.addPage(backgroundPages[n], false);  // Append the copy
#if QPDF_MAJOR_VERSION >= 11
                xoj_assert(background.getAllPages().back().isSameObjectAs(backgroundPages[n].getObjectHandle()));
#endif
            }
            nbValidPages++;
            occurrences[n].number--;
        }
    }

    // The backgrounds are in place. Remove the unused backgrounds.
    auto pages = pageHelper.getAllPages();  // Makes a copy of the vector, so the iterators don't get invalidated
    for (auto it = std::next(pages.begin(), as_signed(nbValidPages)); it != pages.end(); it++) {
        pageHelper.removePage(*it);
    }
    xoj_assert(as_signed(background.getAllPages().size()) ==
               std::count_if(outputPageInfos.begin(), outputPageInfos.end(),
                             [](auto&& a) { return a.pdfBackgroundPageNumber != npos; }));
}

bool QPdfExport::overlayAndSave(const fs::path& saveDestination, std::stringstream& overlaystream,
                                const std::vector<OutputPageInfo>& outputPageInfos) {
    try {
        auto overlaydata = overlaystream.str();  // Should probably be kept alive until we are done (not sure)
        QPDF overlay;
        overlay.processMemoryFile("overlay", overlaydata.data(), overlaydata.length());

        QPDF background;
        background.processFile(doc->getPdfFilepath().u8string().c_str());  // TODO: UTF8 is ok?

        reorderBackgrounds(background, outputPageInfos);

        // Prepare xobjects representing the pages in the overlay document
        std::vector<QPDFObjectHandle> overlaysAsXObjects;
        auto overlayPages = QPDFPageDocumentHelper(overlay).getAllPages();
        overlaysAsXObjects.reserve(overlayPages.size());
        for (auto&& p: overlayPages) {
            overlaysAsXObjects.emplace_back(p.getFormXObjectForPage());
        }

        auto outputPages = QPDFPageDocumentHelper(background).getAllPages();
        for (size_t n = 0, overlayPagesConsumed = 0; n < outputPageInfos.size(); n++) {
            auto [hasOverlay, bgIndex] = outputPageInfos[n];
            if (hasOverlay) {
                if (bgIndex != npos) {
                    auto page = QPDFPageObjectHelper(background.getAllPages()[n]);
                    // See qpdf/examples/pdf-overlay-page.cc

                    // Find a unique resource name for the new form XObject
                    QPDFObjectHandle resources = page.getAttribute("/Resources", true);
                    int min_suffix = 1;
                    std::string name = resources.getUniqueResourceName("/Fx", min_suffix);

                    auto localXObj = background.copyForeignObject(overlaysAsXObjects[overlayPagesConsumed]);

                    // Generate content to place the form XObject centered within destination page's trim box.
                    QPDFMatrix m;
                    std::string content =
                            page.placeFormXObject(localXObj, name, page.getMediaBox().getArrayAsRectangle(), m);
                    if (!content.empty()) {
                        // Append the content to the page's content. Surround the original content with q...Q to
                        // the new content from the page's original content.
                        resources.mergeResources("<< /XObject << >> >>"_qpdf);
                        resources.getKey("/XObject").replaceKey(name, localXObj);
#if QPDF_MAJOR_VERSION > 11 || (QPDF_MAJOR_VERSION == 11 && QPDF_MINOR_VERSION >= 2)
                        page.addPageContents(background.newStream("q\n"), true);
                        page.addPageContents(background.newStream("\nQ\n" + content), false);
#else
                        page.addPageContents(QPDFObjectHandle::newStream(&background, "q\n"), true);
                        page.addPageContents(QPDFObjectHandle::newStream(&background, "\nQ\n" + content), false);
#endif
                    }
                } else {
                    // Simply insert the overlay as a page
                    if (n == 0) {
                        QPDFPageDocumentHelper(background).addPage(overlayPages[overlayPagesConsumed], true);
                    } else {
                        QPDFPageDocumentHelper(background)
                                .addPageAt(overlayPages[overlayPagesConsumed], false, background.getAllPages()[n - 1]);
                    }
                }
                overlayPagesConsumed++;
            }
        }

        auto info = [&]() {
            auto trailer = background.getTrailer();
            if (trailer.hasKey("/Info")) {
                return trailer.getKey("/Info");
            }
            auto info = QPDFObjectHandle::newDictionary();
            trailer.replaceKey("/Info", info);
            return info;
        }();

        auto replaceKey = [&](const char* key, const std::string& val) {
            QPDFObjectHandle str = info.newString(val);
            str.makeDirect();
            info.replaceKey(key, str);
        };

        replaceKey("/Producer", (std::string(PROJECT_STRING) + " + QPDF " + QPDF_VERSION));
        replaceKey("/ModDate", createPDFDateStringForNow());

        QPDFWriter writer(background, saveDestination.u8string().data());  // is UTF8 ok?
        writer.write();
    } catch (const std::exception& e) {
        this->lastError = _("Error with overlay or final export:");
        this->lastError += std::string("\n") + e.what();
        return false;
    }
    return true;
}
#endif
