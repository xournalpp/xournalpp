#include "Cursor.h"

#include "XournalView.h"
#include "control/Control.h"

#include <Util.h>
#include <pixbuf-utils.h>
#include <cmath>

Cursor::Cursor(Control* control)
 : control(control)
{
	XOJ_INIT_TYPE(Cursor);
}

Cursor::~Cursor()
{
	XOJ_RELEASE_TYPE(Cursor);
}

void Cursor::setMouseDown(bool mouseDown)
{
	XOJ_CHECK_TYPE(Cursor);

	if (this->mouseDown == mouseDown)
	{
		return;
	}

	this->mouseDown = mouseDown;
	ToolHandler* handler = control->getToolHandler();
	ToolType type = handler->getToolType();

	// Not always an update is needed
	if (type == TOOL_HAND || type == TOOL_VERTICAL_SPACE)
	{
		updateCursor();
	}
}

void Cursor::setMouseSelectionType(CursorSelectionType selectionType)
{
	XOJ_CHECK_TYPE(Cursor);

	if (this->selectionType == selectionType)
	{
		return;
	}
	this->selectionType = selectionType;
	updateCursor();
}

void Cursor::setCursorBusy(bool busy)
{
	XOJ_CHECK_TYPE(Cursor);

	MainWindow* win = control->getWindow();
	if (!win)
	{
		return;
	}

	if (this->busy == busy)
	{
		return;
	}

	this->busy = busy;

	if (busy)
	{
		GdkWindow* window = gtk_widget_get_window(win->getWindow());
		GdkCursor* cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_WATCH);
		gdk_window_set_cursor(window, cursor);
		g_object_unref(cursor);
	}
	else
	{
		if (gtk_widget_get_window(win->getWindow()))
		{
			gdk_window_set_cursor(gtk_widget_get_window(win->getWindow()), NULL);
		}
	}

	updateCursor();
}

void Cursor::setInsidePage(bool insidePage)
{
	XOJ_CHECK_TYPE(Cursor);

	if (this->insidePage == insidePage)
	{
		return;
	}

	this->insidePage = insidePage;

	updateCursor();
}

void Cursor::setInvisible(bool invisible)
{
	XOJ_CHECK_TYPE(Cursor);

	if (this->invisible == invisible)
	{
		return;
	}

	this->invisible = invisible;

	updateCursor();
}

void Cursor::updateCursor()
{
	XOJ_CHECK_TYPE(Cursor);

	MainWindow* win = control->getWindow();
	if (!win)
	{
		return;
	}

	XournalView* xournal = win->getXournal();
	if (!xournal)
	{
		return;
	}

	GdkWindow* window = gtk_widget_get_window(win->getWindow());

	GdkCursor* cursor = NULL;

	if (this->busy)
	{
		cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_WATCH);
	}
	else
	{
		ToolHandler* handler = control->getToolHandler();
		ToolType type = handler->getToolType();

		if (type == TOOL_HAND)
		{
			if (this->mouseDown)
			{
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_FLEUR);
			}
			else
			{
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_HAND1);
			}
		}
		else if (!this->insidePage)
		{
			// not inside page: so use default cursor
		}
		else if (this->selectionType)
		{
			switch (this->selectionType)
			{
			case CURSOR_SELECTION_MOVE:
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_FLEUR);
				break;
			case CURSOR_SELECTION_TOP_LEFT:
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_TOP_LEFT_CORNER);
				break;
			case CURSOR_SELECTION_TOP_RIGHT:
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_TOP_RIGHT_CORNER);
				break;
			case CURSOR_SELECTION_BOTTOM_LEFT:
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_BOTTOM_LEFT_CORNER);
				break;
			case CURSOR_SELECTION_BOTTOM_RIGHT:
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_BOTTOM_RIGHT_CORNER);
				break;
			case CURSOR_SELECTION_LEFT:
			case CURSOR_SELECTION_RIGHT:
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_SB_H_DOUBLE_ARROW);
				break;
			case CURSOR_SELECTION_ROTATE:
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_EXCHANGE);
				break;
			case CURSOR_SELECTION_TOP:
			case CURSOR_SELECTION_BOTTOM:
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_SB_V_DOUBLE_ARROW);
				break;
			default:
				break;
			}
		}
		else if (type == TOOL_PEN)
		{
			cursor = getPenCursor();

		}
		else if (type == TOOL_ERASER)
		{
			cursor = getEraserCursor();
		}
		else if (type == TOOL_HILIGHTER)
		{
			cursor = getHighlighterCursor();
		}
		else if (type == TOOL_TEXT)
		{
			if (this->invisible)
			{
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_BLANK_CURSOR);
			}
			else
			{
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_XTERM);
			}
		}
		else if (type == TOOL_IMAGE)
		{
			// No special cursor needed
		}
		else if (type == TOOL_VERTICAL_SPACE)
		{
			if (this->mouseDown)
			{
				cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_SB_V_DOUBLE_ARROW);
			}
		}
		else if (type == TOOL_SELECT_OBJECT)
		{
			cursor = gdk_cursor_new_from_name(gdk_window_get_display(window),"default");
		}
		else if (type == TOOL_PLAY_OBJECT) 
		{
			cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_HAND2);
		}
		else // other selections are handled before anyway, because you can move a selection with every tool
		{
			cursor = gdk_cursor_new_for_display(gdk_window_get_display(window), GDK_TCROSS);
		}

	}

	if (gtk_widget_get_window(xournal->getWidget()))
	{
		if (cursor != NULL)
		{
			gdk_window_set_cursor(gtk_widget_get_window(xournal->getWidget()), cursor);
		}

		gtk_widget_set_sensitive(xournal->getWidget(), !this->busy);
	}

	gdk_display_sync(gdk_display_get_default());

	if (cursor)
	{
		g_object_unref(cursor);
	}
}

GdkCursor* Cursor::getEraserCursor()
{
	// Eraser's size follow a quadratic increment, so the cursor will do the same
	double cursorSize = control->getToolHandler()->getThickness() * 2 * control->getZoomControl()->getZoom();

	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
	                                                      cursorSize,
	                                                      cursorSize);

	cairo_t* cr = cairo_create(surface);

	cairo_rectangle(cr, 0, 0, cursorSize, cursorSize);

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_fill(cr);

	cairo_rectangle(cr, 0, 0, cursorSize, cursorSize);

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_stroke(cr);

	cairo_destroy(cr);

	GdkCursor* cursor = gdk_cursor_new_from_surface(gdk_display_get_default(),
													surface,
													cursorSize / 2.0,
													cursorSize / 2.0);

	cairo_surface_destroy(surface);

	return cursor;
}

GdkCursor* Cursor::getHighlighterCursor()
{
	XOJ_CHECK_TYPE(Cursor);

	return createHighlighterOrPenCursor(5, 120 / 255.0);
}


GdkCursor* Cursor::getPenCursor()
{
	XOJ_CHECK_TYPE(Cursor);

	return createHighlighterOrPenCursor(3, 1.0);
}

GdkCursor* Cursor::createHighlighterOrPenCursor(int size, double alpha)
{
	XOJ_CHECK_TYPE(Cursor);

	int rgb = control->getToolHandler()->getColor();
	double r = ((rgb >> 16) & 0xff) / 255.0;
	double g = ((rgb >> 8) & 0xff) / 255.0;
	double b = (rgb & 0xff) / 255.0;

	bool big = control->getSettings()->isShowBigCursor();
	bool highlightPosition = control->getSettings()->isHighlightPosition();

	int height = size;
	int width = size;
	if (big || highlightPosition)
	{
		height = width = 60;
	}

	// We change the drawing method, now the center with the colored dot of the pen
	// is at the center of the cairo surface, and when we load the cursor, we load it
	// with the relative offset
	int centerX = width / 2;
	int centerY = height / 2; 

	cairo_surface_t* crCursor = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cairo_t* cr = cairo_create(crCursor);

	if (big)
	{
		// When using highlighter, paint the icon with the current color
		if (size == 5)
		{
			cairo_set_source_rgb(cr, r, g, b);
		}
		else
		{
			cairo_set_source_rgb(cr, 1, 1, 1);
		}
		cairo_set_line_width(cr, 1.2);
		
		// Plain cursor drawing + color dot
		// cairo_move_to(cr, 1.5, 1.5);
		// cairo_line_to(cr, 2, 19);
		// cairo_line_to(cr, 5.5, 15.5);
		// cairo_line_to(cr, 8.5, 20.5);
		// cairo_line_to(cr, 10.5, 19);
		// cairo_line_to(cr, 8.5, 14);
		// cairo_line_to(cr, 13, 14);


		// Starting point
		cairo_move_to(cr, centerX + 2, centerY);
		// Pencil cursor
		cairo_line_to(cr, centerX + 2, centerY - 4);
		cairo_line_to(cr, centerX + 15, centerY - 17.5);
		cairo_line_to(cr, centerX + 19, centerY - 14);
		cairo_line_to(cr, centerX + 6, centerY );

		cairo_close_path(cr);
		cairo_fill_preserve(cr);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_stroke(cr);

		cairo_fill_preserve(cr);
	}

	if (highlightPosition)
	{
		// A yellow transparent circle with no border
		cairo_set_line_width(cr, 0);
		cairo_set_source_rgba(cr, 255, 255, 0, 0.5);
		cairo_arc(cr, centerX, centerY, 30, 0, 2 * 3.1415);
		cairo_fill_preserve(cr);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_stroke(cr);

	}
	
	cairo_set_source_rgba(cr, r, g, b, alpha);
	// Correct the offset of the coloured dot for big-cursor mode
	cairo_rectangle(cr, centerX, centerY, size, size);
	cairo_fill(cr);

	cairo_destroy(cr);

	GdkPixbuf* pixbuf = xoj_pixbuf_get_from_surface(crCursor, 0, 0, width, height);

	//	cairo_surface_write_to_png(crCursor, "/home/andreas/xoj-cursor-orig.png");
	//	gdk_pixbuf_save(pixbuf, "/home/andreas/xoj-cursor.png", "png", NULL, NULL);

	cairo_surface_destroy(crCursor);
	
	GdkCursor* cursor = gdk_cursor_new_from_pixbuf(
	 		gtk_widget_get_display(control->getWindow()->getXournal()->getWidget()), pixbuf, centerX, centerY);

	g_object_unref(pixbuf);

	return cursor;
}

