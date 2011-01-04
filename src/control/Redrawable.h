/*
 * Xournal++
 *
 * Interface for GUI handling
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __REDRAWABLE_H__
#define __REDRAWABLE_H__

class Redrawable {
public:
	virtual void redrawDocumentRegion(double x1, double y1, double x2, double y2) = 0;
	virtual GdkColor getSelectionColor() = 0;
	virtual void deleteViewBuffer() = 0;
	virtual GtkWidget * getWidget() = 0;
};

#endif /* __REDRAWABLE_H__ */
