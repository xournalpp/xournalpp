/*
 * Xournal++
 *
 * Xournal widget which is the "View" widget
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_XOURNAL(obj) GTK_CHECK_CAST(obj, gtk_xournal_get_type (), GtkXournal)
#define GTK_XOURNAL_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, gtk_xournal_get_type(), GtkXournalClass)
#define GTK_IS_XOURNAL(obj) GTK_CHECK_TYPE(obj, gtk_xournal_get_type())

class EditSelection;
class Layout;
class PageView;
class PagePositionCache;
class Rectangle;
class XournalView;

typedef struct _GtkXournal GtkXournal;
typedef struct _GtkXournalClass GtkXournalClass;

struct _GtkXournal
{
	GtkWidget widget;

	/**
	 * The view class
	 */
	XournalView* view;

	/**
	 * Visible area
	 */
	int x;
	int y;

	int scrollX;
	int scrollY;

	Layout* layout;

	PageView* currentInputPage;
	PagePositionCache* pagePositionCache;

	/**
	 * The last Mouse Position, for scrolling
	 */
	int lastMousePositionX;
	int lastMousePositionY;
	int scrollOffsetX;
	int scrollOffsetY;
	bool inScrolling;

	/**
	 * Selected content, if any
	 */
	EditSelection* selection;
};

struct _GtkXournalClass
{
	GtkWidgetClass parent_class;
};

GtkType gtk_xournal_get_type(void);
GtkWidget* gtk_xournal_new(XournalView* view);

void gtk_xournal_update_xevent(GtkWidget* widget);

cairo_t* gtk_xournal_create_cairo_for(GtkWidget* widget, PageView* view);

Layout* gtk_xournal_get_layout(GtkWidget* widget);

void gtk_xournal_scroll_relative(GtkWidget* widget, double x, double y);

void gtk_xournal_repaint_area(GtkWidget* widget, int x1, int y1, int x2, int y2);

Rectangle* gtk_xournal_get_visible_area(GtkWidget* widget, PageView* p);

G_END_DECLS
