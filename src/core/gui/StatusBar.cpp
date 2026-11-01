#include "StatusBar.h"

#include "gui/MainWindow.h"
#include "util/Util.h"

static constexpr auto STATUS_BAR_ID = "statusbar";
static constexpr auto STATUS_LABEL_ID = "lbState";
static constexpr auto PROGRESS_BAR_ID = "pgState";

StatusBar::StatusBar(MainWindow& parent):
        statusbar(parent.get(STATUS_BAR_ID), xoj::util::ref),
        statusLabel(GTK_LABEL(parent.get(STATUS_LABEL_ID)), xoj::util::ref),
        progressbar(GTK_PROGRESS_BAR(parent.get(PROGRESS_BAR_ID)), xoj::util::ref),
        maxState(100) {}
StatusBar::~StatusBar() {}

void StatusBar::setMaximumState(size_t max) { this->maxState = max; }

void StatusBar::setCurrentState(size_t state) {
    Util::execInUiThread([state, this]() {
        gtk_progress_bar_set_fraction(this->progressbar.get(),
                                      static_cast<gdouble>(state) / static_cast<gdouble>(this->maxState));
    });
}

void StatusBar::setStateLabel(std::string_view lbl) { gtk_label_set_text(this->statusLabel.get(), lbl.data()); }

void StatusBar::setVisible(bool visible) { gtk_widget_set_visible(this->statusbar.get(), visible); }
