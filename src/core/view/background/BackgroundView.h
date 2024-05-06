/*
 * Xournal++
 *
 * Displays a background
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr

#include <cairo.h>  // for cairo_t

#include "model/PageRef.h"  // for PageRef
#include "util/Color.h"     // for Color

class PdfCache;
class PageType;

namespace xoj {
namespace view {

enum PDFBackgroundTreatment : bool { SHOW_PDF_BACKGROUND = true, HIDE_PDF_BACKGROUND = false };
enum ImageBackgroundTreatment : bool { SHOW_IMAGE_BACKGROUND = true, HIDE_IMAGE_BACKGROUND = false };
enum RulingBackgroundTreatment : bool { SHOW_RULING_BACKGROUND = true, HIDE_RULING_BACKGROUND = false };
enum VisibilityTreatment : bool { FORCE_VISIBLE = true, USE_DOCUMENT_VISIBILITY = false };

struct BackgroundFlags {
    PDFBackgroundTreatment showPDF;
    ImageBackgroundTreatment showImage;
    RulingBackgroundTreatment showRuling;
    VisibilityTreatment forceVisible = USE_DOCUMENT_VISIBILITY;
};

static constexpr BackgroundFlags BACKGROUND_SHOW_ALL = {SHOW_PDF_BACKGROUND, SHOW_IMAGE_BACKGROUND,
                                                        SHOW_RULING_BACKGROUND};

static constexpr BackgroundFlags BACKGROUND_SHOW_ONLY_BACKGROUND_COLOR = {HIDE_PDF_BACKGROUND, HIDE_IMAGE_BACKGROUND,
                                                                          HIDE_RULING_BACKGROUND, FORCE_VISIBLE};

class BackgroundView {
public:
    BackgroundView(double pageWidth, double pageHeight): pageWidth(pageWidth), pageHeight(pageHeight) {}
    virtual ~BackgroundView() = default;

    /**
     * @brief Draws the background on the entire mask represented by the cairo context cr
     *
     * Does nothing in the base class - used when the background drawing is suppressed.
     */
    virtual void draw(cairo_t* cr) const {}

    [[nodiscard]] static std::unique_ptr<BackgroundView> createRuled(double width, double height, Color backgroundColor,
                                                                     const PageType& pt, double lineWidthFactor = 1.0);

    [[nodiscard]] static std::unique_ptr<BackgroundView> createForPage(PageRef page, xoj::view::BackgroundFlags bgFlags,
                                                                       PdfCache* pdfCache = nullptr);

protected:
    double pageWidth;
    double pageHeight;
};
};  // namespace view
};  // namespace xoj
