/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "AbstractToolItem.h"
#include "control/zoom/ZoomListener.h"

#include <XournalType.h>

class ZoomControl;

class ToolZoomSlider : public AbstractToolItem, public ZoomListener
{
public:
	ToolZoomSlider(ActionHandler* handler, string id, ActionType type, ZoomControl* zoom);
	virtual ~ToolZoomSlider();

public:
	static void sliderChanged(GtkRange* range, ZoomControl* zoom);
	virtual void zoomChanged(double lastZoom);
	virtual void zoomRangeValuesChanged();
	virtual string getToolDisplayName();

	// Should be called when the window size changes
	void updateScaleMarks();
	virtual GtkToolItem* createItem(bool horizontal);
	virtual GtkToolItem* createTmpItem(bool horizontal);

protected:
	virtual void enable(bool enabled);
	virtual GtkToolItem* newItem();
	virtual GtkWidget* getNewToolIcon();

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget* slider;
	ZoomControl* zoom;
	bool horizontal;
};

