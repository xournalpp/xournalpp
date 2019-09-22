#include "XojPage.h"

#include "BackgroundImage.h"
#include "Document.h"

XojPage::XojPage(double width, double height)
{
	this->bgType.format = PageTypeFormat::Lined;

	this->width = width;
	this->height = height;
}

XojPage::~XojPage()
{
	for (Layer* l : this->layer)
	{
		delete l;
	}
	this->layer.clear();
}

void XojPage::reference()
{
	this->ref++;
}

void XojPage::unreference()
{
	this->ref--;
	if (ref < 1)
	{
		delete this;
	}
}

XojPage* XojPage::clone()
{
	XojPage* page = new XojPage(this->width, this->height);

	page->backgroundImage = this->backgroundImage;
	for (Layer* l : this->layer)
	{
		page->addLayer(l->clone());
	}

	page->currentLayer = this->currentLayer;
	page->bgType = this->bgType;
	page->pdfBackgroundPage = this->pdfBackgroundPage;
	page->backgroundColor = this->backgroundColor;

	return page;
}

void XojPage::addLayer(Layer* layer)
{
	this->layer.push_back(layer);
	this->currentLayer = npos;
}

void XojPage::insertLayer(Layer* layer, int index)
{
	if (index >= (int)this->layer.size())
	{
		addLayer(layer);
		return;
	}

	this->layer.insert(this->layer.begin() + index, layer);
	this->currentLayer = index + 1;
}

void XojPage::removeLayer(Layer* layer)
{
	for (unsigned int i = 0; i < this->layer.size(); i++)
	{
		if (layer == this->layer[i])
		{
			this->layer.erase(this->layer.begin() + i);
			break;
		}
	}
	this->currentLayer = npos;
}

void XojPage::setSelectedLayerId(int id)
{
	this->currentLayer = id;
}

vector<Layer*>* XojPage::getLayers()
{
	return &this->layer;
}

size_t XojPage::getLayerCount()
{
	return this->layer.size();
}

/**
 * Layer ID 0 = Background, Layer ID 1 = Layer 1
 */
int XojPage::getSelectedLayerId()
{
	if (this->currentLayer == npos)
	{
		this->currentLayer = this->layer.size();
	}

	return this->currentLayer;
}

void XojPage::setLayerVisible(int layerId, bool visible)
{
	if (layerId < 0)
	{
		return;
	}

	if (layerId == 0)
	{
		backgroundVisible = visible;
		return;
	}

	layerId--;
	if (layerId >= (int)this->layer.size())
	{
		return;
	}

	this->layer[layerId]->setVisible(visible);
}

bool XojPage::isLayerVisible(int layerId)
{
	if (layerId < 0)
	{
		return false;
	}

	if (layerId == 0)
	{
		return backgroundVisible;
	}

	layerId--;
	if (layerId >= (int)this->layer.size())
	{
		return false;
	}

	return this->layer[layerId]->isVisible();
}

bool XojPage::isLayerVisible(Layer* layer)
{
	return layer->isVisible();
}

void XojPage::setBackgroundPdfPageNr(size_t page)
{
	this->pdfBackgroundPage = page;
	this->bgType.format = PageTypeFormat::Pdf;
	this->bgType.config = "";
}

void XojPage::setBackgroundColor(int color)
{
	this->backgroundColor = color;
}

int XojPage::getBackgroundColor()
{
	return this->backgroundColor;
}

void XojPage::setSize(double width, double height)
{
	this->width = width;
	this->height = height;
}

double XojPage::getWidth() const
{
	return this->width;
}

double XojPage::getHeight() const
{
	return this->height;
}

size_t XojPage::getPdfPageNr()
{
	return this->pdfBackgroundPage;
}

bool XojPage::isAnnotated()
{
	for (Layer* l : this->layer)
	{
		if (l->isAnnotated())
		{
			return true;
		}
	}
	return false;
}

void XojPage::setBackgroundType(PageType bgType)
{
	this->bgType = bgType;

	if (!bgType.isPdfPage())
	{
		this->pdfBackgroundPage = npos;
	}
	if (!bgType.isImagePage())
	{
		this->backgroundImage.free();
	}
}

PageType XojPage::getBackgroundType()
{
	return this->bgType;
}

BackgroundImage& XojPage::getBackgroundImage()
{
	return this->backgroundImage;
}

void XojPage::setBackgroundImage(BackgroundImage img)
{
	this->backgroundImage = img;
}

Layer* XojPage::getSelectedLayer()
{
	if (this->layer.empty())
	{
		addLayer(new Layer());
	}
	size_t layer = getSelectedLayerId();

	if (layer > 0)
	{
		layer--;
	}

	return this->layer[layer];
}
