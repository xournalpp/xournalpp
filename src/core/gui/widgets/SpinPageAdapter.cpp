#include "SpinPageAdapter.h"

#include <algorithm>  // for min
#include <cstdint>  // for uint64_t
#include <string>   // for string
#include <utility>  // for swap

#include <glib-object.h>  // for g_signal_handler_disconnect, G_CALLBACK

#include "util/Assert.h"      // for xoj_assert
#include "util/glib_casts.h"  // for wrap_for_once_v
#include "util/safe_casts.h"

auto SpinPageAdapter::pageNrSpinChangedTimerCallback(SpinPageAdapter* adapter) -> bool {
    adapter->page = strict_cast<size_t>(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(adapter->widget.get())));

    adapter->firePageChanged();

    adapter->timeout.consume();
    return false;
}

void SpinPageAdapter::pageNrSpinChangedCallback(GtkSpinButton* spinbutton, SpinPageAdapter* adapter) {
    // Nothing changed.
    if (as_unsigned(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton))) == adapter->page) {
        return;
    }

    // Give the spin button some time to release, if we don't do he will send new events...
    adapter->timeout = g_timeout_add(100, xoj::util::wrap_for_once_v<pageNrSpinChangedTimerCallback>, adapter);
}

void SpinPageAdapter::setWidget(GtkWidget* widget) {
    if (this->widget && this->pageNrSpinChangedHandlerId) {
        g_signal_handler_disconnect(this->widget.get(), this->pageNrSpinChangedHandlerId);
        g_signal_handler_disconnect(this->widget.get(), this->outputHandlerId);
        g_signal_handler_disconnect(this->widget.get(), this->inputHandlerId);
        this->outputHandlerId = 0;
        this->inputHandlerId = 0;
    }
    xoj_assert(widget);
    this->widget.reset(widget, xoj::util::refsink);
    this->pageNrSpinChangedHandlerId =
            g_signal_connect(widget, "value-changed", G_CALLBACK(pageNrSpinChangedCallback), this);
    this->outputHandlerId = g_signal_connect(widget, "output", G_CALLBACK(spinOutputCallback), this);
    this->inputHandlerId = g_signal_connect(widget, "input", G_CALLBACK(spinInputCallback), this);

    gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget), static_cast<double>(this->min), static_cast<double>(this->max));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), static_cast<double>(this->page));
    if (!this->labels.empty()) {
        gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(widget), FALSE);
    }
}

auto SpinPageAdapter::getPage() const -> size_t { return this->page; }

void SpinPageAdapter::setPage(size_t page) {
    this->page = page;
    if (this->widget) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(this->widget.get()), static_cast<double>(page));
    }
}

void SpinPageAdapter::setMinMaxPage(size_t min, size_t max) {
    this->min = min;
    this->max = max;
    if (this->widget) {
        gtk_spin_button_set_range(GTK_SPIN_BUTTON(this->widget.get()), static_cast<double>(min),
                                  static_cast<double>(max));
    }
}

void SpinPageAdapter::addListener(SpinPageListener* listener) { this->listener = listener; }

void SpinPageAdapter::removeListener(SpinPageListener* listener) {
    xoj_assert(listener == this->listener);
    this->listener = nullptr;
}

void SpinPageAdapter::setLabels(std::vector<std::string> newLabels) {
    this->labels = std::move(newLabels);
    if (this->widget) {
        gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(this->widget.get()), this->labels.empty() ? TRUE : FALSE);
        gboolean did_output = FALSE;
        g_signal_emit_by_name(this->widget.get(), "output", &did_output);
    }
}

void SpinPageAdapter::insertLabel(size_t pos, std::string label) {
    if (this->labels.empty()) {
        if (label.empty()) {
            return;
        }
        this->labels.resize(this->max);
    }
    if (this->labels.empty()) {
        return;
    }
    this->labels.insert(this->labels.begin() + std::min(pos, this->labels.size()), std::move(label));
}

void SpinPageAdapter::deleteLabel(size_t pos) {
    if (this->labels.empty() || pos >= this->labels.size()) {
        return;
    }
    this->labels.erase(this->labels.begin() + pos);
}

void SpinPageAdapter::swapLabels(size_t a, size_t b) {
    if (this->labels.empty() || a >= this->labels.size() || b >= this->labels.size()) {
        return;
    }
    std::swap(this->labels[a], this->labels[b]);
}

auto SpinPageAdapter::hasAnyLabels() const -> bool {
    for (const auto& lbl: this->labels) {
        if (!lbl.empty()) {
            return true;
        }
    }
    return false;
}

gboolean SpinPageAdapter::spinOutputCallback(GtkSpinButton* spin, SpinPageAdapter* adapter) {
    if (adapter->labels.empty()) {
        return FALSE;
    }
    auto idx = static_cast<size_t>(gtk_spin_button_get_value(spin));
    if (idx >= 1 && idx <= adapter->labels.size() && !adapter->labels[idx - 1].empty()) {
        gtk_entry_set_text(GTK_ENTRY(spin), adapter->labels[idx - 1].c_str());
        return TRUE;
    }
    return FALSE;
}

gint SpinPageAdapter::spinInputCallback(GtkSpinButton* spin, gdouble* newValue, SpinPageAdapter* adapter) {
    if (adapter->labels.empty()) {
        return FALSE;
    }
    const std::string input(gtk_entry_get_text(GTK_ENTRY(spin)));

    const size_t labelCount = adapter->labels.size();
    const size_t startIndex = labelCount == 0 ? 0 : adapter->page % labelCount;
    for (size_t offset = 0; offset < labelCount; ++offset) {
        const size_t idx = (startIndex + offset) % labelCount;
        if (adapter->labels[idx] == input) {
            *newValue = static_cast<gdouble>(idx + 1);
            return TRUE;
        }
    }

    *newValue = static_cast<gdouble>(adapter->page);
    return TRUE;
}

void SpinPageAdapter::firePageChanged() { this->listener->pageChanged(this->page); }

SpinPageListener::~SpinPageListener() = default;
