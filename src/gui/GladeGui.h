/*
 * Xournal++
 *
 * Abstract gui class, which loads the glade objcts
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __GLADEGUI_H__
#define __GLADEGUI_H__

#include <gtk/gtk.h>
#include <glade/glade-xml.h>

class GladeGui;

typedef bool (GladeGui::*tFunction)(void * d1, void * d2);

class GladeGui {
public:
	GladeGui(const char * glade, const char * mainWnd);
	virtual ~GladeGui();

	static void addSearchDirectory(const char *directory);
	static char * findFile(const char * subdir, const char * file);

	virtual void show() = 0;

	GtkWidget * get(const char * name);
	GtkWidget * loadIcon(const char * name);
	GdkPixbuf * loadIconPixbuf(const char * filename);

	GtkWidget * getWindow();
private:
	/**
	 * Search directory for icons and Glade files
	 */
	static GList *directories;

	/**
	 * The Glade resources
	 */
	GladeXML *xml;

protected:
	/**
	 * This window
	 */
	GtkWidget * window;
};

#endif /* __GLADEGUI_H__ */
