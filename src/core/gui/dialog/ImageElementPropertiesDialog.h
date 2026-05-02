/*
 * Xournal++
 *
 * Properties dialog for image elements
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>

#include <gtk/gtk.h>

#include "util/raii/GtkWindowUPtr.h"

class GladeSearchpath;
class Settings;

namespace xoj::popup {
class ImageElementPropertiesDialog {
public:
    ImageElementPropertiesDialog(GladeSearchpath* gladeSearchPath, Settings* settings, double width, double height,
                                 std::function<void(double, double)> callback);
    ~ImageElementPropertiesDialog();

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    enum class LastEditedDimension { Width, Height };

    void setDimensions(double width, double height);
    void applyAspectRatioFromWidth();
    void applyAspectRatioFromHeight();

private:
    Settings* settings = nullptr;
    GtkSpinButton* widthSpin = nullptr;
    GtkSpinButton* heightSpin = nullptr;
    GtkCheckButton* keepAspectButton = nullptr;
    xoj::util::GtkWindowUPtr window;
    std::function<void(double, double)> callback;
    double scale = 1.0;
    double aspectRatio = 1.0;
    double initialWidth = 0.0;
    double initialHeight = 0.0;
    bool ignoreValueChange = false;
    LastEditedDimension lastEditedDimension = LastEditedDimension::Width;
};
}  // namespace xoj::popup
