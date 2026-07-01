/*
 * Xournal++
 *
 * Handle automatic reloading of external PDF backgrounds
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <optional>

#include <glib.h>

#include "filesystem.h"

class Control;

class PdfAutoReloadController final {
public:
    explicit PdfAutoReloadController(Control* control);
    ~PdfAutoReloadController();

    void startMonitoring();
    void stopMonitoring();
    void onPdfChanged();

private:
    bool shouldPollForPdfChanges() const;
    static bool onPollingTimeout(PdfAutoReloadController* controller);

    void reconfigurePolling();
    void checkForModifiedPdfAndReload();
    void reloadExternalPdfAndRefreshDocument();
    void clearPendingReloadState();

private:
    Control* control = nullptr;
    guint timeoutId = 0;
    bool monitoringStarted = false;
    std::optional<fs::file_time_type> pendingReloadTime;
    gint64 pendingReloadSinceUs = 0;
};
