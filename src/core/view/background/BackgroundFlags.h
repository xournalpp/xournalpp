/*
 * Xournal++
 *
 * Flags for background painting
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

namespace xoj::view {

enum PDFBackgroundTreatment : bool { SHOW_PDF_BACKGROUND = true, HIDE_PDF_BACKGROUND = false };
enum ImageBackgroundTreatment : bool { SHOW_IMAGE_BACKGROUND = true, HIDE_IMAGE_BACKGROUND = false };
enum RulingBackgroundTreatment : bool { SHOW_RULING_BACKGROUND = true, HIDE_RULING_BACKGROUND = false };
enum BackgroundColorTreatment : bool { FORCE_AT_LEAST_BACKGROUND_COLOR = true, DONT_FORCE_BACKGROUND_COLOR = false };
enum VisibilityTreatment : bool { FORCE_VISIBLE = true, USE_DOCUMENT_VISIBILITY = false };

struct BackgroundFlags {
    PDFBackgroundTreatment showPDF;
    ImageBackgroundTreatment showImage;
    RulingBackgroundTreatment showRuling;
    BackgroundColorTreatment forceBackgroundColor = DONT_FORCE_BACKGROUND_COLOR;
    VisibilityTreatment forceVisible = USE_DOCUMENT_VISIBILITY;
};

static constexpr BackgroundFlags BACKGROUND_SHOW_ALL = {SHOW_PDF_BACKGROUND, SHOW_IMAGE_BACKGROUND,
                                                        SHOW_RULING_BACKGROUND, DONT_FORCE_BACKGROUND_COLOR,
                                                        USE_DOCUMENT_VISIBILITY};

static constexpr BackgroundFlags BACKGROUND_FORCE_PAINT_BACKGROUND_COLOR_ONLY = {
        HIDE_PDF_BACKGROUND, HIDE_IMAGE_BACKGROUND, HIDE_RULING_BACKGROUND, FORCE_AT_LEAST_BACKGROUND_COLOR,
        FORCE_VISIBLE};
}  // namespace xoj::view
