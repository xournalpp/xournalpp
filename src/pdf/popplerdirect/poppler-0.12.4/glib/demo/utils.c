/*
 * Copyright (C) 2007 Carlos Garcia Campos  <carlosgc@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <gtk/gtk.h>
#include <time.h>

#include "utils.h"

void
pgd_table_add_property_with_value_widget (GtkTable    *table,
					  const gchar *markup,
					  GtkWidget  **value_widget,
					  const gchar *value,
					  gint        *row)
{
	GtkWidget *label;

	label = gtk_label_new (NULL);
	g_object_set (G_OBJECT (label), "xalign", 0.0, NULL);
	gtk_label_set_markup (GTK_LABEL (label), markup);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, *row, *row + 1,
			  GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (label);

	*value_widget = label = gtk_label_new (value);
	g_object_set (G_OBJECT (label),
		      "xalign", 0.0,
		      "selectable", TRUE,
		      "ellipsize", PANGO_ELLIPSIZE_END,
		      NULL);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, *row, *row + 1,
			  GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	gtk_widget_show (label);

	*row += 1;
}

void
pgd_table_add_property (GtkTable    *table,
			const gchar *markup,
			const gchar *value,
			gint        *row)
{
	GtkWidget *label;

	pgd_table_add_property_with_value_widget (table, markup, &label, value, row);
}

GtkWidget *
pgd_action_view_new (PopplerDocument *document)
{
	GtkWidget  *frame, *label;

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), "<b>Action Properties</b>");
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_widget_show (label);

	g_object_set_data (G_OBJECT (frame), "document", document);

	return frame;
}

static void
pgd_action_view_add_destination (GtkWidget   *action_view,
				 GtkTable    *table,
				 PopplerDest *dest,
				 gboolean     remote,
				 gint        *row)
{
	PopplerDocument *document;
	GEnumValue      *enum_value;
	gchar           *str;
	
	pgd_table_add_property (table, "<b>Type:</b>", "Destination", row);
	
	enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_DEST_TYPE), dest->type);
	pgd_table_add_property (table, "<b>Destination Type:</b>", enum_value->value_name, row);

	document = g_object_get_data (G_OBJECT (action_view), "document");
	
	if (dest->type != POPPLER_DEST_NAMED) {
		str = NULL;
		
		if (document && !remote) {
			PopplerPage *poppler_page;
			gchar       *page_label;
			
			poppler_page = poppler_document_get_page (document, MAX (0, dest->page_num - 1));
			
			g_object_get (G_OBJECT (poppler_page),
				      "label", &page_label,
				      NULL);
			if (page_label) {
				str = g_strdup_printf ("%d (%s)", dest->page_num, page_label);
				g_free (page_label);
			}
		}
		
		if (!str)
			str = g_strdup_printf ("%d", dest->page_num);
		pgd_table_add_property (table, "<b>Page:</b>", str, row);
		g_free (str);
		
		str = g_strdup_printf ("%.2f", dest->left);
		pgd_table_add_property (table, "<b>Left:</b>", str, row);
		g_free (str);
		
		str = g_strdup_printf ("%.2f", dest->right);
		pgd_table_add_property (table, "<b>Right:</b>", str, row);
		g_free (str);
		
		str = g_strdup_printf ("%.2f", dest->top);
		pgd_table_add_property (table, "<b>Top:</b>", str, row);
		g_free (str);
	
		str = g_strdup_printf ("%.2f", dest->bottom);
		pgd_table_add_property (table, "<b>Bottom:</b>", str, row);
		g_free (str);
	
		str = g_strdup_printf ("%.2f", dest->zoom);
		pgd_table_add_property (table, "<b>Zoom:</b>", str, row);
		g_free (str);
	} else {
		pgd_table_add_property (table, "<b>Named Dest:</b>", dest->named_dest, row);

		if (document && !remote) {
			PopplerDest *new_dest;

			new_dest = poppler_document_find_dest (document, dest->named_dest);
			if (new_dest) {
				GtkWidget *new_table, *alignment;
				gint       new_row = 0;

				alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
				gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 5, 5, 12, 5);
				
				new_table = gtk_table_new (8, 2, FALSE);
				gtk_table_set_col_spacings (GTK_TABLE (new_table), 6);
				gtk_table_set_row_spacings (GTK_TABLE (new_table), 6);
				gtk_table_attach_defaults (table, alignment, 0, 2, *row, *row + 1);
				gtk_widget_show (alignment);
				
				pgd_action_view_add_destination (action_view, GTK_TABLE (new_table),
								 new_dest, FALSE, &new_row);
				poppler_dest_free (new_dest);

				gtk_container_add (GTK_CONTAINER (alignment), new_table);
				gtk_widget_show (new_table);

				*row += 1;
			}
		}
	}
}

void
pgd_action_view_set_action (GtkWidget     *action_view,
			    PopplerAction *action)
{
	GtkWidget  *alignment;
	GtkWidget  *table;
	gint        row = 0;

	alignment = gtk_bin_get_child (GTK_BIN (action_view));
	if (alignment) {
		gtk_container_remove (GTK_CONTAINER (action_view), alignment);
	}
	
	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 5, 5, 12, 5);
	gtk_container_add (GTK_CONTAINER (action_view), alignment);
	gtk_widget_show (alignment);

	if (!action)
		return;

	table = gtk_table_new (10, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 6);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);

	pgd_table_add_property (GTK_TABLE (table), "<b>Title:</b>", action->any.title, &row);
	
	switch (action->type) {
	case POPPLER_ACTION_UNKNOWN:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "Unknown", &row);
		break;
	case POPPLER_ACTION_NONE:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "None", &row);
		break;
	case POPPLER_ACTION_GOTO_DEST:
		pgd_action_view_add_destination (action_view, GTK_TABLE (table), action->goto_dest.dest, FALSE, &row);
		break;
	case POPPLER_ACTION_GOTO_REMOTE:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "Remote Destination", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Filename:</b>", action->goto_remote.file_name, &row);
		pgd_action_view_add_destination (action_view, GTK_TABLE (table), action->goto_remote.dest, TRUE, &row);
		break;
	case POPPLER_ACTION_LAUNCH:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "Launch", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Filename:</b>", action->launch.file_name, &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Params:</b>", action->launch.params, &row);
		break;
	case POPPLER_ACTION_URI:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "External URI", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>URI</b>", action->uri.uri, &row);
		break;
	case POPPLER_ACTION_NAMED:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "Named Action", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Name:</b>", action->named.named_dest, &row);
		break;
	case POPPLER_ACTION_MOVIE:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "Movie", &row);
		break;
	default:
		g_assert_not_reached ();
	}

	gtk_container_add (GTK_CONTAINER (alignment), table);
	gtk_widget_show (table);
}

gchar *
pgd_format_date (time_t utime)
{
	time_t time = (time_t) utime;
	char s[256];
	const char *fmt_hack = "%c";
	size_t len;
#ifdef HAVE_LOCALTIME_R
	struct tm t;
	if (time == 0 || !localtime_r (&time, &t)) return NULL;
	len = strftime (s, sizeof (s), fmt_hack, &t);
#else
	struct tm *t;
	if (time == 0 || !(t = localtime (&time)) ) return NULL;
	len = strftime (s, sizeof (s), fmt_hack, t);
#endif

	if (len == 0 || s[0] == '\0') return NULL;

	return g_locale_to_utf8 (s, -1, NULL, NULL, NULL);
}
