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
#include "Layer.h"
#include "PageHandler.h"
#include "PageType.h"

#include <XournalType.h>
#include <Util.h>


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

	// Do not modify layers directly, use LayerController
	// So notification can be sent on change
protected:
	void addLayer(Layer* layer);
	void insertLayer(Layer* layer, int index);
	void removeLayer(Layer* layer);
	void setLayerVisible(int layerId, bool visible);

public:
	// Also set the size over doc->setPageSize!
	void setBackgroundPdfPageNr(size_t page);

	void setBackgroundType(PageType bgType);
	PageType getBackgroundType();

	/**
	 * Do not call this, cal doc->setPageSize(Page * p, double width, double height);
	 */
	void setSize(double width, double height);

	double getWidth() const;
	double getHeight() const;

	size_t getPdfPageNr();

	bool isAnnotated();

	void setBackgroundColor(int color);
	int getBackgroundColor();

	vector<Layer*>* getLayers();
	size_t getLayerCount();
	int getSelectedLayerId();
	void setSelectedLayerId(int id);
	bool isLayerVisible(Layer* layer);
	bool isLayerVisible(int layerId);

	Layer* getSelectedLayer();

	BackgroundImage& getBackgroundImage();
	void setBackgroundImage(BackgroundImage img);

	/**
	 * Copies this page an all it's contents to a new page
	 */
	XojPage* clone();

private:
	/**
	 * The reference counter
	 */
	int ref = 0;

	/**
	 * The Background image if any
	 */
	BackgroundImage backgroundImage;

	/**
	 * The size of the page
	 */
	double width = 0;
	double height = 0;

	/**
	 * The layer list
	 */
	vector<Layer*> layer;

	/**
	 * The current selected layer ID
	 */
	size_t currentLayer = npos;

	/**
	 * The Background Type of the page
	 */
	PageType bgType;

	/**
	 * If the page has a PDF background, the page number of the PDF Page
	 */
	size_t pdfBackgroundPage = npos;

	/**
	 * The background color if the background type is palain
	 */
	int backgroundColor = 0xffffff;

	/**
	 * Background visible
	 */
	bool backgroundVisible = true;

	// Allow LoadHandler to add layers directly
	friend class LoadHandler;

	// Allow LayerController to modify layers of a page
	// Notifications were be sent
	friend class LayerController;
};
