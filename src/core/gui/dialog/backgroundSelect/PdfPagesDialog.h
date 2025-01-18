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


class PdfPagesDialog final: public BackgroundSelectDialogBase {
public:
    PdfPagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings,
                   std::function<void(size_t)> callback);
    ~PdfPagesDialog();

public:
    void updateOkButton();

    static constexpr double ZOOM_VALUE = 0.25;

private:
    static void onlyNotUsedCallback(GtkToggleButton* tb, PdfPagesDialog* dlg);

    GtkCheckButton* cbUnusedOnly;

    /// Takes parameter the pdf page number
    std::function<void(size_t)> callback;
};
