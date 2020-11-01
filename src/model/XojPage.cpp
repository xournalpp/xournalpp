#include "XojPage.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "BackgroundImage.h"
#include "Document.h"

XojPage::XojPage(double width, double height): width(width), height(height), bgType(PageTypeFormat::Lined) {}

XojPage::~XojPage() {
    for (Layer* l: this->layer) {
        delete l;
    }
    this->layer.clear();
}

XojPage::XojPage(XojPage const& page):
        width(page.width),
        height(page.height),
        backgroundImage(page.backgroundImage),
        currentLayer(page.currentLayer),
        bgType(page.bgType),
        pdfBackgroundPage(page.pdfBackgroundPage),
        backgroundColor(page.backgroundColor) {
    this->layer.reserve(page.layer.size());
    std::transform(begin(page.layer), end(page.layer), std::back_inserter(this->layer),
                   [](auto* layer) { return layer->clone(); });
}

auto XojPage::clone() -> XojPage* { return new XojPage(*this); }

void XojPage::addLayer(Layer* layer) {
    this->layer.push_back(layer);
    this->currentLayer = npos;
}

void XojPage::insertLayer(Layer* layer, int index) {
    if (index >= static_cast<int>(this->layer.size())) {
        addLayer(layer);
        return;
    }

    this->layer.insert(this->layer.begin() + index, layer);
    this->currentLayer = index + 1;
}

void XojPage::removeLayer(Layer* layer) {
    for (unsigned int i = 0; i < this->layer.size(); i++) {
        if (layer == this->layer[i]) {
            this->layer.erase(this->layer.begin() + i);
            break;
        }
    }
    this->currentLayer = npos;
}

void XojPage::setSelectedLayerId(int id) { this->currentLayer = id; }

auto XojPage::getLayers() -> vector<Layer*>* { return &this->layer; }

auto XojPage::getLayerCount() -> size_t { return this->layer.size(); }

/**
 * Layer ID 0 = Background, Layer ID 1 = Layer 1
 */
auto XojPage::getSelectedLayerId() -> int {
    if (this->currentLayer == npos) {
        this->currentLayer = this->layer.size();
    }

    return this->currentLayer;
}

void XojPage::setLayerVisible(int layerId, bool visible) {
    if (layerId < 0) {
        return;
    }

    if (layerId == 0) {
        backgroundVisible = visible;
        return;
    }

    layerId--;
    if (layerId >= static_cast<int>(this->layer.size())) {
        return;
    }

    this->layer[layerId]->setVisible(visible);
}

auto XojPage::isLayerVisible(int layerId) -> bool {
    if (layerId < 0) {
        return false;
    }

    if (layerId == 0) {
        return backgroundVisible;
    }

    layerId--;
    if (layerId >= static_cast<int>(this->layer.size())) {
        return false;
    }

    return this->layer[layerId]->isVisible();
}

auto XojPage::isLayerVisible(Layer* layer) -> bool { return layer->isVisible(); }

void XojPage::setBackgroundPdfPageNr(size_t page) {
    this->pdfBackgroundPage = page;
    this->bgType.format = PageTypeFormat::Pdf;
    this->bgType.config = "";
}

void XojPage::setBackgroundColor(Color color) { this->backgroundColor = color; }

auto XojPage::getBackgroundColor() const -> Color { return this->backgroundColor; }

void XojPage::setSize(double width, double height) {
    this->width = width;
    this->height = height;
}

auto XojPage::getWidth() const -> double { return this->width; }

auto XojPage::getHeight() const -> double { return this->height; }

auto XojPage::getPdfPageNr() const -> size_t { return this->pdfBackgroundPage; }

auto XojPage::isAnnotated() -> bool {
    for (Layer* l: this->layer) {
        if (l->isAnnotated()) {
            return true;
        }
    }
    return false;
}

void XojPage::setBackgroundType(const PageType& bgType) {
    this->bgType = bgType;

    if (!bgType.isPdfPage()) {
        this->pdfBackgroundPage = npos;
    }
    if (!bgType.isImagePage()) {
        this->backgroundImage.free();
    }
}

auto XojPage::getBackgroundType() -> PageType { return this->bgType; }

auto XojPage::getBackgroundImage() -> BackgroundImage& { return this->backgroundImage; }

void XojPage::setBackgroundImage(BackgroundImage img) { this->backgroundImage = std::move(img); }

auto XojPage::getSelectedLayer() -> Layer* {
    if (this->layer.empty()) {
        addLayer(new Layer());
    }
    size_t layer = getSelectedLayerId();

    if (layer > 0) {
        layer--;
    }

    return this->layer[layer];
}
