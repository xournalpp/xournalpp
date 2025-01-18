/*
 * Xournal++
 *
 * Image Tool handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>
#include <memory>

#include "model/PageRef.h"
#include "util/Rectangle.h"

#include "filesystem.h"

class Control;
class Image;

class ImageHandler final {
public:
    ImageHandler(Control* control);
    ~ImageHandler();

public:
    /**
     * inserts an image scaled to the given size
     */
    void insertImageWithSize(PageRef page, const xoj::util::Rectangle<double>& space);

    /// Creates the image from the given file
    [[nodiscard]] static auto createImageFromFile(const fs::path& path) -> std::unique_ptr<Image>;

    static bool addImageToDocument(std::unique_ptr<Image> img, PageRef page, Control* ctrl, bool addUndoAction);

    /**
     * scale down (only if necessary) the image so that it then fits on the page
     * applies (potentially adjusted) width/height to the image
     */
    static void automaticScaling(Image& img, PageRef page);

    /// lets the user choose an image file, creates the image and calls the callback
    void chooseAndCreateImage(std::function<void(std::unique_ptr<Image>)> callback);

private:
    Control* control;
};
