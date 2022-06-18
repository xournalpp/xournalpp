/*
 * Xournal++
 *
 * Dialog to select a Image (to insert as background)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkButton, GtkWindow

#include "BackgroundSelectDialogBase.h"  // for BackgroundSelectDialogBase

struct BackgroundImage;
class Document;
class GladeSearchpath;
class Settings;

class ImagesDialog: public BackgroundSelectDialogBase {
public:
    ImagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings);
    ~ImagesDialog() override;

public:
    BackgroundImage getSelectedImage();
    bool shouldShowFilechooser();
    void show(GtkWindow* parent) override;

private:
    /**
     * Load all images form all pages
     */
    void loadImagesFromPages();

    /**
     * Check if this image is already displayed
     */
    bool isImageAlreadyInTheList(BackgroundImage& image);

    static void okButtonCallback(GtkButton* button, ImagesDialog* dlg);
    static void filechooserButtonCallback(GtkButton* button, ImagesDialog* dlg);

private:
};
