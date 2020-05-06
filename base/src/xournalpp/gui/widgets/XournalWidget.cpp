#include "XournalWidget.h"

#include <cmath>
#include <utility>

#include <gdk/gdk.h>

enum Direction { HORIZONTAL, VERTICAL };

XournalWidget::XournalWidget(const lager::reader<Settings>& settings, lager::reader<Viewport> viewport,
                             lager::context<ViewportAction> context):
        Glib::ObjectBase(typeid(XournalWidget)), context(std::move(context)), viewport(std::move(viewport)) {
    this->set_vexpand();
    this->set_hexpand();
    this->property_hadjustment().signal_changed().connect(sigc::mem_fun(this, &XournalWidget::initHScrollbar));
    this->property_vadjustment().signal_changed().connect(sigc::mem_fun(this, &XournalWidget::initVScrollbar));

    this->docMode = settings[&Settings::mode];
    this->docMode.watch([](auto&&, auto&& mode) {
        /*
         * TODO
         * exchange DrawingManager, reallocate
         */
    });
    this->scrollX = this->viewport[&Viewport::x];
    this->scrollY = this->viewport[&Viewport::y];

    this->rawScale = this->viewport[&Viewport::rawScale];
    this->rawScale.watch([&](auto&&, auto&& v) { this->queue_allocate(); });
    this->rawScale.get();
}

auto XournalWidget::initHScrollbar() -> void {
    if (this->property_hadjustment().get_value()) {
        this->property_hadjustment().get_value()->signal_value_changed().connect(
                sigc::mem_fun(this, &XournalWidget::hScroll));
        this->scrollX.watch([&](auto&&, auto&& x) { updateScrollbar(this->property_hadjustment().get_value(), x); });
    }
}

auto XournalWidget::initVScrollbar() -> void {
    if (this->property_vadjustment().get_value()) {
        this->property_vadjustment().get_value()->signal_value_changed().connect(
                sigc::mem_fun(this, &XournalWidget::vScroll));
        this->scrollY.watch([&](auto&&, auto&& y) { updateScrollbar(this->property_vadjustment().get_value(), y); });
    }
}

auto XournalWidget::updateScrollbar(Glib::RefPtr<Gtk::Adjustment> adj, double value) -> void {
    if (docMode.get() == Settings::DocumentMode::INFINITE) {
        double upper = adj->get_upper();
        double lower = adj->get_lower();
        double fullRange = upper - lower;
        double lowerThreshhold = lower + 0.1 * fullRange;
        double upperThreshhold = upper - 0.1 * fullRange;
        if (value < lowerThreshhold) {
            adj->set_lower(lower - 0.2 * fullRange);
            adj->set_upper(upper - 0.2 * fullRange);
        } else if (value > upperThreshhold) {
            adj->set_lower(lower + 0.2 * fullRange);
            adj->set_upper(upper + 0.2 * fullRange);
        }
    }
}

bool XournalWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) { return Widget::on_draw(cr); }

void XournalWidget::on_size_allocate(Gtk::Allocation& allocation) {
    if (allocation.get_width() != this->viewport->width || allocation.get_height() != this->viewport->height)
        this->context.dispatch(Resize{allocation.get_width(), allocation.get_height()});

    auto hadjustment = this->get_hadjustment();
    auto vadjustment = this->get_vadjustment();

    if (this->docMode.get() == Settings::INFINITE) {
        vadjustment->set_lower(-1.5 * allocation.get_height());
        hadjustment->set_lower(-1.5 * allocation.get_width());
        vadjustment->set_upper(1.5 * allocation.get_height());
        hadjustment->set_upper(1.5 * allocation.get_width());
    } else {
        vadjustment->set_lower(0.0);
        hadjustment->set_lower(0.0);
        // gtk_adjustment_set_upper(vadjustment, self->layout->documentHeight * self->viewport->rawScale);
        // gtk_adjustment_set_upper(hadjustment, self->layout->documentWidth * self->viewport->rawScale);
    }
    hadjustment->set_page_size(allocation.get_width());
    vadjustment->set_page_size(allocation.get_height());
    hadjustment->set_page_increment(allocation.get_width() - STEP_INCREMENT);
    vadjustment->set_page_increment(allocation.get_height() - STEP_INCREMENT);

    // gtk_widget_queue_draw(drawingArea); TODO?
}

void XournalWidget::on_realize() { Widget::on_realize(); }

auto XournalWidget::hScroll() -> void {
    double xDiff = this->get_hadjustment()->get_value();
    this->context.dispatch(Scroll{Scroll::HORIZONTAL, xDiff});
}

auto XournalWidget::vScroll() -> void {
    double xDiff = this->get_vadjustment()->get_value();
    this->context.dispatch(Scroll{Scroll::VERTICAL, xDiff});
}
