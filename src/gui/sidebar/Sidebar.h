/*
 * Xournal++
 *
 * The Sidebar
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/DocumentChangeType.h"
#include "model/DocumentListener.h"

#include <XournalType.h>

#include <gtk/gtk.h>

#include <list>

class AbstractSidebarPage;
class Control;
class GladeGui;
class SidebarPageButton;

class Sidebar : public DocumentListener
{
public:
	Sidebar(GladeGui* gui, Control* control);
	virtual ~Sidebar();

private:
	void initPages(GtkWidget* sidebar, GladeGui* gui);
	void addPage(AbstractSidebarPage* page);

public:

	/**
	 * Initialize the background to white, need to be done after the widget is
	 * added to the main window, else it's not working
	 */
	void setBackgroundWhite();

	/**
	 * A page was selected, so also select this page in the sidebar
	 */
	void selectPageNr(size_t page, size_t pdfPage);

	Control* getControl();

	/**
	 * Sets the current selected page
	 */
	void setSelectedPage(size_t page);

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
	virtual void pageSizeChanged(size_t page);
	virtual void pageChanged(size_t page);
	virtual void pageInserted(size_t page);
	virtual void pageDeleted(size_t page);
	virtual void pageSelected(size_t page);

private:

	/**
	 * Page selected
	 */
	static void buttonClicked(GtkToolButton* toolbutton, SidebarPageButton* buttonData);

private:
	XOJ_TYPE_ATTRIB;

	Control* control;

	/**
	 * The sidebar pages
	 */
	std::list<AbstractSidebarPage*> pages;

	/**
	 * The Toolbar with the pages
	 */
	GtkToolbar* tbSelectPage;

	/**
	 * The close button of the sidebar
	 */
	GtkWidget* buttonCloseSidebar;

	/**
	 * The current visible page in the sidebar
	 */
	GtkWidget* visiblePage;

	/**
	 * The sidebar widget
	 */
	GtkWidget* sidebar;
};

class SidebarPageButton
{
public:
	SidebarPageButton(Sidebar* sidebar, int index, AbstractSidebarPage* page);

public:
	Sidebar* sidebar;
	int index;
	AbstractSidebarPage* page;

};
