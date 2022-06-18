/*
 * Xournal++
 *
 * A background image of a page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for shared_ptr

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gio/gio.h>                // for GInputStream
#include <glib.h>                   // for GError

#include "filesystem.h"  // for path

struct BackgroundImage {
    BackgroundImage();
    BackgroundImage(const BackgroundImage& img);
    BackgroundImage(BackgroundImage&& img) noexcept;
    ~BackgroundImage();

    BackgroundImage& operator=(const BackgroundImage& img) = default;
    BackgroundImage& operator=(BackgroundImage&& img) = default;


    bool operator==(const BackgroundImage& img);

    void free();

    void loadFile(fs::path const& filepath, GError** error);
    void loadFile(GInputStream* stream, fs::path const& filepath, GError** error);

    int getCloneId();
    void setCloneId(int id);
    void clearSaveState();

    fs::path getFilepath() const;
    void setFilepath(fs::path filepath);

    bool isAttached() const;
    void setAttach(bool attach);

    GdkPixbuf* getPixbuf() const;

    bool isEmpty() const;

private:
    struct Content;

    std::shared_ptr<Content> img;
};
