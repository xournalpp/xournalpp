#include "LinkDestination.h"

#include <utility>  // for move

#include "util/Util.h"  // for npos

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
        page(npos),
        expand(false),
        left(0),
        top(0),
        zoom(0),
        changeLeft(false),
        changeZoom(false),
        changeTop(false),
        name(""),
        contents(UnknownType{}) {}

LinkDestination::~LinkDestination() = default;

auto LinkDestination::getPdfPage() const -> size_t { return this->page; }

void LinkDestination::setPdfPage(size_t page) { this->page = page; }

void LinkDestination::setExpand(bool expand) { this->expand = expand; }

auto LinkDestination::getExpand() const -> bool { return this->expand; }

auto LinkDestination::shouldChangeLeft() const -> bool { return changeLeft; }

auto LinkDestination::shouldChangeTop() const -> bool { return changeTop; }

auto LinkDestination::getZoom() const -> double { return zoom; }

auto LinkDestination::getLeft() const -> double { return left; }

auto LinkDestination::getTop() const -> double { return top; }

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

void LinkDestination::setName(std::string name) { this->name = std::move(name); }
auto LinkDestination::getName() const -> const std::string& { return name; }

void LinkDestination::setURI(std::string uri) { this->contents = LinkType{std::move(uri)}; }

auto LinkDestination::isURI() const -> bool { return std::holds_alternative<LinkDestination::LinkType>(contents); }

auto LinkDestination::getURI() const -> std::optional<std::string> {
    if (auto* t = std::get_if<LinkDestination::LinkType>(&contents)) {
        return t->uri;
    } else {
        return std::nullopt;
    }
}
