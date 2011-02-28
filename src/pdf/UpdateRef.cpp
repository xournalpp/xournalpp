#include "UpdateRef.h"

UpdateRef::UpdateRef(int objectId, XojPopplerDocument doc) {
	this->objectId = objectId;
	this->wroteOut = false;
	this->doc = doc;
}

void UpdateRef::destroyDelete(UpdateRef * data) {
	delete data;
}
