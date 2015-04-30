/*
 * Xournal++
 *
 * Abstract GUI class, which loads the glade objects
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <glade/glade-xml.h>
#include <gtk/gtk.h>

#include <XournalType.h>

class GladeSearchpath;

class GladeGui
{
public:
	GladeGui(GladeSearchpath* gladeSearchPath, const char* glade, const char* mainWnd);
	virtual ~GladeGui();

	virtual void show(GtkWindow* parent) = 0;

	operator GtkWindow* ();
	operator GdkWindow* ();

	GtkWidget* get(const char* name);
	GtkWidget* loadIcon(const char* name);
	GdkPixbuf* loadIconPixbuf(const char* filename);

	GtkWidget* getWindow();
	GladeSearchpath* getGladeSearchPath();

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * The Glade resources
	 */
	GladeXML* xml;

	/**
	 * Our search paths
	 */
	GladeSearchpath* gladeSearchPath;

protected:
	/**
	 * This window
	 */
	GtkWidget* window;
};
