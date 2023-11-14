#include "XojPage.h"

#include <algorithm>  // for find, transform
#include <iterator>   // for back_insert_iterator, back_inserter, begin
#include <utility>    // for move

#include "model/Layer.h"     // for Layer, Layer::Index
#include "model/PageType.h"  // for PageType, PageTypeFormat, PageTypeForma...
#include "util/Assert.h"     // for xoj_assert
#include "util/i18n.h"       // for _

#include "BackgroundImage.h"  // for BackgroundImage

XojPage::XojPage(double width, double height, bool suppressLayerCreation): width(width), height(height), bgType(PageTypeFormat::Lined) {
    if (!suppressLayerCreation) {
        // ensure at least one valid layer exists
        this->addLayer(new Layer());
        this->currentLayer = 1;
    }
}

XojPage::~XojPage() {
    for (Layer* l: this->layer) { delete l; }
    this->layer.clear();
}

XojPage::XojPage(XojPage const& page):
        backgroundImage(page.backgroundImage),
        width(page.width),
        height(page.height),
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

void XojPage::insertLayer(Layer* layer, Layer::Index index) {
    if (index >= this->layer.size()) {
        addLayer(layer);
        return;
    }

    this->layer.insert(std::next(this->layer.begin(), static_cast<ptrdiff_t>(index)), layer);
    this->currentLayer = index + 1;
}

void XojPage::removeLayer(Layer* l) {
    if (auto it = std::find(layer.begin(), layer.end(), l); it != layer.end()) {
        this->layer.erase(it);
    }
    this->currentLayer = npos;
    // ensure at least one valid layer exists
    if (layer.empty()) {
        addLayer(new Layer());
    }
}

void XojPage::setSelectedLayerId(Layer::Index id) { this->currentLayer = id; }

auto XojPage::getLayers() -> std::vector<Layer*>& { return this->layer; }

auto XojPage::getLayers() const -> PointerContainerView<std::vector<Layer*>> { return this->layer; }

auto XojPage::getLayerCount() const -> Layer::Index { return this->layer.size(); }

/**
 * Layer ID 0 = Background, Layer ID 1 = Layer 1
 */
auto XojPage::getSelectedLayerId() -> Layer::Index {
    if (this->currentLayer == npos) {
        this->currentLayer = this->layer.size();
    }

    return this->currentLayer;
}

void XojPage::setLayerVisible(Layer::Index layerId, bool visible) {
    if (layerId == 0) {
        backgroundVisible = visible;
        return;
    }

    layerId--;
    if (layerId >= this->layer.size()) {
        return;
    }

    this->layer[layerId]->setVisible(visible);
}

auto XojPage::isLayerVisible(Layer::Index layerId) const -> bool {
    if (layerId == 0) {
        return backgroundVisible;
    }

    layerId--;
    if (layerId >= this->layer.size()) {
        return false;
    }

    return this->layer[layerId]->isVisible();
}

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

auto XojPage::isAnnotated() const -> bool {
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

auto XojPage::getBackgroundType() const -> PageType { return this->bgType; }

auto XojPage::getBackgroundImage() -> BackgroundImage& { return this->backgroundImage; }
auto XojPage::getBackgroundImage() const -> const BackgroundImage& { return this->backgroundImage; }

void XojPage::setBackgroundImage(BackgroundImage img) { this->backgroundImage = std::move(img); }

auto XojPage::getSelectedLayer() -> Layer* {
    xoj_assert(!layer.empty());
    size_t layer = getSelectedLayerId();

    if (layer > 0) {
        layer--;
    }

    return this->layer[layer];
}

auto XojPage::getBackgroundName() const -> std::string { return backgroundName.value_or(_("Background")); }

auto XojPage::backgroundHasName() const -> bool { return backgroundName.has_value(); }

void XojPage::setBackgroundName(const std::string& newName) { backgroundName = newName; }
