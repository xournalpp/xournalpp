#include "PdfAutoReloadController.h"

#include <system_error>

#include "control/PageBackgroundChangeController.h"
#include "control/settings/Settings.h"
#include "model/Document.h"
#include "util/glib_casts.h"

#include "Control.h"

namespace {

struct MonitoredPdfState {
    std::optional<fs::file_time_type> loadedModifiedTime;
    fs::path pdfPath;
};

auto tryReadMonitoredPdfState(Document* doc, MonitoredPdfState* state, bool nonBlocking) -> bool {
    const bool locked = nonBlocking ? doc->try_lock_shared() : (doc->lock_shared(), true);
    if (!locked) {
        return false;
    }

    const bool canAutoReload = doc->canAutoReloadPdf();
    if (canAutoReload && state != nullptr) {
        state->loadedModifiedTime = doc->getPdfLastModifiedTime();
        state->pdfPath = doc->getPdfFilepath();
    }
    doc->unlock_shared();

    return canAutoReload;
}

}  // namespace

PdfAutoReloadController::PdfAutoReloadController(Control* control): control(control) {}

PdfAutoReloadController::~PdfAutoReloadController() { stopMonitoring(); }

void PdfAutoReloadController::startMonitoring() {
    this->monitoringStarted = true;
    reconfigurePolling();
}

void PdfAutoReloadController::stopMonitoring() {
    this->monitoringStarted = false;
    if (this->timeoutId) {
        g_source_remove(this->timeoutId);
        this->timeoutId = 0;
    }
    clearPendingReloadState();
}

void PdfAutoReloadController::onPdfChanged() {
    if (!this->monitoringStarted) {
        return;
    }
    reconfigurePolling();
}

bool PdfAutoReloadController::onPollingTimeout(PdfAutoReloadController* controller) {
    // Periodically poll the external PDF, debounce mtime changes, and reload once the file is stable.
    controller->checkForModifiedPdfAndReload();
    return true;
}

bool PdfAutoReloadController::shouldPollForPdfChanges() const {
    return this->monitoringStarted && this->control->getSettings()->getPdfAutoReloadEnabled();
}

void PdfAutoReloadController::reconfigurePolling() {
    if (this->timeoutId) {
        g_source_remove(this->timeoutId);
        this->timeoutId = 0;
    }

    if (!shouldPollForPdfChanges()) {
        clearPendingReloadState();
        return;
    }

    auto* doc = this->control->getDocument();
    // Reconfiguration runs in direct response to document/state changes, so waiting for a shared lock here is fine.
    if (!tryReadMonitoredPdfState(doc, nullptr, false)) {
        clearPendingReloadState();
        return;
    }

    this->timeoutId = g_timeout_add(this->control->getSettings()->getPdfAutoReloadIntervalMs(),
                                    xoj::util::wrap_v<onPollingTimeout>, this);
}

void PdfAutoReloadController::checkForModifiedPdfAndReload() {
    if (!shouldPollForPdfChanges()) {
        clearPendingReloadState();
        return;
    }

    auto* doc = this->control->getDocument();
    MonitoredPdfState state;
    // Polling runs on the GTK main loop. Avoid blocking the UI if a writer currently owns the document lock.
    if (!tryReadMonitoredPdfState(doc, &state, true)) {
        clearPendingReloadState();
        return;
    }

    std::error_code ec;
    const auto currentModifiedTime = fs::last_write_time(state.pdfPath, ec);
    if (ec || (state.loadedModifiedTime && currentModifiedTime <= *state.loadedModifiedTime)) {
        clearPendingReloadState();
        return;
    }

    if (!this->pendingReloadTime || *this->pendingReloadTime != currentModifiedTime) {
        this->pendingReloadTime = currentModifiedTime;
        this->pendingReloadSinceUs = g_get_monotonic_time();
        return;
    }

    const gint64 stableForUs = g_get_monotonic_time() - this->pendingReloadSinceUs;
    const guint requiredStableUs = this->control->getSettings()->getPdfAutoReloadDebounceMs() * 1000;
    if (stableForUs < requiredStableUs) {
        return;
    }

    reloadExternalPdfAndRefreshDocument();
}

void PdfAutoReloadController::reloadExternalPdfAndRefreshDocument() {
    clearPendingReloadState();

    auto* doc = this->control->getDocument();
    if (!doc->reloadPdf()) {
        return;
    }

    this->control->getPageBackgroundChangeController()->resizePagesToMatchPdf();
    this->control->firePdfContentChanged();
    onPdfChanged();
}

void PdfAutoReloadController::clearPendingReloadState() {
    this->pendingReloadTime.reset();
    this->pendingReloadSinceUs = 0;
}
