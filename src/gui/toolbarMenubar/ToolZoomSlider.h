/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLZOOMSLIDER_H__
#define __TOOLZOOMSLIDER_H__

#include "AbstractToolItem.h"
#include "../../control/ZoomControl.h"

class ToolZoomSlider: public AbstractToolItem, public ZoomListener {
public:
	ToolZoomSlider(ActionHandler * handler, String id, ActionType type, ZoomControl * zoom);
	virtual ~ToolZoomSlider();

public:
	static void sliderChanged(GtkRange *range, ZoomControl * zoom);
	virtual void zoomChanged(double lastZoom);
	virtual void zoomRangeValuesChanged();

	// Should be called when the window size changes
	void updateScaleMarks();
	virtual void setHorizontal(bool horizontal);
	virtual GtkToolItem * createItem(bool horizontal);

protected:
	virtual void enable(bool enabled);
	virtual GtkToolItem * newItem();

private:
	GtkWidget * slider;
	GtkWidget * fixed;
	ZoomControl * zoom;
	bool horizontal;
};

#endif /* __TOOLZOOMSLIDER_H__ */
