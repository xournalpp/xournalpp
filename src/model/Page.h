/*
 * Xournal++
 *
 * A page (PDF or drawings or both)
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PAGE_H__
#define __PAGE_H__

#include "Layer.h"
#include "../util/ListIterator.h"
#include "../util/XournalType.h"
#include "../util/String.h"

#include "BackgroundImage.h"

enum BackgroundType {
	BACKGROUND_TYPE_NONE = 1,
	BACKGROUND_TYPE_PDF,
	BACKGROUND_TYPE_IMAGE,
	BACKGROUND_TYPE_LINED,
	BACKGROUND_TYPE_RULED,
	BACKGROUND_TYPE_GRAPH
};

class XojPage {
public:
	XojPage(double width, double heigth, int x);
	void reference(int debugId);
	void unreference(int debugId);

private:
	XojPage(const XojPage & page);
	virtual ~XojPage();
	void operator=(const XojPage & p);

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

public:
	/**
	 * Background image, if background type is IMAGE, public because the oprator= is overloaded
	 */
	BackgroundImage backgroundImage;

private:
	XOJ_TYPE_ATTRIB;

	int ref;

	double width;
	double height;

	GList * layer;

	int currentLayer;

	int pdfBackgroundPage;
	BackgroundType bgType;
	int backgroundColor;
};

#endif /* __PAGE_H__ */
