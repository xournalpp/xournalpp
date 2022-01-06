#include "PluginNotifier.h"

#include <chrono>

#include "model/DocumentListener.h"

PluginNotifier::PluginNotifier(Control* control): control(control), startTime(std::chrono::steady_clock::now()) {}

void PluginNotifier::documentChanged(DocumentChangeType type) {
    std::chrono::duration<double> timeSpan = std::chrono::steady_clock::now() - this->startTime;
    this->documentChangeTime = timeSpan.count();
    g_message("Document changed at time: %f", this->documentChangeTime);
    changeTimes.clear();
    for (size_t i = 0; i < control->getDocument()->getPageCount(); ++i) {
        changeTimes.push_back(NAN);
        updateChangeTime(i);
    }
}

void PluginNotifier::pageSizeChanged(size_t page) {
    g_message("size changed for page %zu", page);
    updateChangeTime(page);
}

void PluginNotifier::pageChanged(size_t page) {
    g_message("page %zu changed", page);
    updateChangeTime(page);
}

void PluginNotifier::pageInserted(size_t page) {
    g_message("page %zu inserted", page);
    changeTimes.push_back(NAN);
    for (auto i = page; i < changeTimes.size(); ++i) { updateChangeTime(i); }
}

void PluginNotifier::pageDeleted(size_t page) {
    g_message("page %zu deleted", page);
    changeTimes.pop_back();
    for (auto i = page; i < changeTimes.size(); ++i) { updateChangeTime(i); }
}

void PluginNotifier::pageSelected(size_t page) {
    // g_message("page %zu selected", page);
}

void PluginNotifier::updateChangeTime(size_t page) {
    std::chrono::duration<double> timeSpan = std::chrono::steady_clock::now() - this->startTime;
    if (page < changeTimes.size()) {
        changeTimes.at(page) = timeSpan.count();
        g_message("Page %zu has been updated at time point: %f", page, changeTimes[page]);
    } else {
        g_warning("Trying to update non-existing page %zu when there are only %zu pages", page, changeTimes.size());
    }
}

auto PluginNotifier::getLastPageChangeTimes() -> std::vector<double> { return this->changeTimes; }

auto PluginNotifier::getLastDocumentChangeTime() -> double { return this->documentChangeTime; }