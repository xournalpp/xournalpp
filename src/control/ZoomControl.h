/*
 * Xournal++
 *
 * Controls the zoom level
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __ZOOMCONTROL_H__
#define __ZOOMCONTROL_H__

#include <gtk/gtk.h>

class ZoomListener {
public:
	virtual void zoomChanged(double lastZoom) = 0;
	virtual void zoomRangeValuesChanged();
};

class ZoomControl {
public:
	ZoomControl();
	virtual ~ZoomControl();

	void zoomIn();
	void zoomOut();

	void zoomFit();
	void zoom100();

	double getZoom();
	void setZoom(double zoom);

	void setZoom100(double zoom);
	void setZoomFit(double zoom);

	double getZoomFit();
	double getZoom100();

	void addZoomListener(ZoomListener * listener);

	void initZoomHandler(GtkWidget * widget);
protected:
	void fireZoomChanged(double lastZoom);
	void fireZoomRangeValueChanged();

	static bool onScrolledwindowMainScrollEvent(GtkWidget * widget, GdkEventScroll * event, ZoomControl * zoom);
private:
	GList * listener;

	double zoom;

	double zoom100Value;
	double zoomFitValue;
};

#endif /* __ZOOMCONTROL_H__ */
