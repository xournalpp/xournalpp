#include "Layer.h"

#include <Stacktrace.h>

Layer::Layer()
{
}

Layer::~Layer()
{
	for (Element* e : this->elements)
	{
		delete e;
	}
	this->elements.clear();
}

Layer* Layer::clone()
{
	Layer* layer = new Layer();

	for (Element* e : this->elements)
	{
		layer->addElement(e->clone());
	}

	return layer;
}

void Layer::addElement(Element* e)
{
	if (e == nullptr)
	{
		g_warning("addElement(nullptr)!");
		Stacktrace::printStracktrace();
		return;
	}

	for (Element* elem2 : this->elements)
	{
		if (e == elem2)
		{
			g_warning("Layer::addElement: Element is already on this layer!");
			return;
		}
	}

	this->elements.push_back(e);
}

void Layer::insertElement(Element* e, int pos)
{
	if (e == nullptr)
	{
		g_warning("insertElement(nullptr)!");
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

	// prevent crash, even if this never should happen,
	// but there was a bug before which cause this error
	if (pos < 0)
	{
		pos = 0;
	}

	// If the element should be inserted at the top
	if (pos >= (int)this->elements.size())
	{
		this->elements.push_back(e);
	}
	else
	{
		this->elements.insert(this->elements.begin() + pos, e);
	}
}

int Layer::indexOf(Element* e)
{
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
	return !this->elements.empty();
}

/**
 * @return true if the layer is visible
 */
bool Layer::isVisible()
{
	return visible;
}

/**
 * @return true if the layer is visible
 */
void Layer::setVisible(bool visible)
{
	this->visible = visible;
}

vector<Element*>* Layer::getElements()
{
	return &this->elements;
}
