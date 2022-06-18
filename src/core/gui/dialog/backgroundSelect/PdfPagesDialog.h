/*
 * Xournal++
 *
 * Dialog to select a PDF page (to insert as background)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkButton, GtkToggleButton

#include "BackgroundSelectDialogBase.h"  // for BackgroundSelectDialogBase

class Document;
class GladeSearchpath;
class Settings;


class PdfPagesDialog: public BackgroundSelectDialogBase {
public:
    PdfPagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings);
    ~PdfPagesDialog() override;

public:
    void show(GtkWindow* parent) override;
    void updateOkButton();
    static double getZoom();
    int getSelectedPage();

private:
    static void onlyNotUsedCallback(GtkToggleButton* tb, PdfPagesDialog* dlg);
    static void okButtonCallback(GtkButton* button, PdfPagesDialog* dlg);

private:
};
