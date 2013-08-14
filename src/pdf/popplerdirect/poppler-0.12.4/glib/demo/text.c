/*
 * Copyright (C) 2008 Carlos Garcia Campos  <carlosgc@gnome.org>
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

#include <string.h>

#include "text.h"

typedef struct {
	PopplerDocument *doc;

	GtkWidget       *timer_label;
	GtkTextBuffer   *buffer;

	gint             page;
} PgdTextDemo;

static void
pgd_text_free (PgdTextDemo *demo)
{
	if (!demo)
		return;

	if (demo->doc) {
		g_object_unref (demo->doc);
		demo->doc = NULL;
	}

	if (demo->buffer) {
		g_object_unref (demo->buffer);
		demo->buffer = NULL;
	}

	g_free (demo);
}

static void
pgd_text_get_text (GtkWidget   *button,
		   PgdTextDemo *demo)
{
	PopplerPage     *page;
	PopplerRectangle rect;
	gdouble          width, height;
	gchar           *text;
	GTimer          *timer;

	page = poppler_document_get_page (demo->doc, demo->page);
	if (!page)
		return;

	poppler_page_get_size (page, &width, &height);
	rect.x1 = rect.y1 = 0;
	rect.x2 = width;
	rect.y2 = height;

	timer = g_timer_new ();
	text = poppler_page_get_text (page, POPPLER_SELECTION_GLYPH, &rect);
	g_timer_stop (timer);

	if (text) {
		gchar *str;

		str = g_strdup_printf ("<i>got text in %.4f seconds</i>",
				       g_timer_elapsed (timer, NULL));
		gtk_label_set_markup (GTK_LABEL (demo->timer_label), str);
		g_free (str);
	} else {
		gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No text found</i>");
	}

	g_timer_destroy (timer);
	g_object_unref (page);

	if (text) {
		gtk_text_buffer_set_text (demo->buffer, text, strlen (text));
		g_free (text);
	}
}

static void
pgd_text_page_selector_value_changed (GtkSpinButton *spinbutton,
				      PgdTextDemo   *demo)
{
	demo->page = (gint)gtk_spin_button_get_value (spinbutton) - 1;
}

GtkWidget *
pgd_text_create_widget (PopplerDocument *document)
{
	PgdTextDemo *demo;
	GtkWidget   *label;
	GtkWidget   *vbox;
	GtkWidget   *hbox, *page_selector;
	GtkWidget   *button;
	GtkWidget   *swindow, *textview;
	gchar       *str;
	gint         n_pages;

	demo = g_new0 (PgdTextDemo, 1);

	demo->doc = g_object_ref (document);

	n_pages = poppler_document_get_n_pages (document);

	vbox = gtk_vbox_new (FALSE, 12);

	hbox = gtk_hbox_new (FALSE, 6);

	label = gtk_label_new ("Page:");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);

	page_selector = gtk_spin_button_new_with_range (1, n_pages, 1);
	g_signal_connect (G_OBJECT (page_selector), "value-changed",
			  G_CALLBACK (pgd_text_page_selector_value_changed),
			  (gpointer)demo);
	gtk_box_pack_start (GTK_BOX (hbox), page_selector, FALSE, TRUE, 0);
	gtk_widget_show (page_selector);

	str = g_strdup_printf ("of %d", n_pages);
	label = gtk_label_new (str);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);
	g_free (str);

	button = gtk_button_new_with_label ("Get Text");
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (pgd_text_get_text),
			  (gpointer)demo);
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show (button);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);

	demo->timer_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No text found</i>");
	g_object_set (G_OBJECT (demo->timer_label), "xalign", 1.0, NULL);
	gtk_box_pack_start (GTK_BOX (vbox), demo->timer_label, FALSE, TRUE, 0);
	gtk_widget_show (demo->timer_label);

	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	
	demo->buffer = gtk_text_buffer_new (NULL);
	textview = gtk_text_view_new_with_buffer (demo->buffer);

	gtk_container_add (GTK_CONTAINER (swindow), textview);
	gtk_widget_show (textview);

	gtk_box_pack_start (GTK_BOX (vbox), swindow, TRUE, TRUE, 0);
	gtk_widget_show (swindow);

	g_object_weak_ref (G_OBJECT (vbox),
			   (GWeakNotify)pgd_text_free,
			   demo);

	return vbox;
}
