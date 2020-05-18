#include "MainBackgroundPainter.h"

#include "BackgroundConfig.h"
#include "BaseBackgroundPainter.h"
#include "DottedBackgroundPainter.h"
#include "GraphBackgroundPainter.h"
#include "IsometricBackgroundPainter.h"
#include "LineBackgroundPainter.h"
#include "StavesBackgroundPainter.h"

MainBackgroundPainter::MainBackgroundPainter() {
    defaultPainter = new BaseBackgroundPainter();

    painter[PageTypeFormat::Ruled] = new LineBackgroundPainter(false);
    painter[PageTypeFormat::Lined] = new LineBackgroundPainter(true);
    painter[PageTypeFormat::Staves] = new StavesBackgroundPainter();
    painter[PageTypeFormat::Graph] = new GraphBackgroundPainter();
    painter[PageTypeFormat::Dotted] = new DottedBackgroundPainter();
    painter[PageTypeFormat::IsoDotted] = new IsometricBackgroundPainter(false);
    painter[PageTypeFormat::IsoGraph] = new IsometricBackgroundPainter(true);
}

MainBackgroundPainter::~MainBackgroundPainter() {
    for (auto& e: painter) {
        delete e.second;
    }
    painter.clear();

    delete defaultPainter;
    defaultPainter = nullptr;
}

/**
 * Set a factor to draw the lines bolder, for previews
 */
void MainBackgroundPainter::setLineWidthFactor(double factor) {
    for (auto& e: painter) {
        e.second->setLineWidthFactor(factor);
    }
}

void MainBackgroundPainter::paint(PageType pt, cairo_t* cr, PageRef page) {
    auto it = this->painter.find(pt.format);

    BaseBackgroundPainter* painter = defaultPainter;
    if (it != this->painter.end()) {
        painter = it->second;
    }

    BackgroundConfig config(pt.config);

    painter->resetConfig();
    painter->paint(cr, page, &config);
}
