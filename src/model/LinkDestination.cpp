#include "LinkDestination.h"
// TODO: AA: type check

struct _LinkDestClass {
	GObjectClass base_class;
};

G_DEFINE_TYPE (XojLinkDest, link_dest, G_TYPE_OBJECT)

static void link_dest_init(XojLinkDest *linkAction) {
	linkAction->dest = NULL;
}

static void link_dest_finalize(GObject *object) {
	delete LINK_DEST(object)->dest;
	G_OBJECT_CLASS (object)->finalize(object);
}

static void link_dest_class_init(XojLinkDestClass *linkClass) {
	GObjectClass *g_object_class;

	g_object_class = G_OBJECT_CLASS (linkClass);

	g_object_class->finalize = link_dest_finalize;
}

XojLinkDest * link_dest_new() {
	return LINK_DEST (g_object_new (TYPE_LINK_DEST,
					NULL));
}

LinkDestination::LinkDestination() {
	this->page = -1;
	this->changeLeft = false;
	this->changeZoom = false;
	this->changeTop = false;
	this->zoom = 0;
	this->left = 0;
	this->top = 0;
}

int LinkDestination::getPdfPage() {
	return this->page;
}

void LinkDestination::setPdfPage(int page) {
	this->page = page;
}

void LinkDestination::setExpand(bool expand) {
	this->expand = expand;
}

bool LinkDestination::getExpand() {
	return this->expand;
}

bool LinkDestination::shouldChangeLeft() {
	return changeLeft;
}

bool LinkDestination::shouldChangeZoom() {
	return changeZoom;
}

bool LinkDestination::shouldChangeTop() {
	return changeTop;
}

double LinkDestination::getZoom() {
	return zoom;
}

double LinkDestination::getLeft() {
	return left;
}

double LinkDestination::getTop() {
	return top;
}

void LinkDestination::setChangeLeft(double left) {
	this->left = left;
	this->changeLeft = true;
}

void LinkDestination::setChangeZoom(double zoom) {
	this->zoom = zoom;
	this->changeZoom = true;
}

void LinkDestination::setChangeTop(double top) {
	this->top = top;
	this->changeTop = true;
}

void LinkDestination::setName(String name) {
	this->name = name;
}

String LinkDestination::getName() {
	return this->name;
}

