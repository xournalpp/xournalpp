/*
 * Xournal++
 *
 * A page (PDF or drawings or both)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BackgroundImage.h"
#include "BackgroundType.h"
#include "Layer.h"
#include "PageHandler.h"

#include <StringUtils.h>
#include <XournalType.h>

class XojPage : public PageHandler
{
public:
	XojPage(double width, double height);
	void reference();
	void unreference();

private:
	XojPage(const XojPage& page);
	virtual ~XojPage();
	void operator=(const XojPage& p);

public:
	// Also set the size over doc->setPageSize!
	void setBackgroundPdfPageNr(size_t page);

	void setBackgroundType(BackgroundType bgType);
	BackgroundType getBackgroundType();

	/**
	 * Do not call this, cal doc->setPageSize(Page * p, double width, double height);
	 */
	void setSize(double width, double height);

	double getWidth() const;
	double getHeight() const;

	void addLayer(Layer* layer);
	void insertLayer(Layer* layer, int index);
	void removeLayer(Layer* layer);

	size_t getPdfPageNr();

	bool isAnnotated();

	void setBackgroundColor(int color);
	int getBackgroundColor();

	LayerVector* getLayers();
	size_t getLayerCount();
	int getSelectedLayerId();
	void setSelectedLayerId(int id);

	Layer* getSelectedLayer();

	BackgroundImage& getBackgroundImage();
	void setBackgroundImage(BackgroundImage img);

	/**
	 * Copies this page an all it's contents to a new page
	 */
	XojPage* clone();

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
	LayerVector layer;

	/**
	 * The current selected layer ID
	 */
	size_t currentLayer;

	/**
	 * The Background Type of the page
	 */
	BackgroundType bgType;

	/**
	 * If the page has a PDF background, the page number of the PDF Page
	 */
	size_t pdfBackgroundPage;

	/**
	 * The background color if the background type is palain
	 */
	int backgroundColor;
};
