#include "ToolbarItem.h"

int ToolbarItem::sid = 0;

ToolbarItem::ToolbarItem(String name) {
	this->name = name;
	this->id = ToolbarItem::sid++;

	if(ToolbarItem::sid < 0) {
		ToolbarItem::sid = 0;
	}
}

ToolbarItem::~ToolbarItem() {
}

ToolbarItem::operator String() {
	return this->name;
}

bool ToolbarItem::operator ==(ToolbarItem & other) {
	return this->name == other.name;
}

int ToolbarItem::getId() {
	return this->id;
}

