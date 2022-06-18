#include "ImageElementView.h"

#include <gdk-pixbuf/gdk-pixbuf.h>  // for gdk_pixbuf_...
#include <gdk/gdk.h>                // for gdk_cairo_s...

#include "gui/Shadow.h"                                   // for Shadow
#include "gui/dialog/backgroundSelect/BaseElementView.h"  // for BaseElement...

class BackgroundSelectDialogBase;

ImageElementView::ImageElementView(int id, BackgroundSelectDialogBase* dlg): BaseElementView(id, dlg) {}

ImageElementView::~ImageElementView() = default;

void ImageElementView::calcSize() {
    if (this->width == -1) {
        GdkPixbuf* p = backgroundImage.getPixbuf();
        this->width = gdk_pixbuf_get_width(p);
        this->height = gdk_pixbuf_get_height(p);

        if (this->width < this->height) {
            zoom = 128.0 / this->height;
        } else {
            zoom = 128.0 / this->width;
        }
        this->width *= zoom;
        this->height *= zoom;
    }
}

void ImageElementView::paintContents(cairo_t* cr) {
    cairo_scale(cr, this->zoom, this->zoom);

    GdkPixbuf* p = this->backgroundImage.getPixbuf();
    gdk_cairo_set_source_pixbuf(cr, p, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);
    cairo_paint(cr);
}

auto ImageElementView::getContentWidth() -> int { return width; }

auto ImageElementView::getContentHeight() -> int { return height; }
