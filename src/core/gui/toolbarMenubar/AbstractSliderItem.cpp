#include "AbstractSliderItem.h"

#include <utility>  // for move

AbstractSliderItem::AbstractSliderItem(std::string id, Category cat, SliderRange range, ActionRef gAction):
        AbstractToolItem(std::move(id), cat), range(range), gAction(std::move(gAction)) {}
