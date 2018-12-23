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

#include <gtk/gtk.h>

#include <XournalType.h>

class GladeSearchpath;

class GladeGui
{
public:
	GladeGui(GladeSearchpath* gladeSearchPath, string glade, string mainWnd);
	virtual ~GladeGui();

	virtual void show(GtkWindow* parent) = 0;

	operator GtkWindow* ();
	operator GdkWindow* ();

	GtkWidget* get(string name);
	GtkWidget* loadIcon(string name);
	GdkPixbuf* loadIconPixbuf(string filename);

	GtkWidget* getWindow();
	GladeSearchpath* getGladeSearchPath();

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * The Glade resources
	 */
	GtkBuilder* builder;

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
