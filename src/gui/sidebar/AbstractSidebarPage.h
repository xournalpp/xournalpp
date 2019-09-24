/*
 * Xournal++
 *
 * Abstract Sidebar Page
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

#include <XournalType.h>

#include <gtk/gtk.h>

class Control;
class SidebarToolbar;

class AbstractSidebarPage : public DocumentListener, public SidebarToolbarActionListener
{
public:
	AbstractSidebarPage(Control* control, SidebarToolbar* toolbar);
	virtual ~AbstractSidebarPage();

public:
	virtual void enableSidebar() = 0;
	virtual void disableSidebar() = 0;

	/**
	 * The name of this sidebar page
	 */
	virtual string getName() = 0;

	/**
	 * The name of this sidebar page
	 */
	virtual string getIconName() = 0;

	/**
	 * If this sidebar page has data for the current document, e.g. if there are bookmarks or not
	 */
	virtual bool hasData() = 0;

	/**
	 * Gets the Widget to display
	 */
	virtual GtkWidget* getWidget() = 0;

	/**
	 * Temporary disable Sidebar (e.g. while saving)
	 */
	virtual void setTmpDisabled(bool disabled);

	/**
	 * A page was selected, supply additional to the Document Page the PDF Page
	 */
	virtual void selectPageNr(size_t page, size_t pdfPage);

	/**
	 * Returns the Application controller
	 */
	Control* getControl();

private:
	protected:
	/**
	 * The Control of the Application
	 */
	Control* control = nullptr;

	/**
	 * The Toolbar to move, copy & delete pages
	 */
	SidebarToolbar* toolbar = nullptr;

public:
	/**
	 * The Sidebar button
	 */
	GtkToolItem* tabButton = nullptr;

};
