#include "TextView.h"

#include <algorithm>  // for max
#include <cstddef>    // for size_t

#include <glib-object.h>  // for g_object_unref

#include "model/Text.h"           // for Text
#include "pdf/base/XojPdfPage.h"  // for XojPdfRectangle
#include "util/Color.h"           // for cairo_set_source_rgbi
#include "util/StringUtils.h"     // for StringUtils
#include "view/View.h"            // for Context, OPACITY_NO_AUDIO, view

#include "filesystem.h"  // for path

using namespace xoj::view;

TextView::TextView(const Text* text): text(text) {}

TextView::~TextView() = default;

auto TextView::initPango(cairo_t* cr, const Text* t) -> PangoLayout* {
    PangoLayout* layout = pango_cairo_create_layout(cr);

    // Additional Feature: add autowrap and text field size for
    // the next xournal release (with new fileformat...)
    // pango_layout_set_wrap

    pango_cairo_update_layout(cr, layout);

    pango_context_set_matrix(pango_layout_get_context(layout), nullptr);
    updatePangoFont(layout, t);
    return layout;
}

void TextView::updatePangoFont(PangoLayout* layout, const Text* t) {
    PangoFontDescription* desc = pango_font_description_from_string(t->getFontName().c_str());
    pango_font_description_set_absolute_size(desc, t->getFontSize() * PANGO_SCALE);

#if PANGO_VERSION_CHECK(1, 48, 5)  // see https://gitlab.gnome.org/GNOME/pango/-/issues/499
    pango_layout_set_line_spacing(layout, 1.0);
#endif

    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
}

void TextView::draw(const Context& ctx) const {
    if (text->isInEditing()) {
        // The drawing is handled by gui/TextEditor
        return;
    }

    cairo_save(ctx.cr);

    // make elements without audio translucent when highlighting elements with audio
    if (ctx.fadeOutNonAudio && text->getAudioFilename().empty()) {
        cairo_set_operator(ctx.cr, CAIRO_OPERATOR_OVER);
        Util::cairo_set_source_rgbi(ctx.cr, text->getColor(), OPACITY_NO_AUDIO);
    } else {
        cairo_set_operator(ctx.cr, CAIRO_OPERATOR_SOURCE);
        Util::cairo_set_source_rgbi(ctx.cr, text->getColor());
    }

    cairo_translate(ctx.cr, text->getX(), text->getY());

    PangoLayout* layout = initPango(ctx.cr, text);
    std::string content = text->getText();
    pango_layout_set_text(layout, content.c_str(), static_cast<int>(content.length()));

    pango_cairo_show_layout(ctx.cr, layout);

    g_object_unref(layout);

    cairo_restore(ctx.cr);
}

auto TextView::findText(const Text* t, const std::string& search) -> std::vector<XojPdfRectangle> {
    size_t patternLength = search.length();
    if (patternLength == 0) {
        return {};
    }

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t* cr = cairo_create(surface);

    PangoLayout* layout = initPango(cr, t);
    std::string content = t->getText();
    pango_layout_set_text(layout, content.c_str(), static_cast<int>(content.length()));


    std::string text = StringUtils::toLowerCase(content);

    std::string pattern = StringUtils::toLowerCase(search);

    std::vector<XojPdfRectangle> list;

    for (size_t pos = text.find(pattern); pos != std::string::npos; pos = text.find(pattern, pos + 1)) {
        XojPdfRectangle mark;
        PangoRectangle rect = {0};
        pango_layout_index_to_pos(layout, static_cast<int>(pos), &rect);
        mark.x1 = (static_cast<double>(rect.x)) / PANGO_SCALE + t->getX();
        mark.y1 = (static_cast<double>(rect.y)) / PANGO_SCALE + t->getY();

        pango_layout_index_to_pos(layout, static_cast<int>(pos + patternLength - 1), &rect);
        mark.x2 = (static_cast<double>(rect.x) + rect.width) / PANGO_SCALE + t->getX();
        mark.y2 = (static_cast<double>(rect.y) + rect.height) / PANGO_SCALE + t->getY();

        list.push_back(mark);
    }

    g_object_unref(layout);
    cairo_surface_destroy(surface);
    cairo_destroy(cr);

    return list;
}

void TextView::calcSize(const Text* t, double& width, double& height) {
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t* cr = cairo_create(surface);

    PangoLayout* layout = initPango(cr, t);
    std::string content = t->getText();
    pango_layout_set_text(layout, content.c_str(), static_cast<int>(content.length()));
    int w = 0;
    int h = 0;
    pango_layout_get_size(layout, &w, &h);
    width = (static_cast<double>(w)) / PANGO_SCALE;
    height = (static_cast<double>(h)) / PANGO_SCALE;
    g_object_unref(layout);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
