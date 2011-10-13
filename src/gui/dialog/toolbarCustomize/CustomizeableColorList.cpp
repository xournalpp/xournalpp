#include "CustomizeableColorList.h"

const int CustomizeableColorList::PREDEFINED_COLORS[16] = {
		0xffffff,
		0xffff00,
		0xff8000,
		0xff00ff,
		0x00ff00,
		0x00c0ff,
		0x808080,
		0x008000,
		0xff0000,
		0x3333cc,
		0x000000
};


CustomizeableColorList::CustomizeableColorList() {
	XOJ_INIT_TYPE(CustomizeableColorList);

	this->customColors = NULL;
}

CustomizeableColorList::~CustomizeableColorList() {
	XOJ_RELEASE_TYPE(CustomizeableColorList);
}
