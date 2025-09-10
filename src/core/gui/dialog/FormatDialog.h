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

#include <functional>
#include <memory>  // for unique_ptr
#include <vector>  // for vector

#include <gtk/gtk.h>  // for gtk_paper_size_free, GtkComboBox, GtkToggl...

#include "gui/Builder.h"
#include "gui/PaperFormatUtils.h"  // for PaperFormatMenuOptionVector
#include "util/raii/GtkWindowUPtr.h"

class GladeSearchpath;
class Settings;

namespace xoj::popup {
class FormatDialog {
public:
    FormatDialog(GladeSearchpath* gladeSearchPath, Settings* settings, double width, double height,
                 std::function<void(double w, double h)> callback);

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    enum Orientation { ORIENTATION_NOT_DEFINED, ORIENTATION_LANDSCAPE, ORIENTATION_PORTRAIT };

    void setOrientation(Orientation orientation);
    void setSpinValues(double width, double heigth);

    static void portraitSelectedCb(GtkToggleButton* bt, FormatDialog* dlg);
    static void landscapeSelectedCb(GtkToggleButton* bt, FormatDialog* dlg);
    static void cbFormatChangedCb(GtkComboBox* widget, FormatDialog* dlg);
    static void cbUnitChanged(GtkComboBox* widget, FormatDialog* dlg);
    static void spinValueChangedCb(FormatDialog* dlg);
    static void reset(FormatDialog* self);

private:
    Settings* settings = nullptr;

    PaperFormatUtils::PaperFormatMenuOptionVector paperSizes;

    Orientation orientation = ORIENTATION_NOT_DEFINED;
    int selectedScale = 0;
    double scale = 0;

    double origWidth = 0;
    double origHeight = 0;

    bool ignoreSpinChange = false;


    GtkComboBox* paperTemplatesCombo;
    GtkSpinButton* widthSpin;
    GtkSpinButton* heightSpin;
    GtkToggleButton* landscapeButton;
    GtkToggleButton* portraitButton;

    xoj::util::GtkWindowUPtr window;
    std::function<void(double w, double h)> callbackFun;
};
};  // namespace xoj::popup
