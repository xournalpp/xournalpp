/*
 * Xournal++
 *
 * An Image on the document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>      // for size_t
#include <optional>     // for optional
#include <string>       // for string
#include <string_view>  // for string_view
#include <utility>      // for pair, make_pair

#include <cairo.h>                  // for cairo_surface_t, cairo_status_t
#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbufFormat, GdkPixbuf

#include "util/raii/CairoWrappers.h"

#include "RectangularElement.h"

class ObjectInputStream;
class ObjectOutputStream;


class Image: public RectangularElement {
public:
    Image();
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&&) = delete;
    Image& operator=(Image&&) = delete;
    virtual ~Image();

public:
    /// Set the image data by copying the data from the provided string_view.
    void setImage(std::string_view data);

    /// Set the image data by moving the data.
    void setImage(std::string&& data);

    /// Set the image data by copying the data from the provided pixbuf.
    ///
    /// \deprecated Pass the raw image data instead.
    ///
    /// FIXME: remove this method. Currently, it is used by Control::clipboardPasteImage.
    [[deprecated]] void setImage(GdkPixbuf* img);

    /// The image is rendered lazily by default; call this method to render it.
    /// Returns std::nullopt on success, an error message on failure
    std::optional<std::string> renderBuffer() const;

    /// Returns the internal surface that contains the rendered image data.
    cairo_surface_t* getImage() const;

    auto clone() const -> ElementPtr override;

    bool hasData() const;

    /// Return a pointer to the raw data. Note that the pointer will be invalidated if the data is changed.
    const unsigned char* getRawData() const;

    /// Return the length of the raw data.
    size_t getRawDataLength() const;

    [[maybe_unused]] GdkPixbufFormat* getImageFormat() const;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

private:
    void calcSize() const override;

private:
    /// Temporary surface used as a render buffer.
    mutable xoj::util::raii::CairoSurfaceSPtr image;

    /// Image format information.
    mutable GdkPixbufFormat* format = nullptr;

    std::string data;
};
