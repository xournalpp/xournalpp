/*
 * Xournal++
 *
 * The page format dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr
#include <vector>  // for vector

#include <gtk/gtk.h>  // for gtk_paper_size_free, GtkComboBox, GtkToggl...

#include "gui/GladeGui.h"  // for GladeGui

class GladeSearchpath;
class Settings;

enum Orientation { ORIENTATION_NOT_DEFINED, ORIENTATION_LANDSCAPE, ORIENTATION_PORTRAIT };

class FormatDialog: public GladeGui {
public:
    FormatDialog(GladeSearchpath* gladeSearchPath, Settings* settings, double width, double height);

public:
    void show(GtkWindow* parent) override;

    double getWidth() const;
    double getHeight() const;

private:
    void loadPageFormats();
    void setOrientation(Orientation orientation);
    void setSpinValues(double width, double heigth);

    static void portraitSelectedCb(GtkToggleToolButton* bt, FormatDialog* dlg);
    static void landscapeSelectedCb(GtkToggleToolButton* bt, FormatDialog* dlg);
    static void cbFormatChangedCb(GtkComboBox* widget, FormatDialog* dlg);
    static void cbUnitChanged(GtkComboBox* widget, FormatDialog* dlg);
    static void spinValueChangedCb(GtkSpinButton* spinbutton, FormatDialog* dlg);

private:
    Settings* settings = nullptr;

    std::vector<std::unique_ptr<GtkPaperSize, decltype(&gtk_paper_size_free)>> paperSizes;

    Orientation orientation = ORIENTATION_NOT_DEFINED;
    double scale = 0;
    int selectedScale = 0;

    double origWidth = 0;
    double origHeight = 0;

    double width = -1;
    double height = -1;

    double ignoreSpinChange = false;
};
