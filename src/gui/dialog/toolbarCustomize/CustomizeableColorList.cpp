#include "CustomizeableColorList.h"

#include <utility>

#include <config.h>

#include "i18n.h"

CustomizeableColorList::CustomizeableColorList() {
    this->addPredefinedColor(0x000000U, _("Black"));
    this->addPredefinedColor(0x008000U, _("Green"));
    this->addPredefinedColor(0x00c0ffU, _("Light Blue"));
    this->addPredefinedColor(0x00ff00U, _("Light Green"));
    this->addPredefinedColor(0x3333ccU, _("Blue"));
    this->addPredefinedColor(0x808080U, _("Gray"));
    this->addPredefinedColor(0xff0000U, _("Red"));
    this->addPredefinedColor(0xff00ffU, _("Magenta"));
    this->addPredefinedColor(0xff8000U, _("Orange"));
    this->addPredefinedColor(0xffff00U, _("Yellow"));
    this->addPredefinedColor(0xffffffU, _("White"));
}

CustomizeableColorList::~CustomizeableColorList() {
    for (XojColor* c: this->colors) {
        delete c;
    }
    this->colors.clear();
}

auto CustomizeableColorList::getPredefinedColors() -> vector<XojColor*>* { return &this->colors; }

void CustomizeableColorList::addPredefinedColor(Color color, string name) {
    this->colors.push_back(new XojColor(color, std::move(name)));
}
