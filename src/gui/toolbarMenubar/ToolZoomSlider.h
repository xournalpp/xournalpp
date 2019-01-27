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

#define SCALE_LOG_OFFSET 0.20753

class ZoomControl;

class ToolZoomSlider : public AbstractToolItem, public ZoomListener
{
public:
	ToolZoomSlider(ActionHandler* handler, string id, ActionType type, ZoomControl* zoom);
	virtual ~ToolZoomSlider();

public:
	static void sliderChanged(GtkRange* range, ToolZoomSlider* self);
	static bool sliderFocusIn(GtkRange* range, GdkEvent *event, ToolZoomSlider* self);
	static bool sliderFocusOut(GtkRange* range, GdkEvent *event, ToolZoomSlider* self);
	static gchar* sliderFormatValue(GtkRange *range, gdouble value);

	virtual void zoomChanged();
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
	static double scaleFunc(double x);
	static double scaleFuncInv(double x);

private:
	XOJ_TYPE_ATTRIB;

	bool ignoreChange;
	int counter;

	/**
	 * The slider is currently changing by user, do not update value
	 */
	bool sliderChangingByUser;
	GtkWidget* slider;
	ZoomControl* zoom;
	bool horizontal;
};

