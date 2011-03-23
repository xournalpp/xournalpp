#include "UpdateRefKey.h"
// TODO: AA: type check

guint UpdateRefKey::hashFunction(UpdateRefKey * key) {
	return key->ref.num + key->ref.gen * 13 + key->doc.getId() * 23;
}

bool UpdateRefKey::equalFunction(UpdateRefKey * a, UpdateRefKey * b) {
	return a->ref.gen == b->ref.gen && a->ref.num == b->ref.num && a->doc == b->doc;
}

void UpdateRefKey::destroyDelete(UpdateRefKey * data) {
	delete data;
}

