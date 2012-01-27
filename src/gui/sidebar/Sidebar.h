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

#include "../model/DocumentChangeType.h"
#include "../model/DocumentListener.h"

class Control;
class GladeGui;
class AbstractSidebarPage;
class GladeGui;
class SidebarPageButton;

class Sidebar : public DocumentListener {
public:
	Sidebar(GladeGui * gui, Control * control);
	virtual ~Sidebar();

private:
	void initPages(GtkWidget * sidebar, GladeGui * gui);
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
	 * Sets the current selected page
	 */
	void setSelectedPage(int page);

	/**
	 * Enable active and siable inactive buttons, select first active page
	 */
	void updateEnableDisableButtons();

	/**
	 * Temporary disable Sidebar (e.g. while saving)
	 */
	void setTmpDisabled(bool disabled);

	/**
	 * Saves the current size to the settings
	 */
	void saveSize();

public:
	// DocumentListener interface
	virtual void documentChanged(DocumentChangeType type);
	virtual void pageSizeChanged(int page);
	virtual void pageChanged(int page);
	virtual void pageInserted(int page);
	virtual void pageDeleted(int page);
	virtual void pageSelected(int page);

private:

	/**
	 * Page selected
	 */
	static void buttonClicked(GtkToolButton * toolbutton, SidebarPageButton * buttonData);

private:
	XOJ_TYPE_ATTRIB;

	Control * control;

	/**
	 * The sidebar pages
	 */
	GList * pages;

	/**
	 * The Toolbar with the pages
	 */
	GtkToolbar * tbSelectPage;

	/**
	 * The close button of the sidebar
	 */
	GtkWidget * buttonCloseSidebar;

	/**
	 * The current visible page in the sidebar
	 */
	GtkWidget * visiblePage;

	/**
	 * The sidebar widget
	 */
	GtkWidget * sidebar;
};

class SidebarPageButton {
public:
	SidebarPageButton(Sidebar * sidebar, int index, AbstractSidebarPage * page);

public:
	Sidebar * sidebar;
	int index;
	AbstractSidebarPage * page;

};


#endif /* __SIDEBAR_H__ */
