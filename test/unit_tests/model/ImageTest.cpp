/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <fstream>

#include <config-test.h>
#include <gtest/gtest.h>

#include "model/Image.h"


TEST(Image, testGetImageApplyOrientation) {
    auto image = Image();

    // Image width is 500px and height 130px - but has exif data saying image should be
    // rotated 90 deg CW to have correct orientation.
    std::ifstream imageFile{GET_TESTFILE("images/r90.jpg"), std::ios::binary};
    auto imageData = std::string(std::istreambuf_iterator<char>(imageFile), {});

    GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
    gdk_pixbuf_loader_write(loader, reinterpret_cast<const guchar*>(imageData.c_str()), imageData.length(), nullptr);
    gdk_pixbuf_loader_close(loader, nullptr);

    GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);

    // Image size before  orientation
    auto origImageSize = std::make_pair(gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
    auto rotatedImageSize = std::make_pair(origImageSize.second, origImageSize.first);
    g_object_unref(loader);

    image.setImage(imageData);

    // Test Image object has no size before the image has be rendered
    EXPECT_EQ(image.getImageSize(), Image::NOSIZE);

    // getImage render the image in a cairo surface
    auto surface = image.getImage();

    // Test image now have the correct size - which is the image has been rotated.
    EXPECT_EQ(image.getImageSize(), rotatedImageSize);
    EXPECT_EQ(image.getImageSize(), std::make_pair(130, 500));
    EXPECT_EQ(std::make_pair(cairo_image_surface_get_width(surface), cairo_image_surface_get_height(surface)),
              rotatedImageSize);
}
