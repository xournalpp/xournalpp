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

protected:
	void layout();

private:
	XOJ_TYPE_ATTRIB;

protected:
	Settings* settings;
	GtkWidget* scrollPreview;
	GtkWidget* widget;

	vector<BaseElementView*> elements;
};
