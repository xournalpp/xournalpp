/*
 * Xournal++
 *
 * Base class for Background selection dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/GladeGui.h"
#include <XournalType.h>

#include <vector>

class Document;
class Settings;
class BaseElementView;

using std::vector;

class BackgroundSelectDialogBase : public GladeGui
{
public:
	BackgroundSelectDialogBase(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings,
			string glade, string mainWnd);
	~BackgroundSelectDialogBase();

public:
	Settings* getSettings();
	virtual void show(GtkWindow* parent);
	virtual void setSelected(int selected);

protected:
	void layout();

private:
	static void sizeAllocate(GtkWidget* widget, GtkRequisition* requisition, BackgroundSelectDialogBase* dlg);
	static gboolean drawBackgroundCallback(GtkWidget* widget, cairo_t* cr, GtkWidget* layoutContainer);

private:
	XOJ_TYPE_ATTRIB;

protected:
	Settings* settings;
	GtkWidget* scrollPreview;
	GtkWidget* layoutContainer;

	Document* doc;

	/**
	 * Selected image, none if negative
	 */
	int selected;

	/**
	 * To check if the size has real changed
	 */
	int lastWidth;

	/**
	 * Elements to display
	 */
	vector<BaseElementView*> elements;
};
