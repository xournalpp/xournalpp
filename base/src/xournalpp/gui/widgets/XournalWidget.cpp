#include "XournalWidget.h"

#include <utility>

/*
 * GtkAdjustment value specifies top left corner!
 */

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
    if (this->get_hadjustment()) {
        this->get_hadjustment()->signal_value_changed().connect(sigc::mem_fun(this, &XournalWidget::hScroll));
        this->scrollX.watch([&](auto&&, auto&& x) { updateScrollbar(this->get_hadjustment(), x); });
        if (docMode.get() == Settings::DocumentMode::INFINITE) {
            this->get_hadjustment()->set_lower(-10.0);
            this->get_hadjustment()->set_upper(20.0);
            this->get_hadjustment()->set_page_size(10.0);
        }
    }
}

auto XournalWidget::initVScrollbar() -> void {
    if (this->get_vadjustment()) {
        this->get_vadjustment()->signal_value_changed().connect(sigc::mem_fun(this, &XournalWidget::vScroll));
        this->scrollY.watch([&](auto&&, auto&& y) { updateScrollbar(this->get_vadjustment(), y); });
        if (docMode.get() == Settings::DocumentMode::INFINITE) {
            this->get_vadjustment()->set_lower(-10.0);
            this->get_vadjustment()->set_upper(20.0);
            this->get_vadjustment()->set_page_size(10.0);
        }
    }
}

auto XournalWidget::updateScrollbar(const Glib::RefPtr<Gtk::Adjustment>& adj, double value) -> void {
    if (docMode.get() == Settings::DocumentMode::INFINITE) {
        double upper = adj->get_upper();
        double lower = adj->get_lower();
        double fullRange = adj->get_page_size();
        double lowerThreshhold = lower + 0.1 * fullRange;
        double upperThreshhold = upper - fullRange - 0.1 * fullRange;
        if (value < lowerThreshhold) {
            adj->set_lower(lower - fullRange * 0.5);
        } else if (value > upperThreshhold) {
            adj->set_upper(upper + fullRange * 0.5);
        }
    }
}

bool XournalWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) { return Widget::on_draw(cr); }

void XournalWidget::on_size_allocate(Gtk::Allocation& allocation) {
    if (allocation.get_width() != this->viewport->width || allocation.get_height() != this->viewport->height)
        this->context.dispatch(Resize{allocation.get_width(), allocation.get_height()});

    auto hadjustment = this->get_hadjustment();
    auto vadjustment = this->get_vadjustment();

    if (this->docMode.get() != Settings::INFINITE) {
        vadjustment->set_lower(0.0);
        hadjustment->set_lower(0.0);
        // gtk_adjustment_set_upper(vadjustment, self->layout->documentHeight * self->viewport->rawScale);
        // gtk_adjustment_set_upper(hadjustment, self->layout->documentWidth * self->viewport->rawScale);
        hadjustment->set_page_size(allocation.get_width());
        vadjustment->set_page_size(allocation.get_height());
        hadjustment->set_page_increment(allocation.get_width() - STEP_INCREMENT);
        vadjustment->set_page_increment(allocation.get_height() - STEP_INCREMENT);
    }


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
