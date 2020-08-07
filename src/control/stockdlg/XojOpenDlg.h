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

#include <gtk/gtk.h>
#include <filesystem>

#include "control/settings/Settings.h"


class XojOpenDlg {
public:
    XojOpenDlg(GtkWindow* win, Settings* settings);
    virtual ~XojOpenDlg();

public:
    std::filesystem::path showOpenDialog(bool pdf, bool& attachPdf);
    std::filesystem::path showOpenTemplateDialog();

protected:
    void addFilterAllFiles();
    void addFilterPdf();
    void addFilterXoj();
    void addFilterXopp();
    void addFilterXopt();

    std::filesystem::path runDialog();

private:
    static void updatePreviewCallback(GtkFileChooser* fileChooser, void* userData);

private:
    GtkWidget* dialog;

    GtkWindow* win;
    Settings* settings;
};
