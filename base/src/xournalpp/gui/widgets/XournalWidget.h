/*
 * Xournal++
 *
 * Xournal widget which is the "View" widget
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

#include <gtkmm.h>
#include <lager/reader.hpp>
#include <xournalpp/settings/Settings.h>
#include <xournalpp/view/Viewport.h>


class XournalWidget: public virtual Gtk::DrawingArea, public virtual Gtk::Scrollable {
public:
    XournalWidget(const lager::reader<Settings>& settings, lager::reader<Viewport> viewport,
                  lager::context<ViewportAction> context);

private:
    auto initHScrollbar() -> void;
    auto initVScrollbar() -> void;
    auto updateScrollbar(Glib::RefPtr<Gtk::Adjustment> adj, double value) -> void;

    auto hScroll() -> void;
    auto vScroll() -> void;

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    void on_size_allocate(Gtk::Allocation& allocation) override;
    void on_realize() override;

private:
    /** State */
    lager::reader<Viewport> viewport;
    lager::reader<double> scrollX;
    lager::reader<double> scrollY;
    lager::reader<double> rawScale;
    lager::reader<Settings::DocumentMode> docMode;
    lager::context<ViewportAction> context;

    /*
     * References to library state, document stuff etc.
     */


    constexpr static double STEP_INCREMENT = 10;
};
