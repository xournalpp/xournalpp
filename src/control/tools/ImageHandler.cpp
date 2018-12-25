#include "ImageHandler.h"

#include "control/Control.h"
#include "control/stockdlg/ImageOpenDlg.h"
#include "gui/PageView.h"
#include "model/Image.h"
#include "model/Layer.h"
#include "undo/InsertUndoAction.h"

#include <i18n.h>
#include <XojMsgBox.h>

ImageHandler::ImageHandler(Control* control, XojPageView* view)
{
	XOJ_INIT_TYPE(ImageHandler);

	this->control = control;
	this->view = view;
}

ImageHandler::~ImageHandler()
{
	XOJ_RELEASE_TYPE(ImageHandler);
}

bool ImageHandler::insertImage(double x, double y)
{
	XOJ_CHECK_TYPE(ImageHandler);

	GFile* file = ImageOpenDlg::show(control->getGtkWindow(), control->getSettings());
	if (file == NULL)
	{
		return false;
	}
	return insertImage(file, x, y);
}

bool ImageHandler::insertImage(GFile* file, double x, double y)
{
	XOJ_CHECK_TYPE(ImageHandler);

	GError* err = NULL;
	GFileInputStream* in = g_file_read(file, NULL, &err);

	g_object_unref(file);

	GdkPixbuf* pixbuf = NULL;

	if (!err)
	{
		pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), NULL, &err);
		g_input_stream_close(G_INPUT_STREAM(in), NULL, NULL);
	}
	else
	{
		XojMsgBox::showErrorToUser(control->getGtkWindow(), FS(_F("This image could not be loaded. Error message: {1}") % err->message));
		g_error_free(err);
		return false;
	}

	Image* img = new Image();
	img->setX(x);
	img->setY(y);
	img->setImage(pixbuf);

	int width = gdk_pixbuf_get_width(pixbuf);
	int height = gdk_pixbuf_get_height(pixbuf);
	g_object_unref(pixbuf);

	double zoom = 1;

	PageRef page = view->getPage();

	if (x + width > page->getWidth() || y + height > page->getHeight())
	{
		double maxZoomX = (page->getWidth() - x) / width;
		double maxZoomY = (page->getHeight() - y) / height;

		if (maxZoomX < maxZoomY)
		{
			zoom = maxZoomX;
		}
		else
		{
			zoom = maxZoomY;
		}
	}

	img->setWidth(width * zoom);
	img->setHeight(height * zoom);

	page->getSelectedLayer()->addElement(img);

	InsertUndoAction* insertUndo = new InsertUndoAction(page, page->getSelectedLayer(), img);
	control->getUndoRedoHandler()->addUndoAction(insertUndo);

	view->rerenderElement(img);

	return true;
}
