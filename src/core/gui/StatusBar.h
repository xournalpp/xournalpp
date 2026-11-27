/*
 * Xournal++
 *
 * A progress bar wrapper
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>

#include <gtk/gtk.h>

#include "control/jobs/ProgressListener.h"
#include "util/raii/GObjectSPtr.h"

class MainWindow;

class StatusBar: public ProgressListener {
public:
    StatusBar(MainWindow& parent);
    ~StatusBar() override;

    void setMaximumState(size_t max) override;
    void setCurrentState(size_t state) override;

    /// The parameter is encoded in UTF8
    void setStateLabel(std::string_view lbl);

    void setVisible(bool visible);

private:
    xoj::util::WidgetSPtr statusbar;
    xoj::util::GObjectSPtr<GtkLabel> statusLabel;
    xoj::util::GObjectSPtr<GtkProgressBar> progressbar;
    size_t maxState;
};
