#include "BackgroundImage.h"

#include <string>   // for string
#include <utility>  // for move

#include <glib-object.h>  // for g_object_unref

#include "util/Stacktrace.h"  // for Stacktrace

/*
 * The contents of a background image
 *
 * Internal impl object, dont move this to an external header/source file due this is the best way to reduce code
 * bloat and increase encapsulation. This object is only used in this source scope and is a RAII Container for the
 * GdkPixbuf*
 * No xournal memory leak tests necessary, because we use smart ptrs to ensure memory correctness
 */

struct BackgroundImage::Content {
    Content(fs::path path, GError** error):
            path(std::move(path)), pixbuf(gdk_pixbuf_new_from_file(this->path.u8string().c_str(), error)) {}

    Content(GInputStream* stream, fs::path path, GError** error):
            path(std::move(path)), pixbuf(gdk_pixbuf_new_from_stream(stream, nullptr, error)) {}

    ~Content() {
        g_object_unref(this->pixbuf);
        this->pixbuf = nullptr;
    };

    Content(const Content&) = delete;
    Content(Content&&) = default;
    auto operator=(const Content&) -> Content& = delete;
    auto operator=(Content&&) -> Content& = default;

    fs::path path;
    GdkPixbuf* pixbuf = nullptr;
    int pageId = -1;
    bool attach = false;
};

BackgroundImage::BackgroundImage() = default;

BackgroundImage::BackgroundImage(const BackgroundImage& img) = default;

BackgroundImage::BackgroundImage(BackgroundImage&& img) noexcept: img(std::move(img.img)) {}

BackgroundImage::~BackgroundImage() = default;

auto BackgroundImage::operator==(const BackgroundImage& img) -> bool { return this->img == img.img; }

void BackgroundImage::free() { this->img.reset(); }

void BackgroundImage::loadFile(fs::path const& path, GError** error) {
    this->img = std::make_shared<Content>(path, error);
}

void BackgroundImage::loadFile(GInputStream* stream, fs::path const& path, GError** error) {
    this->img = std::make_shared<Content>(stream, path, error);
}

auto BackgroundImage::getCloneId() -> int { return this->img ? this->img->pageId : -1; }

void BackgroundImage::setCloneId(int id) {
    if (this->img) {
        this->img->pageId = id;
    }
}

void BackgroundImage::clearSaveState() { this->setCloneId(-1); }

auto BackgroundImage::getFilepath() const -> fs::path { return this->img ? this->img->path : fs::path{}; }

void BackgroundImage::setFilepath(fs::path path) {
    if (this->img) {
        this->img->path = std::move(path);
    }
}

auto BackgroundImage::isAttached() const -> bool { return this->img ? this->img->attach : false; }

void BackgroundImage::setAttach(bool attach) {
    if (!this->img) {
        g_warning("BackgroundImage::setAttach: please load first an image before call setAttach!");
        Stacktrace::printStracktrace();
        return;
    }
    this->img->attach = attach;
}

auto BackgroundImage::getPixbuf() const -> GdkPixbuf* { return this->img ? this->img->pixbuf : nullptr; }

auto BackgroundImage::isEmpty() const -> bool { return !this->img; }
