#include "LinkDestination.h"

#include "Util.h"

struct _LinkDestClass
{
	GObjectClass base_class;
};

G_DEFINE_TYPE(XojLinkDest, link_dest, G_TYPE_OBJECT) // @suppress("Unused static function")

static void link_dest_init(XojLinkDest* linkAction)
{
	linkAction->dest = nullptr;
}

static gpointer parent_class = nullptr;

static void link_dest_finalize(GObject* object)
{
	delete LINK_DEST(object)->dest;
	LINK_DEST(object)->dest = nullptr;

	G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void link_dest_dispose(GObject* object)
{
	G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void link_dest_class_init(XojLinkDestClass* linkClass)
{
	GObjectClass* g_object_class;

	parent_class = g_type_class_peek_parent(linkClass);

	g_object_class = G_OBJECT_CLASS(linkClass);

	g_object_class->dispose = link_dest_dispose;
	g_object_class->finalize = link_dest_finalize;
}

XojLinkDest* link_dest_new()
{
	return LINK_DEST(g_object_new(TYPE_LINK_DEST, nullptr));
}

LinkDestination::LinkDestination()
{
	this->page = npos;
	this->changeLeft = false;
	this->changeZoom = false;
	this->changeTop = false;
	this->zoom = 0;
	this->left = 0;
	this->top = 0;
	this->expand = false;
}

LinkDestination::~LinkDestination()
{
}

size_t LinkDestination::getPdfPage()
{
	return this->page;
}

void LinkDestination::setPdfPage(size_t page)
{
	this->page = page;
}

void LinkDestination::setExpand(bool expand)
{
	this->expand = expand;
}

bool LinkDestination::getExpand()
{
	return this->expand;
}

bool LinkDestination::shouldChangeLeft()
{
	return changeLeft;
}

bool LinkDestination::shouldChangeZoom()
{
	return changeZoom;
}

bool LinkDestination::shouldChangeTop()
{
	return changeTop;
}

double LinkDestination::getZoom()
{
	return zoom;
}

double LinkDestination::getLeft()
{
	return left;
}

double LinkDestination::getTop()
{
	return top;
}

void LinkDestination::setChangeLeft(double left)
{
	this->left = left;
	this->changeLeft = true;
}

void LinkDestination::setChangeZoom(double zoom)
{
	this->zoom = zoom;
	this->changeZoom = true;
}

void LinkDestination::setChangeTop(double top)
{
	this->top = top;
	this->changeTop = true;
}

void LinkDestination::setName(string name)
{
	this->name = name;
}

string LinkDestination::getName()
{
	return this->name;
}
