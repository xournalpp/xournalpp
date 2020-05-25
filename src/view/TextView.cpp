#include "TextView.h"

#include "control/settings/Settings.h"
#include "model/Text.h"
#include "pdf/base/XojPdfPage.h"

#include "StringUtils.h"
#include "Util.h"

TextView::TextView() = default;

TextView::~TextView() = default;

static int textDpi = 72;

void TextView::setDpi(int dpi) { textDpi = dpi; }

auto TextView::initPango(cairo_t* cr, const Text* t) -> PangoLayout* {
    PangoLayout* layout = pango_cairo_create_layout(cr);

    // Additional Feature: add autowrap and text field size for
    // the next xournal release (with new fileformat...)
    // pango_layout_set_wrap

    pango_cairo_context_set_resolution(pango_layout_get_context(layout), textDpi);
    pango_cairo_update_layout(cr, layout);

    pango_context_set_matrix(pango_layout_get_context(layout), nullptr);
    updatePangoFont(layout, t);
    return layout;
}

void TextView::updatePangoFont(PangoLayout* layout, const Text* t) {
    PangoFontDescription* desc = pango_font_description_from_string(t->getFontName().c_str());
    // pango_font_description_set_absolute_size(desc, t->getFontSize() * PANGO_SCALE);
    pango_font_description_set_size(desc, t->getFontSize() * PANGO_SCALE);

    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
}

void TextView::drawText(cairo_t* cr, const Text* t) {
    cairo_save(cr);

    cairo_translate(cr, t->getX(), t->getY());

    PangoLayout* layout = initPango(cr, t);
    string str = t->getText();
    pango_layout_set_text(layout, str.c_str(), str.length());

    pango_cairo_show_layout(cr, layout);

    g_object_unref(layout);

    cairo_restore(cr);
}

auto TextView::findText(const Text* t, string& search) -> vector<XojPdfRectangle> {
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t* cr = cairo_create(surface);

    PangoLayout* layout = initPango(cr, t);
    string str = t->getText();
    pango_layout_set_text(layout, str.c_str(), str.length());


    string text = t->getText();

    string srch = StringUtils::toLowerCase(search);

    vector<XojPdfRectangle> list;

    int pos = -1;
    do {
        pos = StringUtils::toLowerCase(text).find(srch, pos + 1);
        if (pos != -1) {
            XojPdfRectangle mark;
            PangoRectangle rect = {0};
            pango_layout_index_to_pos(layout, pos, &rect);
            mark.x1 = (static_cast<double>(rect.x)) / PANGO_SCALE + t->getX();
            mark.y1 = (static_cast<double>(rect.y)) / PANGO_SCALE + t->getY();

            pango_layout_index_to_pos(layout, pos + srch.length(), &rect);
            mark.x2 = (static_cast<double>(rect.x) + rect.width) / PANGO_SCALE + t->getX();
            mark.y2 = (static_cast<double>(rect.y) + rect.height) / PANGO_SCALE + t->getY();

            list.push_back(mark);
        }
    } while (pos != -1);

    g_object_unref(layout);
    cairo_surface_destroy(surface);
    cairo_destroy(cr);

    return list;
}

void TextView::calcSize(const Text* t, double& width, double& height) {
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t* cr = cairo_create(surface);

    PangoLayout* layout = initPango(cr, t);
    string str = t->getText();
    pango_layout_set_text(layout, str.c_str(), str.length());
    int w = 0;
    int h = 0;
    pango_layout_get_size(layout, &w, &h);
    width = (static_cast<double>(w)) / PANGO_SCALE;
    height = (static_cast<double>(h)) / PANGO_SCALE;
    g_object_unref(layout);

    cairo_surface_destroy(surface);
    cairo_destroy(cr);
}
