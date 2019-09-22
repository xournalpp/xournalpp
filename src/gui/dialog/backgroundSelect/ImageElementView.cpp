#include "ImageElementView.h"

#include "gui/Shadow.h"

ImageElementView::ImageElementView(int id, BackgroundSelectDialogBase* dlg)
 : BaseElementView(id, dlg)
{
}

ImageElementView::~ImageElementView()
{
}

void ImageElementView::calcSize()
{
	if (this->width == -1)
	{
		GdkPixbuf* p = backgroundImage.getPixbuf();
		this->width = gdk_pixbuf_get_width(p);
		this->height = gdk_pixbuf_get_height(p);

		if (this->width < this->height)
		{
			zoom = 128.0 / this->height;
		}
		else
		{
			zoom = 128.0 / this->width;
		}
		this->width *= zoom;
		this->height *= zoom;
	}
}

void ImageElementView::paintContents(cairo_t* cr)
{
	cairo_scale(cr, this->zoom, this->zoom);

	GdkPixbuf* p = this->backgroundImage.getPixbuf();
	gdk_cairo_set_source_pixbuf(cr, p, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);
	cairo_paint(cr);
}

int ImageElementView::getContentWidth()
{
	return width;
}

int ImageElementView::getContentHeight()
{
	return height;
}
