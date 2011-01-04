#include "Layer.h"

Layer::Layer() {
	this->elements = NULL;
}

Layer::~Layer() {
	for (GList * l = elements; l != NULL; l = l->next) {
		delete (Element *) l->data;
	}
	g_list_free(this->elements);
	this->elements = NULL;
}

void Layer::addElement(Element * e) {
	if (e == NULL) {
		g_warning("addElement(NULL)!");
		return;
	}

	this->elements = g_list_append(this->elements, e);
}

void Layer::insertElement(Element * e, int pos) {
	if (e == NULL) {
		g_warning("insertElement(NULL)!");
		return;
	}

	this->elements = g_list_insert(this->elements, e, pos);
}

int Layer::indexOf(Element * e) {
	GList * elem = g_list_find(this->elements, e);

	if (elem == NULL) {
		return -1;
	}

	return g_list_position(this->elements, elem);
}

int Layer::removeElement(Element * e, bool free) {
	GList * elem = g_list_find(this->elements, e);

	if (elem == NULL) {
		g_warning("could not remove element from layer, its not on the layer!");
		return -1;
	}

	int pos = g_list_position(this->elements, elem);
	this->elements = g_list_remove_link(this->elements, elem);

	if (free) {
		delete e;
	}
	return pos;
}

bool Layer::isAnnotated() {
	return this->elements != NULL;
}

ListIterator<Element *> Layer::elementIterator() {
	return ListIterator<Element *> (this->elements);
}

