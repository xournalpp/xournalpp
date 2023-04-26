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

#include <cstddef>   // for size_t
#include <optional>  // for optional
#include <string>    // for string
#include <vector>    // for vector

#include "util/Color.h"  // for Color
#include "util/Util.h"   // for npos

#include "BackgroundImage.h"  // for BackgroundImage
#include "Layer.h"            // for Layer, Layer::Index
#include "PageHandler.h"      // for PageHandler
#include "PageType.h"         // for PageType

class Document;

class XojPage: public PageHandler {
public:
    XojPage(double width, double height, bool ghost = false, bool suppressLayerCreation = false);
    ~XojPage() override;
    XojPage(const XojPage& page);
    void operator=(const XojPage& p) = delete;

    // Do not modify layers directly, use LayerController
    // So notification can be sent on change
protected:
    void addLayer(Layer* layer);
    void insertLayer(Layer* layer, Layer::Index index);
    void removeLayer(Layer* layer);
    void setLayerVisible(Layer::Index layerId, bool visible);

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

    bool isAnnotated() const;

    void setBackgroundColor(Color color);
    Color getBackgroundColor() const;

    std::vector<Layer*>* getLayers();
    Layer::Index getLayerCount() const;
    Layer::Index getSelectedLayerId();
    void setSelectedLayerId(Layer::Index id);
    bool isLayerVisible(Layer::Index layerId) const;

    Layer* getSelectedLayer();

    BackgroundImage& getBackgroundImage();
    void setBackgroundImage(BackgroundImage img);

    std::string getBackgroundName() const;
    bool backgroundHasName() const;
    void setBackgroundName(const std::string& newName);

    /**
     * Copies this page an all it's contents to a new page
     */
    XojPage* clone();

    /**
     * @brief Returns true if the page is a ghost page (i.e. will not be saved)
     * A page with elements on it should never be a ghost page
     */
    inline bool isGhost() const { return ghostPage; }

    /**
     * @brief Unghosts the page and returns true if the page was indeed a ghost page before
     */
    bool unghost();
    /**
     * @brief Lock the document and add the element to the page's active layer. Also Unghosts the page and calls
     * firePageUnghosted() if needed
     */
    void safeAddElementToActiveLayer(Document* doc, Element* e);

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
    std::vector<Layer*> layer;

    /**
     * The current selected layer ID
     */
    Layer::Index currentLayer = npos;

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
    Color backgroundColor{Colors::white};

    /**
     * Background visible
     */
    bool backgroundVisible = true;

    /**
     * @brief If true, the page is greyed out and not saved, until an element is added to it
     */
    bool ghostPage = false;

    /**
     * Background name
     */
    std::optional<std::string> backgroundName;

    // Allow LoadHandler to add layers directly
    friend class LoadHandler;

    // Allow LayerController to modify layers of a page
    // Notifications were be sent
    friend class LayerController;
};
