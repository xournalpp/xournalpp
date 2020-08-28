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

#include <string>
#include <vector>

#include "BackgroundImage.h"
#include "Layer.h"
#include "PageHandler.h"
#include "PageType.h"
#include "Util.h"
#include "XournalType.h"


class XojPage: public PageHandler {
public:
    XojPage(double width, double height);
    ~XojPage() override;
    XojPage(const XojPage& page);
    void operator=(const XojPage& p) = delete;

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

    void setBackgroundType(const PageType& bgType);
    PageType getBackgroundType();

    /**
     * Do not call this, cal doc->setPageSize(Page * p, double width, double height);
     */
    void setSize(double width, double height);

    double getWidth() const;
    double getHeight() const;

    size_t getPdfPageNr() const;

    bool isAnnotated();

    void setBackgroundColor(Color color);
    Color getBackgroundColor() const;

    vector<Layer*>* getLayers();
    size_t getLayerCount();
    int getSelectedLayerId();
    void setSelectedLayerId(int id);
    static bool isLayerVisible(Layer* layer);
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
     * The background color if the background type is plain
     */
    Color backgroundColor{0xffffffU};

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
