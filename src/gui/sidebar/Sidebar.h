/*
 * Xournal++
 *
 * The Sidebar
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SIDEBAR_H__
#define __SIDEBAR_H__

#include <gtk/gtk.h>
#include <XournalType.h>


class Control;
class GladeGui;
class AbstractSidebarPage;

class Sidebar {
public:
	Sidebar(GladeGui * gui, Control * control);
	virtual ~Sidebar();

private:
	void initPages(GtkWidget * sidebar);
	void addPage(AbstractSidebarPage * page);

public:

	/**
	 * Initialize the background to white, need to be done after the widget is
	 * added to the main window, else it's not working
	 */
	void setBackgroundWhite();

	/**
	 * A page was selected, so also select this page in the sidebar
	 */
	void selectPageNr(int page, int pdfPage);

	Control * getControl();

	/**
	 * Temporary disable Sidebar (e.g. while saving)
	 */
	void setTmpDisabled(bool disabled);

private:

	/**
	 * Page selected
	 */
	static void cbChangedCallback(GtkComboBox * widget, Sidebar * sidebar);

private:
	XOJ_TYPE_ATTRIB;

	Control * control;

	GList * pages;

	/**
	 * The combobox with the pages
	 */
	GtkComboBox * comboBox;

	/**
	 * The close button of the sidebar
	 */
	GtkWidget * buttonCloseSidebar;

	/**
	 * The current visible page in the sidebar
	 */
	GtkWidget * visiblePage;

};

#endif /* __SIDEBAR_H__ */
