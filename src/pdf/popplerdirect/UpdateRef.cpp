#include "UpdateRef.h"

UpdateRef::UpdateRef(int objectId, XojPopplerDocument doc) {
	XOJ_INIT_TYPE(UpdateRef);

	this->objectId = objectId;
	this->wroteOut = false;
	this->doc = doc;
}

UpdateRef::~UpdateRef() {
	XOJ_RELEASE_TYPE(UpdateRef);
}

void UpdateRef::destroyDelete(UpdateRef * data) {
	delete data;
}
