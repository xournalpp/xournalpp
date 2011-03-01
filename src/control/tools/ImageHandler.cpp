#include "ImageHandler.h"
#include "../Control.h"
#include "../../gui/PageView.h"
#include "../../model/Image.h"
#include "../../util/pixbuf-utils.h"
#include "../../undo/InsertUndoAction.h"

ImageHandler::ImageHandler(Control * control, PageView * view) {
	this->control = control;
	this->view = view;
}

ImageHandler::~ImageHandler() {
}

bool ImageHandler::insertImage(double x, double y) {
	GtkWidget * dialog = gtk_file_chooser_dialog_new(_("Open Image"), (GtkWindow*) *this->control->getWindow(), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);

	// here we can handle remote files without problems with backward compatibility
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), false);

	GtkFileFilter * filterSupported = gtk_file_filter_new();
	gtk_file_filter_set_name(filterSupported, _("Images"));
	gtk_file_filter_add_pixbuf_formats(filterSupported);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);

	String lastImagePath = control->getSettings()->getLastImagePath();
	if (!lastImagePath.isEmpty()) {
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), lastImagePath.c_str());
	}

	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
		gtk_widget_destroy(dialog);
		return false;
	}
	GFile * file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));

	char * folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
	control->getSettings()->setLastImagePath(folder);
	g_free(folder);

	gtk_widget_destroy(dialog);

	return insertImage(file, x, y);
}

bool ImageHandler::insertImage(GFile * file, double x, double y) {
	GError * err = NULL;
	GFileInputStream * in = g_file_read(file, NULL, &err);
	GdkPixbuf * pixbuf = NULL;

	if (!err) {
		pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), NULL, &err);
		g_input_stream_close(G_INPUT_STREAM(in), NULL, NULL);
	} else {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *control->getWindow(), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				_("This image could not be loaded. Error message: %s"), err->message);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		g_error_free(err);
		return false;
	}

	Image * img = new Image();
	img->setX(x);
	img->setY(y);
	img->setImage(f_pixbuf_to_cairo_surface(pixbuf));

	int width = gdk_pixbuf_get_width(pixbuf);
	int height = gdk_pixbuf_get_height(pixbuf);
	gdk_pixbuf_unref(pixbuf);

	double zoom = 1;

	XojPage * page = view->getPage();

	if (x + width > page->getWidth() || y + height > page->getHeight()) {
		double maxZoomX = (page->getWidth() - x) / width;
		double maxZoomY = (page->getHeight() - y) / height;

		if (maxZoomX < maxZoomY) {
			zoom = maxZoomX;
		} else {
			zoom = maxZoomY;
		}
	}

	img->setWidth(width * zoom);
	img->setHeight(height * zoom);

	page->getSelectedLayer()->addElement(img);

	InsertUndoAction * insertUndo = new InsertUndoAction(page, page->getSelectedLayer(), img, view);
	control->getUndoRedoHandler()->addUndoAction(insertUndo);

	view->repaint();

	return true;
}
