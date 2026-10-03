/*
 * Xournal++
 *
 * Dialog with export settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <functional>
#include <memory>

#include <gtk/gtk.h>  // for GtkComboBox, GtkWindow

#include "gui/Builder.h"
#include "util/raii/GtkWindowUPtr.h"

class GladeSearchpath;
struct ExportParameters;

namespace xoj::popup {
class ExportDialog {
public:
    ExportDialog(GladeSearchpath* gladeSearchPath, size_t currentPage, size_t pageCount, bool hasPdfBackground,
                 std::function<void(std::unique_ptr<ExportParameters>)> callbackFun);
    ~ExportDialog();

public:
    inline GtkWindow* getWindow() const { return window.get(); }

private:
    /**
     * @brief Handler for changes in combobox cbQuality
     */
    static void selectQualityCriterion(GtkComboBox* comboBox, ExportDialog* self);

    static void onSuccessCallback(ExportDialog* self);

private:
    xoj::util::GtkWindowUPtr window;

    size_t currentPage = 0;
    size_t pageCount = 0;

    bool confirmed = false;
    std::unique_ptr<ExportParameters> parameters;

    Builder builder;

    std::function<void(std::unique_ptr<ExportParameters>)> callbackFun;
};
};  // namespace xoj::popup
