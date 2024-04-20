#include "ImageElementView.h"

#include <gdk-pixbuf/gdk-pixbuf.h>  // for gdk_pixbuf_...
#include <gdk/gdk.h>                // for gdk_cairo_s...

#include "gui/Shadow.h"                                   // for Shadow
#include "gui/dialog/backgroundSelect/BaseElementView.h"  // for BaseElement...
#include "util/safe_casts.h"                              // for ceil_cast

class BackgroundSelectDialogBase;

constexpr int MINIATURE_PIXEL_SIZE = 128;

ImageElementView::ImageElementView(size_t id, BackgroundSelectDialogBase* dlg): BaseElementView(id, dlg) {}

ImageElementView::~ImageElementView() = default;

void ImageElementView::calcSize() {
    if (this->width == -1) {
        GdkPixbuf* p = backgroundImage.getPixbuf();
        this->width = gdk_pixbuf_get_width(p);
        this->height = gdk_pixbuf_get_height(p);

        if (this->width < this->height) {
            zoom = static_cast<double>(MINIATURE_PIXEL_SIZE) / this->height;
            this->height = MINIATURE_PIXEL_SIZE;
            this->width = floor_cast<int>(this->width * zoom);
        } else {
            zoom = static_cast<double>(MINIATURE_PIXEL_SIZE) / this->width;
            this->width = MINIATURE_PIXEL_SIZE;
            this->height = floor_cast<int>(this->height * zoom);
        }
    }
}

void ImageElementView::paintContents(cairo_t* cr) {
    cairo_scale(cr, this->zoom, this->zoom);

    GdkPixbuf* p = this->backgroundImage.getPixbuf();
    gdk_cairo_set_source_pixbuf(cr, p, 0, 0);
    cairo_paint(cr);
}

auto ImageElementView::getContentWidth() -> int { return width; }

auto ImageElementView::getContentHeight() -> int { return height; }
