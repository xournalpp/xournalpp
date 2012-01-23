#include "Cursor.h"
#include "XournalView.h"
#include "../control/Control.h"
#include <Util.h>
#include <pixbuf-utils.h>

/************** drawing nice cursors *********/

static char CURSOR_HIGLIGHTER_BITS[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
		0x08, 0x08, 0x08, 0xf8, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static char CURSOR_HILIGHTER_MASK[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f, 0xf8, 0x0f, 0xf8, 0x0f, 0xf8, 0x0f, 0xf8, 0x0f, 0xf8, 0x0f, 0xf8, 0x0f,
		0xf8, 0x0f, 0xf8, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

Cursor::Cursor(Control * control) {
	XOJ_INIT_TYPE(Cursor);

	this->control = control;
	this->busy = false;
	this->invisible = false;
	this->selectionType = CURSOR_SELECTION_NONE;
	this->insidePage = false;

	this->mouseDown = false;
}

Cursor::~Cursor() {
	XOJ_RELEASE_TYPE(Cursor);
}

void Cursor::setMouseDown(bool mouseDown) {
	XOJ_CHECK_TYPE(Cursor);

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
	XOJ_CHECK_TYPE(Cursor);

	if (this->selectionType == selectionType) {
		return;
	}
	this->selectionType = selectionType;
	updateCursor();
}

void Cursor::setCursorBusy(bool busy) {
	XOJ_CHECK_TYPE(Cursor);

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
	XOJ_CHECK_TYPE(Cursor);

	if(this->insidePage == insidePage) {
		return;
	}

	this->insidePage = insidePage;

	updateCursor();
}

void Cursor::setInvisible(bool invisible) {
	XOJ_CHECK_TYPE(Cursor);

	g_return_if_fail(this != NULL);

	if (this->invisible == invisible) {
		return;
	}

	this->invisible = invisible;

	updateCursor();
}

GdkCursor * Cursor::getPenCursor() {
	XOJ_CHECK_TYPE(Cursor);

	ToolHandler * handler = control->getToolHandler();

	bool big = control->getSettings()->isShowBigCursor();

	int height = 3;
	int width = 3;
	if (big) {
		height = 22;
		width = 15;
	}

	cairo_surface_t * crCursor = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
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

	GdkPixbuf * pixbuf = xoj_pixbuf_get_from_surface(crCursor, 0, 0, width, height);

//	cairo_surface_write_to_png(crCursor, "/home/andreas/xoj-cursor-orig.png");
//	gdk_pixbuf_save(pixbuf, "/home/andreas/xoj-cursor.png", "png", NULL, NULL);

	cairo_surface_destroy(crCursor);

	GdkCursor * cursor = gdk_cursor_new_from_pixbuf(gtk_widget_get_display(control->getWindow()->getXournal()->getWidget()), pixbuf, 1, 1);

	gdk_pixbuf_unref(pixbuf);

	return cursor;
}

//void fix_buffer_after_cairo(GdkPixbuf * pixbuf)
//{
//  guint8 * pixels = gdk_pixbuf_get_pixels(pixbuf);
//  int height = gdk_pixbuf_get_height(pixbuf);
//  int width = gdk_pixbuf_get_height(pixbuf);
//  int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
//
//  guint8 tmp;
//  guint8* p;
//  guint8* end;
//
//  for (int j = height; j > 0; --j)
//  {
//    p = pixels;
//    end = p + 4 * width;
//    while (p < end)
//    {
//      tmp = p[0];
//      if (G_BYTE_ORDER == G_LITTLE_ENDIAN)
//      {
//        p[0] = p[2]; p[2] = tmp;
//      }
//      else
//      {
//        p[0] = p[1]; p[1] = p[2]; p[2] = p[3]; p[3] = tmp;
//      }
//      p += 4;
//    }
//    pixels += rowstride;
//  }
//}
//}


void Cursor::updateCursor() {
	XOJ_CHECK_TYPE(Cursor);

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


			// TODO: Cursor
//			int width = 20;
//			int height = 20;
//			GdkPixbuf *  pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, width, height);
//
//			cairo_surface_t * surface = cairo_image_surface_create_for_data(gdk_pixbuf_get_pixels(pixbuf), CAIRO_FORMAT_ARGB32,
//					width, height, gdk_pixbuf_get_rowstride(pixbuf));
//
//			cairo_t * cr = cairo_create(surface);
//
//			cairo_save(cr);
//			cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
//			cairo_paint(cr);
//			cairo_restore(cr);
//
//			cairo_rectangle(cr, 2, 2, 16, 16);
//			cairo_set_source_rgba(cr, 1, 0, 0 , 0.5);
//			cairo_fill(cr);
//			cairo_destroy(cr);
//
//			cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), pixbuf, 10, 10 );



//			GdkPixbuf * source = gdk_pixbuf_new_from_file("/home/andreas/Desktop/cursor/foreground.png", NULL);
//			GdkPixbuf * mask = gdk_pixbuf_new_from_file("/home/andreas/Desktop/cursor/mask.png", NULL);
////
////
////			GdkColor bg = { 65535, 65535, 65535, 65535 };
////			GdkColor fg = { 0, 65535, 0, 0 };
//
//			mask->_GdkPixbuf
//
//			cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), mask, 10, 10 );
//
//			gdk_bitmap_unref(source);
//			gdk_bitmap_unref(mask);


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
