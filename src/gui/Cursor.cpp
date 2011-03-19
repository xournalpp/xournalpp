#include "Cursor.h"
#include "XournalView.h"
#include "../control/Control.h"
#include "../util/Util.h"
#include "../util/pixbuf-utils.h"

/************** drawing nice cursors *********/

static char CURSOR_HIGLIGHTER_BITS[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
		0x08, 0x08, 0x08, 0xf8, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static char CURSOR_HILIGHTER_MASK[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f, 0xf8, 0x0f, 0xf8, 0x0f, 0xf8, 0x0f, 0xf8, 0x0f, 0xf8, 0x0f, 0xf8, 0x0f,
		0xf8, 0x0f, 0xf8, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static char ERASER_CURSOR_BITS[] = { 0x00, 0x00, 0x00, 0xc0, 0x3a, 0x00, 0x70, 0xe0, 0x00, 0x18, 0x80, 0x01, 0x0c, 0x00, 0x03, 0x04, 0x00, 0x06, 0x06, 0x00,
		0x06, 0x02, 0x00, 0x0c, 0x02, 0x00, 0x0c, 0x00, 0x00, 0x08, 0x02, 0x00, 0x0c, 0x02, 0x00, 0x0c, 0x02, 0x00, 0x0c, 0x06, 0x00, 0x06, 0x04, 0x00, 0x06,
		0x0c, 0x00, 0x03, 0x18, 0x80, 0x01, 0x30, 0xe0, 0x00, 0xe0, 0x75, 0x00, 0x80, 0x1f, 0x00 };

static char ERASER_CURSOR_MASK[] = { 0x00, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0xf0, 0xff, 0x00, 0xf8, 0xff, 0x01, 0xfc, 0xff, 0x03, 0xfc, 0xff, 0x07, 0xfe, 0xff,
		0x07, 0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x07, 0xfc, 0xff, 0x07,
		0xfc, 0xff, 0x03, 0xf8, 0xff, 0x01, 0xf0, 0xff, 0x00, 0xe0, 0x7f, 0x00, 0x80, 0x1f, 0x00 };

Cursor::Cursor(Control * control) {
	this->control = control;
	this->busy = false;
	this->invisible = false;
	this->selectionType = CURSOR_SELECTION_NONE;
	this->insidePage = false;
}

Cursor::~Cursor() {
}

void Cursor::setMouseDown(bool mouseDown) {
	g_return_if_fail(this != NULL);

	if (this->mouseDown == mouseDown) {
		return;
	}

	this->mouseDown = mouseDown;
	ToolHandler * handler = control->getToolHandler();
	ToolType type = handler->getToolType();

	// Not always an update is needed
	if (type == TOOL_HAND || type == TOOL_VERTICAL_SPACE) {
		updateCursor();
	}
}

void Cursor::setMouseSelectionType(CursorSelectionType selectionType) {
	g_return_if_fail(this != NULL);

	if (this->selectionType == selectionType) {
		return;
	}
	this->selectionType = selectionType;
	updateCursor();
}

void Cursor::setCursorBusy(bool busy) {
	g_return_if_fail(this != NULL);

	MainWindow * win = control->getWindow();
	if (!win) {
		return;
	}

	if (this->busy == busy) {
		return;
	}

	this->busy = busy;

	if (busy) {
		GdkCursor * cursor = gdk_cursor_new(GDK_WATCH);
		if (gtk_widget_get_window(win->getWindow())) {
			gdk_window_set_cursor(gtk_widget_get_window(win->getWindow()), cursor);
		}

		gdk_cursor_unref(cursor);
	} else {
		if (gtk_widget_get_window(win->getWindow())) {
			gdk_window_set_cursor(gtk_widget_get_window(win->getWindow()), NULL);
		}
	}

	updateCursor();
}

void Cursor::setInsidePage(bool insidePage) {
	g_return_if_fail(this != NULL);

	if(this->insidePage == insidePage) {
		return;
	}

	this->insidePage = insidePage;

	updateCursor();
}

void Cursor::setInvisible(bool invisible) {
	g_return_if_fail(this != NULL);

	if (this->invisible == invisible) {
		return;
	}

	this->invisible = invisible;

	updateCursor();
}

GdkCursor * Cursor::getPenCursor() {
	ToolHandler * handler = control->getToolHandler();

	bool big = control->getSettings()->isShowBigCursor();

	int heigth = 3;
	int width = 3;
	if (big) {
		heigth = 22;
		width = 15;
	}

	cairo_surface_t * crCursor = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, heigth);
	cairo_t * cr = cairo_create(crCursor);

	Util::cairo_set_source_rgbi(cr, handler->getColor());

	if (big) {
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_set_line_width(cr, 1.2);
		cairo_move_to(cr, 1.5, 1.5);
		cairo_line_to(cr, 2, 19);
		cairo_line_to(cr, 5.5, 15.5);
		cairo_line_to(cr, 8.5, 20.5);
		cairo_line_to(cr, 10.5, 19);
		cairo_line_to(cr, 8.5, 14);
		cairo_line_to(cr, 13, 14);
		cairo_close_path(cr);
		cairo_fill_preserve(cr);

		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_stroke(cr);

		Util::cairo_set_source_rgbi(cr, handler->getColor());
		cairo_rectangle(cr, 0, 0, 3, 3);
		cairo_fill(cr);
	} else {
		cairo_rectangle(cr, 0, 0, 3, 3);
		cairo_fill(cr);
	}

	cairo_destroy(cr);

	GdkPixbuf * pixbuf = f_pixbuf_from_cairo_surface(crCursor);
	//	cairo_surface_write_to_png(crCursor, "/home/andreas/tmp/01/1.png");
	//	gdk_pixbuf_save(pixbuf, "/home/andreas/tmp/01/2.png", "PNG", NULL, NULL);

	cairo_surface_destroy(crCursor);

	GdkCursor * cursor = gdk_cursor_new_from_pixbuf(gtk_widget_get_display(control->getWindow()->getXournal()->getWidget()), pixbuf, 1, 1);

	gdk_pixbuf_unref(pixbuf);

	return cursor;
}

void Cursor::updateCursor() {
	g_return_if_fail(this != NULL);

	MainWindow * win = control->getWindow();
	if (!win) {
		return;
	}

	XournalView * xournal = win->getXournal();
	if (!xournal) {
		return;
	}

	GdkCursor * cursor = NULL;

	if (this->busy) {
		cursor = gdk_cursor_new(GDK_WATCH);
	} else {
		ToolHandler * handler = control->getToolHandler();
		ToolType type = handler->getToolType();

		if (type == TOOL_HAND) {
			if (this->mouseDown) {
				cursor = gdk_cursor_new(GDK_FLEUR);
			} else {
				cursor = gdk_cursor_new(GDK_HAND1);
			}
		} else if(!this->insidePage) {
			// not inside page: so use default cursor
		} else if (this->selectionType) {
			switch (this->selectionType) {
			case CURSOR_SELECTION_MOVE:
				cursor = gdk_cursor_new(GDK_FLEUR);
				break;
			case CURSOR_SELECTION_TOP_LEFT:
				cursor = gdk_cursor_new(GDK_TOP_LEFT_CORNER);
				break;
			case CURSOR_SELECTION_TOP_RIGHT:
				cursor = gdk_cursor_new(GDK_TOP_RIGHT_CORNER);
				break;
			case CURSOR_SELECTION_BOTTOM_LEFT:
				cursor = gdk_cursor_new(GDK_BOTTOM_LEFT_CORNER);
				break;
			case CURSOR_SELECTION_BOTTOM_RIGHT:
				cursor = gdk_cursor_new(GDK_BOTTOM_RIGHT_CORNER);
				break;
			case CURSOR_SELECTION_LEFT:
			case CURSOR_SELECTION_RIGHT:
				cursor = gdk_cursor_new(GDK_SB_H_DOUBLE_ARROW);
				break;
			case CURSOR_SELECTION_TOP:
			case CURSOR_SELECTION_BOTTOM:
				cursor = gdk_cursor_new(GDK_SB_V_DOUBLE_ARROW);
				break;
			}
		} else if (type == TOOL_PEN) {
			cursor = getPenCursor();
		} else if (type == TOOL_ERASER) {
			GdkColor bg = { 0, 65535, 65535, 65535 };
			GdkColor fg = { 0, 0, 0, 0 };
			GdkPixmap * source = gdk_bitmap_create_from_data(NULL, CURSOR_HIGLIGHTER_BITS, 16, 16);
			GdkPixmap * mask = gdk_bitmap_create_from_data(NULL, CURSOR_HILIGHTER_MASK, 16, 16);
			cursor = gdk_cursor_new_from_pixmap(source, mask, &fg, &bg, 7, 7);
			gdk_bitmap_unref(source);
			gdk_bitmap_unref(mask);
		} else if (type == TOOL_HILIGHTER) {
			GdkColor fg = { 0, 0, 0, 0 };
			GdkColor bg = handler->getGdkColor();
			GdkPixmap * source = gdk_bitmap_create_from_data(NULL, CURSOR_HIGLIGHTER_BITS, 16, 16);
			GdkPixmap * mask = gdk_bitmap_create_from_data(NULL, CURSOR_HILIGHTER_MASK, 16, 16);
			cursor = gdk_cursor_new_from_pixmap(source, mask, &fg, &bg, 7, 7);
			gdk_bitmap_unref(source);
			gdk_bitmap_unref(mask);
		} else if (type == TOOL_TEXT) {
			if (this->invisible) {
				cursor = gdk_cursor_new(GDK_BLANK_CURSOR);
			} else {
				cursor = gdk_cursor_new(GDK_XTERM);
			}
		} else if (type == TOOL_IMAGE) {
			// No special cursor needed
		} else if (type == TOOL_VERTICAL_SPACE) {
			if (this->mouseDown) {
				cursor = gdk_cursor_new(GDK_SB_V_DOUBLE_ARROW);
			}
		} else if (type != TOOL_SELECT_OBJECT) { // other selections are handled before anyway, because you can move a selection with every tool
			cursor = gdk_cursor_new(GDK_TCROSS);
		}
	}

	if (gtk_widget_get_window(xournal->getWidget())) {
		gdk_window_set_cursor(gtk_widget_get_window(xournal->getWidget()), cursor);

		gtk_widget_set_sensitive(xournal->getWidget(), !this->busy);
	}

	gdk_display_sync(gdk_display_get_default());

	if (cursor) {
		gdk_cursor_unref(cursor);
	}
}
