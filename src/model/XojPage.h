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

#ifndef __XOJ_PAGE_H__
#define __XOJ_PAGE_H__

#include "Layer.h"
#include <ListIterator.h>
#include <XournalType.h>
#include <String.h>

#include "BackgroundType.h"
#include "BackgroundImage.h"


class XojPage {
public:
	XojPage(double width, double heigth);
	void reference();
	void unreference();

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

	BackgroundImage & getBackgroundImage();
	void setBackgroundImage(BackgroundImage img);

	/**
	 * Copies this page an all it's contents to a new page
	 */
	XojPage * clone();

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * The reference counter
	 */
	int ref;

	/**
	 * The Background image if any
	 */
	BackgroundImage backgroundImage;

	/**
	 * The size of the page
	 */
	double width;
	double height;

	/**
	 * The layer list
	 */
	GList * layer;

	/**
	 * The current selected layer ID
	 */
	int currentLayer;

	/**
	 * The Background Type of the page
	 */
	BackgroundType bgType;

	/**
	 * If the page has a PDF background, the page number of the PDF Page
	 */
	int pdfBackgroundPage;

	/**
	 * The background color if the background type is palain
	 */
	int backgroundColor;
};

#endif /* __XOJ_PAGE_H__ */
