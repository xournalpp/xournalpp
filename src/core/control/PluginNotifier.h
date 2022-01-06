/*
 * Xournal++
 *
 * Controls the zoom level
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>

#include <gtk/gtk.h>

#include "model/DocumentListener.h"

#include "Control.h"

class DocumentListener;

class PluginNotifier: public DocumentListener {
public:
    PluginNotifier(Control* control);
    virtual ~PluginNotifier() = default;

    void documentChanged(DocumentChangeType type) override;
    void pageSizeChanged(size_t page) override;
    void pageChanged(size_t page) override;
    void pageInserted(size_t page) override;
    void pageDeleted(size_t page) override;
    void pageSelected(size_t page) override;

public:
    std::vector<double> getLastPageChangeTimes();
    double getLastDocumentChangeTime();

private:
    Control* control;
    std::vector<double> changeTimes = {};
    double documentChangeTime = NAN;
    std::chrono::steady_clock::time_point startTime;
    void updateChangeTime(size_t page);
};
