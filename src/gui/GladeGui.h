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

	void setThemePath(string themePath);

	GtkWidget* getWindow();
	GladeSearchpath* getGladeSearchPath();

 	GtkBuilder* getBuilder();

private:
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
	GtkWidget* window = nullptr;
};
