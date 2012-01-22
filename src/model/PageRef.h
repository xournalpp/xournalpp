/*
 * Xournal++
 *
 * A page reference, should only allocated on the stack
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PAGEREF_H__
#define __PAGEREF_H__

#include "BackgroundType.h"
#include <ListIterator.h>
#include <XournalType.h>

class XojPage;
class Layer;
class BackgroundImage;

class PageRef {
public:
	PageRef();
	PageRef(const PageRef & ref);
	PageRef(XojPage * page);
	virtual ~PageRef();

public:
	bool isValid();

	operator XojPage *();

	bool operator==(const PageRef & ref);
	void operator=(const PageRef & ref);
	void operator=(XojPage * page);

	PageRef clone();

public:
	// Also set the size over doc->setPageSize!
	void setBackgroundPdfPageNr(int page);

	void setBackgroundType(BackgroundType bgType);
	BackgroundType getBackgroundType();

	/**
	 * Do not call this, cal doc->setPageSize(Page * p, double width, double height);
	 */
	void setSize(double width, double height);

	double getWidth();
	double getHeight();

	void addLayer(Layer * layer);
	void insertLayer(Layer * layer, int index);
	void removeLayer(Layer * layer);

	int getPdfPageNr();

	bool isAnnotated();

	void setBackgroundColor(int color);
	int getBackgroundColor();

	ListIterator<Layer*> layerIterator();
	int getLayerCount();
	int getSelectedLayerId();
	void setSelectedLayerId(int id);
	Layer * getSelectedLayer();

	BackgroundImage & getBackgroundImage();
	void setBackgroundImage(BackgroundImage & img);

private:
	XOJ_TYPE_ATTRIB;

	XojPage * page;
};

#endif /* __PAGEREF_H__ */
