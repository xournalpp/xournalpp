/*
 * Xournal++
 *
 * Xournal widget which is the "View" widget
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __XOURNALWIDGET_H__
#define __XOURNALWIDGET_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_XOURNAL(obj) GTK_CHECK_CAST(obj, gtk_xournal_get_type (), GtkXournal)
#define GTK_XOURNAL_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, gtk_xournal_get_type(), GtkXournalClass)
#define GTK_IS_XOURNAL(obj) GTK_CHECK_TYPE(obj, gtk_xournal_get_type())

class XournalView;

typedef struct _GtkXournal GtkXournal;
typedef struct _GtkXournalClass GtkXournalClass;

struct _GtkXournal {
	GtkWidget widget;

	/**
	 * The view class
	 */
	XournalView * view;

	int scrollX;
	int scrollY;
};

struct _GtkXournalClass {
	GtkWidgetClass parent_class;
};

GtkType gtk_xournal_get_type(void);
GtkWidget * gtk_xournal_new(XournalView * view);

void gtk_xournal_redraw(GtkWidget * widget);
void gtk_xournal_update_xevent(GtkWidget * widget);

G_END_DECLS

#endif /* __XOURNALWIDGET_H__ */
