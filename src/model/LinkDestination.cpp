#include "LinkDestination.h"

struct _LinkDestClass {
	GObjectClass base_class;
};

G_DEFINE_TYPE (XojLinkDest, link_dest, G_TYPE_OBJECT)

static void link_dest_init(XojLinkDest * linkAction) {
	linkAction->dest = NULL;
}

static gpointer parent_class = NULL;

static void link_dest_finalize(GObject * object) {
	delete LINK_DEST(object)->dest;
	LINK_DEST(object)->dest = NULL;

	G_OBJECT_CLASS (parent_class)->finalize(object);
}

static void link_dest_dispose(GObject * object) {
	G_OBJECT_CLASS (parent_class)->dispose(object);
}

static void link_dest_class_init(XojLinkDestClass *linkClass) {
	GObjectClass *g_object_class;

	parent_class = g_type_class_peek_parent(linkClass);

	g_object_class = G_OBJECT_CLASS (linkClass);

	g_object_class->dispose = link_dest_dispose;
	g_object_class->finalize = link_dest_finalize;
}

XojLinkDest * link_dest_new() {
	return LINK_DEST(g_object_new (TYPE_LINK_DEST, NULL));
}


LinkDestination::LinkDestination() {
	XOJ_INIT_TYPE(LinkDestination);

	this->page = -1;
	this->changeLeft = false;
	this->changeZoom = false;
	this->changeTop = false;
	this->zoom = 0;
	this->left = 0;
	this->top = 0;
}

LinkDestination::~LinkDestination() {
	XOJ_RELEASE_TYPE(LinkDestination);
}

int LinkDestination::getPdfPage() {
	XOJ_CHECK_TYPE(LinkDestination);

	return this->page;
}

void LinkDestination::setPdfPage(int page) {
	XOJ_CHECK_TYPE(LinkDestination);

	this->page = page;
}

void LinkDestination::setExpand(bool expand) {
	XOJ_CHECK_TYPE(LinkDestination);

	this->expand = expand;
}

bool LinkDestination::getExpand() {
	XOJ_CHECK_TYPE(LinkDestination);

	return this->expand;
}

bool LinkDestination::shouldChangeLeft() {
	XOJ_CHECK_TYPE(LinkDestination);

	return changeLeft;
}

bool LinkDestination::shouldChangeZoom() {
	XOJ_CHECK_TYPE(LinkDestination);

	return changeZoom;
}

bool LinkDestination::shouldChangeTop() {
	XOJ_CHECK_TYPE(LinkDestination);

	return changeTop;
}

double LinkDestination::getZoom() {
	XOJ_CHECK_TYPE(LinkDestination);

	return zoom;
}

double LinkDestination::getLeft() {
	XOJ_CHECK_TYPE(LinkDestination);

	return left;
}

double LinkDestination::getTop() {
	XOJ_CHECK_TYPE(LinkDestination);

	return top;
}

void LinkDestination::setChangeLeft(double left) {
	XOJ_CHECK_TYPE(LinkDestination);

	this->left = left;
	this->changeLeft = true;
}

void LinkDestination::setChangeZoom(double zoom) {
	XOJ_CHECK_TYPE(LinkDestination);

	this->zoom = zoom;
	this->changeZoom = true;
}

void LinkDestination::setChangeTop(double top) {
	XOJ_CHECK_TYPE(LinkDestination);

	this->top = top;
	this->changeTop = true;
}

void LinkDestination::setName(String name) {
	XOJ_CHECK_TYPE(LinkDestination);

	this->name = name;
}

String LinkDestination::getName() {
	XOJ_CHECK_TYPE(LinkDestination);

	return this->name;
}

