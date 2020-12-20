#include "PopplerGlibPage.h"


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

auto PopplerGlibPage::findText(string& text) -> vector<XojPdfRectangle> {
    vector<XojPdfRectangle> findings;

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
