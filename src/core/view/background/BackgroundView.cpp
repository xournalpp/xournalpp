#include "BackgroundView.h"

#include "model/BackgroundConfig.h"

#include "DottedBackgroundView.h"
#include "GraphBackgroundView.h"
#include "IsoDottedBackgroundView.h"
#include "IsoGraphBackgroundView.h"
#include "LinedBackgroundView.h"
#include "PlainBackgroundView.h"
#include "RuledBackgroundView.h"
#include "StavesBackgroundView.h"

using namespace xoj::view;

auto BackgroundView::create(double width, double height, Color backgroundColor, const PageType& pt,
                            double lineWidthFactor) -> std::unique_ptr<BackgroundView> {
    std::unique_ptr<OneColorBackgroundView> res;
    switch (pt.format) {
        case PageTypeFormat::Plain:
            return std::make_unique<PlainBackgroundView>(width, height, backgroundColor);
        case PageTypeFormat::Ruled:
            res = std::make_unique<RuledBackgroundView>(width, height, backgroundColor, pt.config);
            break;
        case PageTypeFormat::Lined:
            res = std::make_unique<LinedBackgroundView>(width, height, backgroundColor, pt.config);
            break;
        case PageTypeFormat::Graph:
            res = std::make_unique<GraphBackgroundView>(width, height, backgroundColor, pt.config);
            break;
        case PageTypeFormat::Staves:
            res = std::make_unique<StavesBackgroundView>(width, height, backgroundColor, pt.config);
            break;
        case PageTypeFormat::Dotted:
            res = std::make_unique<DottedBackgroundView>(width, height, backgroundColor, pt.config);
            break;
        case PageTypeFormat::IsoGraph:
            res = std::make_unique<IsoGraphBackgroundView>(width, height, backgroundColor, pt.config);
            break;
        case PageTypeFormat::IsoDotted:
            res = std::make_unique<IsoDottedBackgroundView>(width, height, backgroundColor, pt.config);
            break;
        default:
            g_warning("BackgroundView::createForPage unknowntype: %d", (int)pt.format);
            return nullptr;
    }
    res->multiplyLineWidth(lineWidthFactor);
    return res;
}
