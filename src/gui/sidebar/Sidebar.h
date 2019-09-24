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
#include "gui/sidebar/previews/base/SidebarToolbar.h"

#include <gtk/gtk.h>
#include <list>

class AbstractSidebarPage;
class Control;
class GladeGui;
class SidebarPageButton;

class Sidebar : public DocumentListener, public SidebarToolbarActionListener
{
public:
	Sidebar(GladeGui* gui, Control* control);
	virtual ~Sidebar();

private:
	void initPages(GtkWidget* sidebar, GladeGui* gui);
	void addPage(AbstractSidebarPage* page);

	// SidebarToolbarActionListener
public:
	/**
	 * Called when an action is performed
	 */
	virtual void actionPerformed(SidebarActions action);

public:
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

private:

	/**
	 * Page selected
	 */
	static void buttonClicked(GtkToolButton* toolbutton, SidebarPageButton* buttonData);

private:
	Control* control = nullptr;

	GladeGui* gui = nullptr;

	/**
	 * The sidebar pages
	 */
	std::list<AbstractSidebarPage*> pages;

	/**
	 * The Toolbar with the pages
	 */
	GtkToolbar* tbSelectPage = nullptr;

	/**
	 * The close button of the sidebar
	 */
	GtkWidget* buttonCloseSidebar = nullptr;

	/**
	 * The current visible page in the sidebar
	 */
	GtkWidget* visiblePage = nullptr;

	/**
	 * Current active page
	 */
	AbstractSidebarPage* currentPage = nullptr;

	/**
	 * The sidebar widget
	 */
	GtkWidget* sidebar = nullptr;

	/**
	 * Sidebar toolbar
	 */
	SidebarToolbar toolbar;
};

class SidebarPageButton
{
public:
	SidebarPageButton(Sidebar* sidebar, int index, AbstractSidebarPage* page);

public:
	Sidebar* sidebar = nullptr;
	int index = 0;
	AbstractSidebarPage* page = nullptr;

};
