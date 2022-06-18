/*
 * Xournal++
 *
 * GTK Open dialog to select image with preview
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gio/gio.h>                // for GFile
#include <glib.h>                   // for gint
#include <gtk/gtk.h>                // for GtkFileChooser, GtkWindow

class Settings;

class ImageOpenDlg {
private:
    ImageOpenDlg();
    virtual ~ImageOpenDlg();

public:
    static GFile* show(GtkWindow* win, Settings* settings, bool localOnly = false, bool* attach = nullptr);

private:
    static void updatePreviewCallback(GtkFileChooser* fileChooser, void* userData);
    static GdkPixbuf* pixbufScaleDownIfNecessary(GdkPixbuf* pixbuf, gint maxSize);
};
