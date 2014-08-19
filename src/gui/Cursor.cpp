#include "Cursor.h"
#include "XournalView.h"
#include "../control/Control.h"
#include <Util.h>
#include <pixbuf-utils.h>
#include <math.h>


Cursor::Cursor(Control* control)
{
	XOJ_INIT_TYPE(Cursor);

	this->control = control;
	this->busy = false;
	this->invisible = false;
	this->selectionType = CURSOR_SELECTION_NONE;
	this->insidePage = false;

	this->mouseDown = false;
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
		GdkCursor* cursor = gdk_cursor_new(GDK_WATCH);
		if (gtk_widget_get_window(win->getWindow()))
		{
			gdk_window_set_cursor(gtk_widget_get_window(win->getWindow()), cursor);
		}

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

	if(this->insidePage == insidePage)
	{
		return;
	}

	this->insidePage = insidePage;

	updateCursor();
}

void Cursor::setInvisible(bool invisible)
{
	XOJ_CHECK_TYPE(Cursor);

	g_return_if_fail(this != NULL);

	if (this->invisible == invisible)
	{
		return;
	}

	this->invisible = invisible;

	updateCursor();
}

GdkCursor* Cursor::getPenCursor()
{
	XOJ_CHECK_TYPE(Cursor);

	ToolHandler* handler = control->getToolHandler();

	bool big = control->getSettings()->isShowBigCursor();

	int height = 3;
	int width = 3;
	if (big)
	{
		height = 22;
		width = 15;
	}

	cairo_surface_t* crCursor = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
	                                                       width, height);
	cairo_t* cr = cairo_create(crCursor);

	Util::cairo_set_source_rgbi(cr, handler->getColor());

	if (big)
	{
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
	}
	else
	{
		cairo_rectangle(cr, 0, 0, 3, 3);
		cairo_fill(cr);
	}

	cairo_destroy(cr);

	GdkPixbuf* pixbuf = xoj_pixbuf_get_from_surface(crCursor, 0, 0, width, height);

	cairo_surface_destroy(crCursor);

	GdkCursor* cursor = gdk_cursor_new_from_pixbuf(gtk_widget_get_display(
	                                                   control->getWindow()->getXournal()->getWidget()), pixbuf, 1, 1);

	g_object_unref(pixbuf);

	return cursor;
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

	GdkCursor* cursor = NULL;

	if (this->busy)
	{
		cursor = gdk_cursor_new(GDK_WATCH);
	}
	else
	{
		ToolHandler* handler = control->getToolHandler();
		ToolType type = handler->getToolType();

		if (type == TOOL_HAND)
		{
			if (this->mouseDown)
			{
				cursor = gdk_cursor_new(GDK_FLEUR);
			}
			else
			{
				cursor = gdk_cursor_new(GDK_HAND1);
			}
		}
		else if(!this->insidePage)
		{
			// not inside page: so use default cursor
		}
		else if (this->selectionType)
		{
			switch (this->selectionType)
			{
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
		}
		else if (type == TOOL_PEN)
		{
			cursor = getPenCursor();

		}
		else if (type == TOOL_ERASER)
		{
			cursor = eraserCursor();
		}
		else if (type == TOOL_HILIGHTER)
		{
			cursor = highlighterCursor();
		}
		else if (type == TOOL_TEXT)
		{
			if (this->invisible)
			{
				cursor = gdk_cursor_new(GDK_BLANK_CURSOR);
			}
			else
			{
				cursor = gdk_cursor_new(GDK_XTERM);
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
				cursor = gdk_cursor_new(GDK_SB_V_DOUBLE_ARROW);
			}
		}
		else if (type != TOOL_SELECT_OBJECT)     // other selections are handled before anyway, because you can move a selection with every tool
		{
			cursor = gdk_cursor_new(GDK_TCROSS);
		}
	}

	if (gtk_widget_get_window(xournal->getWidget()))
	{
		gdk_window_set_cursor(gtk_widget_get_window(xournal->getWidget()), cursor);

		gtk_widget_set_sensitive(xournal->getWidget(), !this->busy);
	}

	gdk_display_sync(gdk_display_get_default());

	if (cursor)
	{
		g_object_unref(cursor);
	}
}

GdkCursor* Cursor::eraserCursor()
{
	//g_message("Creating eraser cursor");

	Tool& eraser = control->getToolHandler()->getTool(TOOL_ERASER);
	GdkCursor* cursor = NULL;

	/*
	const double zoom = control->getZoomControl()->getZoom();
	const double cursorSize = 2*zoom*thickness;
	const double thickness = eraser.getThickness(eraser.getSize());
	*/

	const double cursorSize = 8;

	//g_message("Cursor size: %f", cursorSize);

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

	cursor = gdk_cursor_new_from_surface(gdk_display_get_default(),
	                                    surface,
	                                    cursorSize/2.,
	                                    cursorSize/2.);

	cairo_surface_destroy(surface);

	return cursor;
}

GdkCursor* Cursor::highlighterCursor()
{
	Tool& highlighter = control->getToolHandler()->getTool(TOOL_HILIGHTER);

	/*
	const double thickness = highlighter.getThickness(highlighter.getSize());
	const double zoom = control->getZoomControl()->getZoom();
	const double cursorSize = zoom*thickness;
	*/

	const double cursorSize = 8;

	GdkCursor* cursor = NULL;
	int rgb = highlighter.getColor();

	double r = ((rgb >> 16) & 0xff) / 255.0;
	double g = ((rgb >> 8) & 0xff) / 255.0;
	double b = (rgb & 0xff) / 255.0;

	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
	                                                      cursorSize,
	                                                      cursorSize);

	cairo_t* cr = cairo_create(surface);

	cairo_rectangle(cr, 0, 0, cursorSize, cursorSize);
	cairo_set_source_rgba(cr, 1, 1, 1, 0);
	cairo_fill(cr);

	cairo_arc(cr, cursorSize/2., cursorSize/2., cursorSize/2.-1, 0, 2*M_PI);

	cairo_set_source_rgba(cr, r, g, b, 120/255.);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	cairo_fill(cr);

	cairo_destroy(cr);

	cursor = gdk_cursor_new_from_surface(gdk_display_get_default(),
	                                     surface,
	                                     cursorSize/2.,
	                                     cursorSize/2.);

	cairo_surface_destroy(surface);

	return cursor;
}
