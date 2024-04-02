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
#include <string>       // for string
#include <string_view>  // for string_view
#include <utility>      // for pair, make_pair

#include <cairo.h>                  // for cairo_surface_t, cairo_status_t
#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbufFormat, GdkPixbuf

#include "Element.h"  // for Element

class ObjectInputStream;
class ObjectOutputStream;


class Image final: public Element {
public:
    Image();
    Image(Image&&) = delete;
    Image(Image const&) = delete;
    auto operator=(Image&&) -> Image& = delete;
    auto operator=(Image const&) -> Image& = delete;
    ~Image() override;

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

    /// Returns the internal surface that contains the rendered image data.
    ///
    /// Note that the image is rendered lazily by default; call this method to render it.
    auto getImage() const -> cairo_surface_t*;

    auto clone() const -> ElementPtr override;

    auto hasData() const -> bool;

    /// Return a pointer to the raw data. Note that the pointer will be invalidated if the data is changed.
    auto getRawData() const -> const unsigned char*;

    /// Return the length of the raw data.
    auto getRawDataLength() const -> size_t;

    /// Return the size of the raw image, or (-1, -1) if the image has not been rendered yet.
    auto getImageSize() const -> std::pair<int, int>;

    [[maybe_unused]] auto getImageFormat() const -> GdkPixbufFormat*;

    static constexpr std::pair<int, int> NOSIZE = std::make_pair(-1, -1);

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

private:
    auto internalUpdateBounds() const -> std::pair<xoj::util::Rectangle<double>, xoj::util::Rectangle<double>> override;

    static auto cairoReadFunction(const Image* image, unsigned char* data, unsigned int length) -> cairo_status_t;

private:
    /// Set the image data by rendering the surface to PNG and copying the PNG data.
    ///
    /// \deprecated Pass the raw image data instead.
    ///
    /// FIXME: remove this when setImage(GdkPixbuf*) is removed.
    [[deprecated]] void setImage(cairo_surface_t* image);

    /// Temporary surface used as a render buffer.
    mutable cairo_surface_t* image = nullptr;

    /// Image format information.
    mutable GdkPixbufFormat* format = nullptr;
    mutable std::pair<int, int> imageSize = {0, 0};

    std::string data;
};
