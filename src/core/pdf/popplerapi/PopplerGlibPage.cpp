#include "PopplerGlibPage.h"

#include <algorithm>  // for max, min
#include <cstdlib>    // for abs, NULL, ptrdiff_t
#include <memory>     // for make_unique
#include <sstream>    // for operator<<, ostringstream, bas...

#include <glib.h>          // for g_free, g_utf8_offset_to_pointer
#include <poppler-page.h>  // for _PopplerRectangle, _PopplerLin...
#include <poppler.h>       // for PopplerRectangle, g_object_ref

#include "pdf/base/XojPdfAction.h"     // for XojPdfAction
#include "pdf/base/XojPdfPage.h"       // for XojPdfRectangle, XojPdfPage::Link
#include "util/GListView.h"            // for GListView, GListView<>::GListV...
#include "util/raii/CLibrariesSPtr.h"  // for adopt
#include "util/raii/CairoWrappers.h"   // for CairoRegionSPtr

#include "PopplerGlibAction.h"  // for PopplerGlibAction
#include "cairo.h"              // for cairo_region_create, cairo_reg...

PopplerGlibPage::PopplerGlibPage(PopplerPage* page, PopplerDocument* parentDoc): page(page), document(parentDoc) {
    if (page != nullptr) {
        g_object_ref(page);
    }
}

PopplerGlibPage::PopplerGlibPage(const PopplerGlibPage& other): page(other.page), document(other.document) {
    if (page != nullptr) {
        g_object_ref(page);
    }

    document = nullptr;
}

PopplerGlibPage::~PopplerGlibPage() {
    if (page) {
        g_object_unref(page);
        page = nullptr;
    }
}

PopplerGlibPage& PopplerGlibPage::operator=(const PopplerGlibPage& other) {
    if (&other == this) {
        return *this;
    }
    if (page) {
        g_object_unref(page);
        page = nullptr;
    }

    page = other.page;
    if (page != nullptr) {
        g_object_ref(page);
    }

    document = other.document;

    return *this;
}

auto PopplerGlibPage::getWidth() const -> double {
    double width = 0;
    poppler_page_get_size(const_cast<PopplerPage*>(page), &width, nullptr);

    return width;
}

auto PopplerGlibPage::getHeight() const -> double {
    double height = 0;
    poppler_page_get_size(const_cast<PopplerPage*>(page), nullptr, &height);

    return height;
}

void PopplerGlibPage::render(cairo_t* cr) const {
    cairo_save(cr);
    cairo_set_source_rgb(cr, 1., 1., 1.);
    cairo_paint(cr);
    poppler_page_render(page, cr);
    cairo_restore(cr);
}

void PopplerGlibPage::renderForPrinting(cairo_t* cr) const { poppler_page_render_for_printing(page, cr); }

auto PopplerGlibPage::getPageId() const -> int { return poppler_page_get_index(page); }

auto PopplerGlibPage::findText(const std::string& text) -> std::vector<XojPdfRectangle> {
    std::vector<XojPdfRectangle> findings;

    double height = getHeight();
    GList* matches = poppler_page_find_text(page, text.c_str());
    for (auto& rect: GListView<PopplerRectangle>(matches)) {
        findings.emplace_back(rect.x1, height - rect.y1, rect.x2, height - rect.y2);
        poppler_rectangle_free(&rect);
    }
    g_list_free(matches);

    return findings;
}

auto getPopplerSelectionStyle(XojPdfPageSelectionStyle style) -> PopplerSelectionStyle {
    switch (style) {
        case XojPdfPageSelectionStyle::Word:
            return POPPLER_SELECTION_WORD;
        case XojPdfPageSelectionStyle::Line:
            return POPPLER_SELECTION_LINE;
        case XojPdfPageSelectionStyle::Linear:
        case XojPdfPageSelectionStyle::Area:
            return POPPLER_SELECTION_GLYPH;
        default:
            g_assert(false && "unimplemented");
    }
}

auto PopplerGlibPage::selectText(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) -> std::string {
    PopplerRectangle pRect = {rect.x1, rect.y1, rect.x2, rect.y2};
    const auto pStyle = getPopplerSelectionStyle(style);
    if (style == XojPdfPageSelectionStyle::Area) {
        PopplerRectangle* rectArray = nullptr;
        guint numRects = 0;
        if (!poppler_page_get_text_layout_for_area(this->page, &pRect, &rectArray, &numRects)) {
            return "";
        }
        char* textBytes = poppler_page_get_text_for_area(page, &pRect);
        g_assert_nonnull(textBytes);

        double y = rectArray[0].y2;
        std::ostringstream ss;
        for (guint i = 0; i < numRects; i++) {
            // do not copy characters whose bounding box has a non-empty intersection with rect
            const auto& r = rectArray[i];
            {
                auto x1 = std::max(rect.x1, r.x1);
                auto y1 = std::max(rect.y1, r.y1);
                auto x2 = std::min(rect.x2, r.x2);
                auto y2 = std::min(rect.y2, r.y2);

                bool inBounds = x2 > x1 && y2 > y1;
                if (!inBounds)
                    continue;
            }

            const auto eps = 1e-5;
            if (std::abs(y - r.y2) > eps) {
                // new line
                ss << '\n';
                y = rectArray[i].y2;
            }

            char* const startPos = g_utf8_offset_to_pointer(textBytes, i);
            char* const endPos = g_utf8_offset_to_pointer(textBytes, i + 1);
            for (long j = 0; j < static_cast<ptrdiff_t>(endPos - startPos); ++j) { ss << startPos[j]; }
        }
        g_free(textBytes);
        return ss.str();
    } else {
        char* text = poppler_page_get_selected_text(page, pStyle, &pRect);
        if (text) {
            std::string ret(text);
            g_free(text);
            return ret;
        } else {
            return "";
        }
    }
}

auto PopplerGlibPage::selectTextRegion(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) -> cairo_region_t* {
    PopplerRectangle pRect = {rect.x1, rect.y1, rect.x2, rect.y2};
    const auto pStyle = getPopplerSelectionStyle(style);
    // The computed region is technically wrong for
    // XojPdfPageSelectionStyle::Area, but there is no selection preview with
    // area select.
    cairo_region_t* region = poppler_page_get_selected_region(page, 1.0, pStyle, &pRect);
    return region;
}

namespace {
cairo_rectangle_int_t cairoRectFromDouble(double x1, double y1, double width, double height) {
    return {static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(width), static_cast<int>(height)};
}
}  // namespace

auto PopplerGlibPage::selectTextLines(const XojPdfRectangle& selectRect, XojPdfPageSelectionStyle style)
        -> TextSelection {
    std::vector<XojPdfRectangle> textRects;

    // The selection rectangle may be "improper" by having x2 <= x1 or y1 <= y2 (e.g., if user
    // selects from right to left or from bottom to top). This is incompatible with cairo, so
    // construct a proper rectangle satisfying x1 <= x2 and y1 <= y2.
    PopplerRectangle rect{std::min(selectRect.x1, selectRect.x2), std::min(selectRect.y1, selectRect.y2),
                          std::max(selectRect.x1, selectRect.x2), std::max(selectRect.y1, selectRect.y2)};

    PopplerRectangle* rectArray = nullptr;
    guint numRects = 0;
    if (style == XojPdfPageSelectionStyle::Area) {
        // We always want to select in the "proper" rectangle.
        PopplerRectangle area{rect.x1, rect.y1, rect.x2, rect.y2};
        if (!poppler_page_get_text_layout_for_area(this->page, &area, &rectArray, &numRects)) {
            return {xoj::util::CairoRegionSPtr(cairo_region_create(), xoj::util::adopt), textRects};
        }
    } else {
        if (!poppler_page_get_text_layout(this->page, &rectArray, &numRects)) {
            return {xoj::util::CairoRegionSPtr(cairo_region_create(), xoj::util::adopt), textRects};
        }
    }

    // construct the region later for area selection, but use poppler's region
    // for other selection styles
    cairo_region_t* region = nullptr;
    if (style != XojPdfPageSelectionStyle::Area) {
        // do not use the "proper" rectangle here as it may be different from the actual selection.
        region = selectTextRegion(selectRect, style);
    }

    const auto isSameLine = [&](const auto& r1, const auto& r2) {
        const auto eps = 1e-5;
        return std::abs(r1.y1 - r2.y1) < eps && std::abs(r1.y2 - r2.y2) < eps;
    };

    PopplerRectangle prevRect = rectArray[0];
    if (style == XojPdfPageSelectionStyle::Area) {
        // helper to add only those rectangles that have nonempty intersection with the selected area
        const auto addTextRectsInArea = [&](const PopplerRectangle& r) {
            auto x1 = std::max(rect.x1, r.x1);
            auto y1 = std::max(rect.y1, r.y1);
            auto x2 = std::min(rect.x2, r.x2);
            auto y2 = std::min(rect.y2, r.y2);

            bool inBounds = x2 > x1 && y2 > y1;
            if (inBounds) {
                textRects.emplace_back(r.x1, r.y1, r.x2, r.y2);
            }
        };

        // construct the text rectangles
        for (guint i = 1; i < numRects; i++) {
            PopplerRectangle nextRect = rectArray[i];
            if (isSameLine(prevRect, nextRect)) {
                // Merge if both prev & next rectangles are in bounds. Note that
                // only x is checked since rectArray was constructed for the
                // selected area.
                bool shouldMerge = (rect.x1 <= prevRect.x1 && prevRect.x2 <= rect.x2 && rect.x1 <= nextRect.x1 &&
                                    nextRect.x2 <= rect.x2);
                if (shouldMerge) {
                    prevRect.x1 = std::min(prevRect.x1, nextRect.x2);
                    prevRect.x2 = std::max(prevRect.x2, nextRect.x2);
                    continue;
                }
            }

            addTextRectsInArea(prevRect);
            prevRect = nextRect;
        }
        addTextRectsInArea(prevRect);

        region = cairo_region_create();
        for (const XojPdfRectangle& r: textRects) {
            const auto x1 = std::min(r.x1, r.x2);
            const auto x2 = std::max(r.x1, r.x2);
            const auto y1 = std::min(r.y1, r.y2);
            const auto y2 = std::max(r.y1, r.y2);
            cairo_rectangle_int_t crect = cairoRectFromDouble(x1, y1, x2 - x1, y2 - y1);
            cairo_region_union_rectangle(region, &crect);
        }
    } else {
        // this is for all other styles (e.g., linear)

        // helper to add only those rectangles that are contained in the selection region
        const auto addTextRectsInRegion = [&](const PopplerRectangle& r) {
            auto crect = cairoRectFromDouble(r.x1, r.y1, r.x2 - r.x1, r.y2 - r.y1);
            if (cairo_region_contains_rectangle(region, &crect) == CAIRO_REGION_OVERLAP_IN) {
                textRects.emplace_back(r.x1, r.y1, r.x2, r.y2);
            }
        };

        // construct the text rectangles
        for (guint i = 1; i < numRects; i++) {
            PopplerRectangle nextRect = rectArray[i];
            if (isSameLine(prevRect, nextRect)) {
                // merge the rectangles if they, when combined, are contained in the selection region
                auto x1 = std::min(prevRect.x1, nextRect.x2);
                auto x2 = std::max(prevRect.x2, nextRect.x2);
                auto crect = cairoRectFromDouble(x1, prevRect.y1, x2 - x1, prevRect.y2 - prevRect.y1);
                if (cairo_region_contains_rectangle(region, &crect) == CAIRO_REGION_OVERLAP_IN) {
                    prevRect.x1 = x1;
                    prevRect.x2 = x2;
                    continue;
                }
            }
            addTextRectsInRegion(prevRect);
            prevRect = nextRect;
        }
        addTextRectsInRegion(prevRect);
    }

    g_assert_nonnull(region);
    return {xoj::util::CairoRegionSPtr(region, xoj::util::adopt), textRects};
}

auto PopplerGlibPage::getLinks() -> std::vector<Link> {
    std::vector<Link> results;
    const double height = getHeight();

    GList* links = poppler_page_get_link_mapping(this->page);
    for (GList* l = links; l != NULL; l = g_list_next(l)) {
        const auto& link = *static_cast<PopplerLinkMapping*>(l->data);

        if (link.action) {
            XojPdfRectangle rect{link.area.x1, height - link.area.y2, link.area.x2, height - link.area.y1};
            results.emplace_back(Link{rect, std::make_unique<PopplerGlibAction>(link.action, document)});
        }
    }
    poppler_page_free_link_mapping(links);

    return results;
}
