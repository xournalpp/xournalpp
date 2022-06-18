/*
 * Xournal++
 *
 * GTK Open dialog to select XOJ (or PDF) file with preview
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkWindow, GtkFileChooser, GtkWidget

#include "filesystem.h"  // for path

class Settings;


class XojOpenDlg {
public:
    XojOpenDlg(GtkWindow* win, Settings* settings);
    virtual ~XojOpenDlg();

public:
    fs::path showOpenDialog(bool pdf, bool& attachPdf);
    fs::path showOpenTemplateDialog();

protected:
    void addFilterAllFiles();
    void addFilterPdf();
    void addFilterXoj();
    void addFilterXopp();
    void addFilterXopt();

    fs::path runDialog();

private:
    static void updatePreviewCallback(GtkFileChooser* fileChooser, void* userData);

private:
    GtkWidget* dialog;

    GtkWindow* win;
    Settings* settings;
};
