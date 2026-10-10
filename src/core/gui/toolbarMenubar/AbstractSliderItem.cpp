#include "AbstractSliderItem.h"

#include <utility>  // for move

AbstractSliderItem::AbstractSliderItem(std::string id, Category cat, SliderRange range, ActionRef gAction,
                                       std::string iconName):
        ItemWithNamedIcon(std::move(id), cat),
        range(range),
        gAction(std::move(gAction)),
        iconName(std::move(iconName)) {}

const char* AbstractSliderItem::getIconName() const { return iconName.c_str(); }
