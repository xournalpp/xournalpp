#include "Layer.h"

#include <Stacktrace.h>

#include <iostream>
using std::cout;
using std::endl;

Layer::Layer()
{
	XOJ_INIT_TYPE(Layer);
}

Layer::~Layer()
{
	XOJ_CHECK_TYPE(Layer);

	for (Element* e : this->elements)
	{
		delete e;
	}

	XOJ_RELEASE_TYPE(Layer);
}

Layer* Layer::clone()
{
	XOJ_CHECK_TYPE(Layer);

	Layer* layer = new Layer();

	for (Element* e : this->elements)
	{
		layer->addElement(e->clone());
	}

	return layer;
}

void Layer::addElement(Element* e)
{
	XOJ_CHECK_TYPE(Layer);

	if (e == NULL)
	{
		g_warning("addElement(NULL)!");
		Stacktrace::printStracktrace();
		return;
	}

	for (Element* elem2 : this->elements)
	{
		if (e == elem2)
		{
			cout << "Layer::addElement: Element is already on this layer!" << endl;
			return;
		}
	}

	this->elements.push_back(e);
}

void Layer::insertElement(Element* e, int pos)
{
	XOJ_CHECK_TYPE(Layer);

	if (e == NULL)
	{
		g_warning("insertElement(NULL)!");
		Stacktrace::printStracktrace();
		return;
	}

	for (Element* elem2 : this->elements)
	{
		if (e == elem2)
		{
			g_warning("Layer::insertElement() try to add an element twice!");
			Stacktrace::printStracktrace();
			return;
		}
	}

	this->elements.insert(this->elements.begin() + pos, e);
}

int Layer::indexOf(Element* e)
{
	XOJ_CHECK_TYPE(Layer);
	
	for (unsigned int i = 0; i < this->elements.size(); i++)
	{
		if (this->elements[i] == e)
		{
			return i;
		}
	}
	
	return -1;
}

int Layer::removeElement(Element* e, bool free)
{
	XOJ_CHECK_TYPE(Layer);

	for (unsigned int i = 0; i < this->elements.size(); i++)
	{
		if (e == this->elements[i])
		{
			this->elements.erase(this->elements.begin() + i);
			
			if (free)
			{
				delete e;
			}
			return i;
		}
	}
	
	g_warning("Could not remove element from layer, it's not on the layer!");
	Stacktrace::printStracktrace();
	return -1;
}

bool Layer::isAnnotated()
{
	XOJ_CHECK_TYPE(Layer);

	return !this->elements.empty();
}

ElementVector* Layer::getElements()
{
	XOJ_CHECK_TYPE(Layer);

	return &this->elements;
}
