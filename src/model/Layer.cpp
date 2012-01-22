#include "Layer.h"
#include <Stacktrace.h>

#include "LayerListener.h"

Layer::Layer() {
	XOJ_INIT_TYPE(Layer);

	this->elements = NULL;
	this->listeners = NULL;
}

Layer::~Layer() {
	XOJ_CHECK_TYPE(Layer);

	for(GList * l = this->listeners; l != NULL; l = l->next) {
		LayerListener * listener = (LayerListener *)l->data;
		listener->layerDeleted();
	}

	g_list_free(this->listeners);
	this->listeners = NULL;

	for (GList * l = elements; l != NULL; l = l->next) {
		delete (Element *) l->data;
	}
	g_list_free(this->elements);
	this->elements = NULL;

	XOJ_RELEASE_TYPE(Layer);
}

Layer * Layer::clone() {
	XOJ_CHECK_TYPE(Layer);

	Layer * layer = new Layer();

	for (GList * l = elements; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		layer->addElement(e->clone());
	}

	return layer;
}

void Layer::addElement(Element * e) {
	XOJ_CHECK_TYPE(Layer);

	if (e == NULL) {
		g_warning("addElement(NULL)!");
		Stacktrace::printStracktrace();
		return;
	}

	GList * elem2 = g_list_find(this->elements, e);
	if (elem2) {
		printf("Layer::addElement: Element is already on this layer!\n");
		return;
	}

	this->elements = g_list_append(this->elements, e);

	for(GList * l = this->listeners; l != NULL; l = l->next) {
		LayerListener * listener = (LayerListener *)l->data;
		listener->elementAdded(e, this);
	}
}

void Layer::insertElement(Element * e, int pos) {
	XOJ_CHECK_TYPE(Layer);

	if (e == NULL) {
		g_warning("insertElement(NULL)!");
		Stacktrace::printStracktrace();
		return;
	}

	GList * elem2 = g_list_find(this->elements, e);
	if (elem2) {
		g_warning("Layer::insertElement() try to add an element twice!");
		Stacktrace::printStracktrace();
		return;
	}

	this->elements = g_list_insert(this->elements, e, pos);

	for(GList * l = this->listeners; l != NULL; l = l->next) {
		LayerListener * listener = (LayerListener *)l->data;
		listener->elementAdded(e, this);
	}
}

int Layer::indexOf(Element * e) {
	XOJ_CHECK_TYPE(Layer);

	GList * elem = g_list_find(this->elements, e);

	if (elem == NULL) {
		return -1;
	}

	return g_list_position(this->elements, elem);
}

int Layer::removeElement(Element * e, bool free) {
	XOJ_CHECK_TYPE(Layer);

	GList * elem = g_list_find(this->elements, e);

	if (elem == NULL) {
		g_warning("could not remove element from layer, its not on the layer!");
		Stacktrace::printStracktrace();
		return -1;
	}

	int pos = g_list_position(this->elements, elem);
	this->elements = g_list_delete_link(this->elements, elem);

	for(GList * l = this->listeners; l != NULL; l = l->next) {
		LayerListener * listener = (LayerListener *)l->data;
		listener->elementRemoved(e, this);
	}

	if (free) {
		delete e;
	}
	return pos;
}

bool Layer::isAnnotated() {
	XOJ_CHECK_TYPE(Layer);

	return this->elements != NULL;
}

ListIterator<Element *> Layer::elementIterator() {
	XOJ_CHECK_TYPE(Layer);

	return ListIterator<Element *> (this->elements);
}

void Layer::addListener(LayerListener * listener) {
	this->listeners = g_list_append(this->listeners, listener);
}

void Layer::removeListener(LayerListener * listener) {
	this->listeners = g_list_remove(this->listeners, listener);
}


