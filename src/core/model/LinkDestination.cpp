#include "LinkDestination.h"

#include <utility>

#include "util/Util.h"

struct _LinkDestClass {
    GObjectClass base_class;
};

G_DEFINE_TYPE(XojLinkDest, link_dest, G_TYPE_OBJECT)  // @suppress("Unused static function")

static void link_dest_init(XojLinkDest* linkAction) { linkAction->dest = nullptr; }

static gpointer parent_class = nullptr;

static void link_dest_finalize(GObject* object) {
    delete LINK_DEST(object)->dest;
    LINK_DEST(object)->dest = nullptr;

    G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void link_dest_dispose(GObject* object) { G_OBJECT_CLASS(parent_class)->dispose(object); }

static void link_dest_class_init(XojLinkDestClass* linkClass) {
    GObjectClass* g_object_class = nullptr;

    parent_class = g_type_class_peek_parent(linkClass);

    g_object_class = G_OBJECT_CLASS(linkClass);

    g_object_class->dispose = link_dest_dispose;
    g_object_class->finalize = link_dest_finalize;
}

auto link_dest_new() -> XojLinkDest* { return LINK_DEST(g_object_new(TYPE_LINK_DEST, nullptr)); }

LinkDestination::LinkDestination():
        page_(npos),
        changeLeft_(false),
        changeZoom_(false),
        changeTop_(false),
        zoom_(0),
        left_(0),
        top_(0),
        expand_(false),
        isURI_(false),
        name_(""),
        uri_("") {}

LinkDestination::~LinkDestination() = default;

auto LinkDestination::getPdfPage() const -> size_t { return page_; }

void LinkDestination::setPdfPage(size_t page) { page_ = page; }

void LinkDestination::setExpand(bool expand) { expand_ = expand; }

auto LinkDestination::getExpand() const -> bool { return expand_; }

auto LinkDestination::shouldChangeLeft() const -> bool { return changeLeft_; }

auto LinkDestination::shouldChangeTop() const -> bool { return changeTop_; }

auto LinkDestination::getZoom() const -> double { return zoom_; }

auto LinkDestination::getLeft() const -> double { return left_; }

auto LinkDestination::getTop() const -> double { return top_; }

void LinkDestination::setChangeLeft(double left) {
    left_ = left;
    changeLeft_ = true;
}

void LinkDestination::setChangeZoom(double zoom) {
    zoom_ = zoom;
    changeZoom_ = true;
}

void LinkDestination::setChangeTop(double top) {
    top_ = top;
    changeTop_ = true;
}

void LinkDestination::setName(std::string name) { name_ = std::move(name); }
auto LinkDestination::getName() -> std::string { return name_; }

void LinkDestination::setURI(std::string uri) {
    uri_ = std::move(uri);
    isURI_ = true;
}
auto LinkDestination::getURI() const -> std::string { return uri_; }
bool LinkDestination::isURI() const { return isURI_; }
