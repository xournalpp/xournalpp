#include "PopplerGlibPage.h"

#include <sstream>

#include <poppler-page.h>
#include <poppler.h>

#include "cairo.h"


PopplerGlibPage::PopplerGlibPage(PopplerPage* page): page(page) {
    if (page != nullptr) {
        g_object_ref(page);
    }
}

PopplerGlibPage::PopplerGlibPage(const PopplerGlibPage& other): page(other.page) {
    if (page != nullptr) {
        g_object_ref(page);
    }
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
    return *this;
}

auto PopplerGlibPage::getWidth() -> double {
    double width = 0;
    poppler_page_get_size(page, &width, nullptr);

    return width;
}

auto PopplerGlibPage::getHeight() -> double {
    double height = 0;
    poppler_page_get_size(page, nullptr, &height);

    return height;
}

void PopplerGlibPage::render(cairo_t* cr, bool forPrinting)  // NOLINT(google-default-arguments)
{
    if (forPrinting) {
        poppler_page_render_for_printing(page, cr);
    } else {
        poppler_page_render(page, cr);
    }
}

auto PopplerGlibPage::getPageId() -> int { return poppler_page_get_index(page); }

auto PopplerGlibPage::findText(std::string& text) -> std::vector<XojPdfRectangle> {
    std::vector<XojPdfRectangle> findings;

    double height = getHeight();
    GList* matches = poppler_page_find_text(page, text.c_str());

    for (GList* l = matches; l && l->data; l = g_list_next(l)) {
        auto* rect = static_cast<PopplerRectangle*>(l->data);

        findings.emplace_back(rect->x1, height - rect->y1, rect->x2, height - rect->y2);

        poppler_rectangle_free(rect);
    }
    g_list_free(matches);

    return findings;
}

auto getPopplerSelectionStyle(XojPdfPageSelectionStyle style) -> PopplerSelectionStyle {
    switch (style) {
        case XojPdfPageSelectionStyle::XOJ_PDF_SELECTION_WORD:
            return POPPLER_SELECTION_WORD;
        case XojPdfPageSelectionStyle::XOJ_PDF_SELECTION_LINE:
            return POPPLER_SELECTION_LINE;
        default:
            return POPPLER_SELECTION_GLYPH;
    }
}

auto PopplerGlibPage::selectHeadTailText(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) -> std::string {
    PopplerRectangle pRect = {rect.x1, rect.y1, rect.x2, rect.y2};

    auto pStyle = getPopplerSelectionStyle(style);

    return poppler_page_get_selected_text(page, pStyle, &pRect);
}

auto PopplerGlibPage::selectHeadTailTextRegion(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style)
        -> cairo_region_t* {
    PopplerRectangle pRect = {rect.x1, rect.y1, rect.x2, rect.y2};

    auto pStyle = getPopplerSelectionStyle(style);

    return poppler_page_get_selected_region(page, 1.0, pStyle, &pRect);
}

void PopplerGlibPage::selectHeadTailFinally(const XojPdfRectangle& rect, cairo_region_t** region,
                                            std::vector<XojPdfRectangle>* rects, std::string* text,
                                            XojPdfPageSelectionStyle style) {
    rects->clear();
    text->clear();
    cairo_region_destroy(*region);

    *region = this->selectHeadTailTextRegion(rect, style);
    if (cairo_region_is_empty(*region))
        return;

    *text = this->selectHeadTailText(rect, style);
    PopplerRectangle area = {
            .x1 = rect.x1,
            .y1 = rect.y1,
            .x2 = rect.x2,
            .y2 = rect.y2,
    };

    // check if it was only clicked
    if (std::abs(rect.y1 - rect.y2) + std::abs(rect.x1 - rect.x2) < 0.001) {
        // if it was just a single click, do nothing
        if (style == XOJ_PDF_SELECTION_GLYPH) {
            rects->clear();
            text->clear();
            cairo_region_destroy(*region);
        } else {
            int count = cairo_region_num_rectangles(*region);

            for (int i = 0; i < count; ++i) {
                cairo_rectangle_int_t r;
                cairo_region_get_rectangle(*region, i, &r);
                rects->emplace_back(r.x, r.y, r.x + r.width, r.y + r.height);
            }
        }
        return;
    }

    // [2021-08-18] this part is come from:
    // https://gitlab.freedesktop.org/poppler/poppler/-/blob/master/glib/demo/annots.c
    PopplerRectangle* pRects;
    guint rectNums;
    if (!poppler_page_get_text_layout_for_area(this->page, &area, &pRects, &rectNums))
        return;
    auto r = PopplerRectangle{G_MAXDOUBLE, G_MAXDOUBLE, G_MINDOUBLE, G_MINDOUBLE};

    for (int i = 0; i < rectNums; i++) {
        /* Check if the rectangle belongs to the same line.
           On a new line, start a new target rectangle.
           On the same line, make an union of rectangles at
           the same line */
        if (std::abs(r.y2 - pRects[i].y2) > 0.0001) {
            if (i > 0)
                rects->emplace_back(r.x1, r.y1, r.x2, r.y2);
            r.x1 = pRects[i].x1;
            r.y1 = pRects[i].y1;
            r.x2 = pRects[i].x2;
            r.y2 = pRects[i].y2;
        } else {
            r.x1 = std::min(r.x1, pRects[i].x1);
            r.y1 = std::min(r.y1, pRects[i].y1);
            r.x2 = std::max(r.x2, pRects[i].x2);
            r.y2 = std::max(r.y2, pRects[i].y2);
        }
    }
    rects->emplace_back(r.x1, r.y1, r.x2, r.y2);
    g_free(pRects);

    amendHeadAndTail(rects, *region, style);
}

void PopplerGlibPage::amendHeadAndTail(std::vector<XojPdfRectangle>* rects, const cairo_region_t* region,
                                       XojPdfPageSelectionStyle style) {
    if (style == XojPdfPageSelectionStyle::XOJ_PDF_SELECTION_GLYPH || rects->empty())
        return;

    auto cairoRegion1 = cairo_region_copy(region);
    auto rectFirst = &(rects->at(0));
    cairo_rectangle_int_t rectCairo1 = {0, static_cast<int>(rectFirst->y1), static_cast<int>(rectFirst->x2),
                                        static_cast<int>(rectFirst->y2 - rectFirst->y1)};

    cairo_region_intersect_rectangle(cairoRegion1, &rectCairo1);
    if (cairo_region_num_rectangles(cairoRegion1) > 0) {
        cairo_rectangle_int_t r;
        cairo_region_get_rectangle(cairoRegion1, 0, &r);
        if (rectFirst->x1 > r.x)
            rectFirst->x1 = r.x;
    }
    cairo_region_destroy(cairoRegion1);

    auto cairoRegion2 = cairo_region_copy(region);
    auto rectLast = &(rects->at(rects->size() - 1));
    cairo_rectangle_int_t rectCairo2 = {static_cast<int>(rectLast->x1), static_cast<int>(rectLast->y1),
                                        INT_MAX - static_cast<int>(rectLast->x1) - 1,
                                        static_cast<int>(rectLast->y2 - rectLast->y1)};
    cairo_region_intersect_rectangle(cairoRegion2, &rectCairo2);
    int count2 = cairo_region_num_rectangles(cairoRegion2);
    if (count2 > 0) {
        cairo_rectangle_int_t r;
        cairo_region_get_rectangle(cairoRegion2, count2 - 1, &r);

        if (rectLast->x2 < r.x + r.width)
            rectLast->x2 = r.x + r.width;
    }
    cairo_region_destroy(cairoRegion2);
}

void PopplerGlibPage::selectAreaFinally(const XojPdfRectangle& rect, cairo_region_t** region,
                                        std::vector<XojPdfRectangle>* rects, std::string* text) {
    rects->clear();
    text->clear();

    auto chars = poppler_page_get_text(this->page);
    if (!g_utf8_validate(chars, -1, nullptr)) {
        // Only support utf-8 string now, else fallback to use selectHeadTailFinally()
        g_warning("Not a UTF-8 String");
        g_free(chars);

        selectHeadTailFinally(rect, region, rects, text);

        return;
    }

    PopplerRectangle* pRects;
    guint rectCount;
    if (!poppler_page_get_text_layout(this->page, &pRects, &rectCount)) {
        g_free(chars);
        return;
    }

    auto x1Box = std::min(rect.x1, rect.x2);
    auto x2Box = std::max(rect.x1, rect.x2);
    auto y1Box = std::min(rect.y1, rect.y2);
    auto y2Box = std::max(rect.y1, rect.y2);

    auto r = PopplerRectangle{G_MAXDOUBLE, G_MAXDOUBLE, G_MINDOUBLE, G_MINDOUBLE};

    int startIndex = -1;
    for (int i = 0; i < rectCount; i++) {
        if (pRects[i].x1 > x2Box || pRects[i].y1 > y2Box || pRects[i].x2 < x1Box || pRects[i].y2 < y1Box)
            continue;
        startIndex = i;

        r.x1 = pRects[i].x1;
        r.y1 = pRects[i].y1;
        r.x2 = pRects[i].x2;
        r.y2 = pRects[i].y2;

        break;
    }

    if (startIndex == -1 || startIndex >= rectCount)
        return;

    std::string returnString;
    std::string line;
    for (int i = startIndex; i < rectCount; i++) {
        if (pRects[i].x1 > x2Box || pRects[i].y1 > y2Box || pRects[i].x2 < x1Box || pRects[i].y2 < y1Box)
            continue;

        /* Check if the rectangle belongs to the same line.
           On a new line, start a new target rectangle.
           On the same line, make an union of rectangles at
           the same line */
        if (pRects[i].y2 - r.y2 > (pRects[i].y2 - pRects[i].y1) / 2) {
            rects->emplace_back(r.x1, r.y1, r.x2, r.y2);
            r.x1 = pRects[i].x1;
            r.y1 = pRects[i].y1;
            r.x2 = pRects[i].x2;
            r.y2 = pRects[i].y2;
            if (line.size() > 0 && line.compare(line.size() - 1, 1, "\n") != 0)
                line.append("\n");
            returnString.append(line);
            line.clear();
        } else {
            r.x1 = std::min(r.x1, pRects[i].x1);
            r.y1 = std::min(r.y1, pRects[i].y1);
            r.x2 = std::max(r.x2, pRects[i].x2);
            r.y2 = std::max(r.y2, pRects[i].y2);
        }

        auto start = g_utf8_offset_to_pointer(chars, i) - chars;
        auto end = g_utf8_offset_to_pointer(chars, i + 1) - chars;

        line.append(chars, start, end - start);
    }
    rects->emplace_back(r.x1, r.y1, r.x2, r.y2);
    returnString.append(line);
    line.clear();

    g_free(pRects);
    g_free(chars);

    auto tmpRegion = cairo_region_create();
    cairo_rectangle_int_t cRect;
    for (const auto& item: *rects) {
        cRect.x = static_cast<int>(item.x1);
        cRect.y = static_cast<int>(item.y1);
        cRect.width = static_cast<int>(item.x2 - item.x1);
        cRect.height = static_cast<int>(item.y2 - item.y1);
        cairo_region_union_rectangle(tmpRegion, &cRect);
    }

    *region = tmpRegion;
    *text = returnString;
}
