#include <cassert>  // for assert
#include <memory>   // for make_unique, unique_ptr

#include "model/Element.h"   // for Element, ELEMENT_IMAGE, ELEMENT_STROKE
#include "model/Image.h"     // for Image
#include "model/Stroke.h"    // for Stroke
#include "model/TexImage.h"  // for TexImage
#include "model/Text.h"      // for Text

#include "ImageView.h"     // for ImageView
#include "StrokeView.h"    // for StrokeView
#include "TexImageView.h"  // for TexImageView
#include "TextView.h"      // for TextView
#include "View.h"          // for ElementView, view

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
