#include "AbstractSliderItem.h"

#include <utility>  // for move

AbstractSliderItem::AbstractSliderItem(std::string id, SliderRange range, ActionRef gAction):
        AbstractToolItem(std::move(id)), range(range), gAction(std::move(gAction)) {}
