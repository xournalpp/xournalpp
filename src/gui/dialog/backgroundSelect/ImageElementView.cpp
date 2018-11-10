#include "ImageElementView.h"

ImageElementView::ImageElementView()
{
	XOJ_INIT_TYPE(ImageElementView);
}

ImageElementView::~ImageElementView()
{
	XOJ_CHECK_TYPE(ImageElementView);

	XOJ_RELEASE_TYPE(ImageElementView);
}

/**



class ImageView
{
public:
	ImageView(int id, ImagesDialog* dlg)
	{
		XOJ_INIT_TYPE(ImageView);

		this->selected = false;
		this->dlg = dlg;
		this->id = id;
		this->width = -1;
		this->height = -1;
		this->zoom = 1;

		gtk_widget_set_events(widget, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

		// TODO Draw!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		g_signal_connect(this->widget, "expose_event", G_CALLBACK(exposeEventCallback), this);
		g_signal_connect(this->widget, "button-press-event", G_CALLBACK(mouseButtonPressCallback), this);
	}


	void calcSize()
	{
		XOJ_CHECK_TYPE(ImageView);

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

	int getWidth()
	{
		XOJ_CHECK_TYPE(ImageView);

		calcSize();
		return width + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 4;
	}

	int getHeight()
	{
		XOJ_CHECK_TYPE(ImageView);

		calcSize();
		return height + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 4;
	}

	void setSelected(bool selected)
	{
		XOJ_CHECK_TYPE(ImageView);

		if (this->selected == selected)
		{
			return;
		}
		this->selected = selected;

		repaint();
	}

	void repaint()
	{
		XOJ_CHECK_TYPE(ImageView);

		if (this->crBuffer)
		{
			cairo_surface_destroy(this->crBuffer);
			this->crBuffer = NULL;
		}
		gtk_widget_queue_draw(this->widget);
	}


private:
	static gboolean exposeEventCallback(GtkWidget* widget, GdkEventExpose* event, ImageView* page)
	{
		XOJ_CHECK_TYPE_OBJ(page, ImageView);

		page->paint();
		return true;
	}

	static gboolean mouseButtonPressCallback(GtkWidget* widget, GdkEventButton* event, ImageView* page)
	{
		XOJ_CHECK_TYPE_OBJ(page, ImageView);

		page->dlg->setSelected(page->id);
		return true;
	}

public:
	BackgroundImage backgroundImage;

private:
	XOJ_TYPE_ATTRIB;

	ImagesDialog* dlg;
	int id;
	int width;
	int height;
	double zoom;
};


*/
