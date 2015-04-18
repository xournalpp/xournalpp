/*
 * Xournal++
 *
 * Control to display a single color button
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

#define SELECT_COLOR(obj) GTK_CHECK_CAST(obj, selectcolor_get_type(), SelectColor)
#define SELECT_COLOR_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, selectcolor_get_type(), SelectColorClass)
#define IS_SELECT_COLOR(obj) GTK_CHECK_TYPE(obj, selectcolor_get_type())

typedef struct _SelectColor SelectColor;
typedef struct _SelectColorClass SelectColorClass;

struct _SelectColor
{
	//	GtkWidget widget;
	GtkMisc widget;

	gint color;
	gboolean circle;

	gint size;
};

struct _SelectColorClass
{
	GtkMiscClass parent_class;
};

GtkType selectcolor_get_type(void);
GtkWidget* selectcolor_new(gint color);
void selectcolor_set_color(GtkWidget* sc, gint color);
void selectcolor_set_circle(GtkWidget* sc, gboolean circle);
void selectcolor_set_size(GtkWidget* sc, gint size);

G_END_DECLS
