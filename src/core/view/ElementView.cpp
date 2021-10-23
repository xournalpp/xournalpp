#include "model/Element.h"
#include "model/Image.h"
#include "model/Stroke.h"
#include "model/TexImage.h"
#include "model/Text.h"

#include "ImageView.h"
#include "StrokeView.h"
#include "TexImageView.h"
#include "TextView.h"
#include "View.h"

using namespace xoj::view;

auto ElementView::createFromElement(const Element* e) -> std::unique_ptr<ElementView> {
    switch (e->getType()) {
        case ELEMENT_STROKE:
            return std::make_unique<StrokeView>(dynamic_cast<const Stroke*>(e));
        case ELEMENT_TEXT:
            return std::make_unique<TextView>(dynamic_cast<const Text*>(e));
        case ELEMENT_IMAGE:
            return std::make_unique<ImageView>(dynamic_cast<const Image*>(e));
        case ELEMENT_TEXIMAGE:
            return std::make_unique<TexImageView>(dynamic_cast<const TexImage*>(e));
        default:
            assert(false && "ElementView::getFromElement: Unknown element type!");
            return nullptr;
    }
}
