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
	static bool sliderButtonPress(GtkRange* range, GdkEvent *event, ToolZoomSlider* self);
	static bool sliderButtonRelease(GtkRange* range, GdkEvent *event, ToolZoomSlider* self);
	static bool sliderHoverScroll(GtkWidget* range,  GdkEventScroll* event, ToolZoomSlider* self);
	static gchar* sliderFormatValue(GtkRange *range, gdouble value, ToolZoomSlider* self);

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
	/**
	 * The slider is currently changing by user, do not update value
	 */
	bool sliderChangingByZoomControlOrInit = false;
	bool sliderChangingBySliderDrag = false;
	bool sliderChangingBySliderHoverScroll = false;
	gint64 sliderHoverScrollLastTime = 0;

	GtkWidget* slider = nullptr;
	ZoomControl* zoom = nullptr;
	bool horizontal = true;
};

