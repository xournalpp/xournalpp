/*
 * Xournal++
 *
 * Link editor gui (for Link Tool)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#include <string>

#include <gdk/gdk.h>  // for GdkEventKey
#include <gtk/gtk.h>  // for GtkIMContext, GtkTextIter, GtkWidget

#include "control/zoom/ZoomListener.h"  // for ZoomListener
#include "model/Link.h"                 // for Link
#include "model/PageRef.h"              // for PageRef


class Control;
class XournalView;
class XojPageView;

class LinkPopover {
public:
    LinkPopover(XournalView* view);
    ~LinkPopover();
    void hide();
    void show();
    void show_all();
    void popup();
    void popdown();
    void updateLabel(bool markup);
    void positionPopover();
    bool hasLink();

    void linkTo(Link* link);
    inline GtkPopover* getPopover() const { return this->popover; }
    inline Link* getLink() const { return this->link; }

private:
    XournalView* view;

    GtkPopover* popover = nullptr;
    GtkLabel* label = nullptr;
    Link* link = nullptr;

    static constexpr int POPOVER_PADDING = 2;
};

class LinkEditor: public ZoomListener {
public:
    LinkEditor(XournalView* view);
    ~LinkEditor();

    /* Called on mouse double click event
     * -> Opens the link editor dialog, or tries to open the link when controlDown = CTRL */
    void startEditing(const PageRef& page, const int x, const int y);

    /* Called on mouse single click event
     * -> Make link permanently selected (red border + popover), or tries to open the link when contrlDown = CTRL */
    void select(const PageRef& page, const int x, const int y, const bool controlDown, XojPageView* pageView);

    /** Called on mouse move / hover event
     * -> Make link temporary highlighted (red border + popover) as long mouse above element */
    void highlight(const PageRef& page, const int x, const int y, XojPageView* pageView);

    void zoomChanged() override;

private:
    XournalView* view;
    Control* control;
    Link* linkElement = nullptr;

    // There is always at most one selected link and one highlighted link.
    // Both can be present at the same time, when one link being selected another one is highlighted.
    std::unique_ptr<LinkPopover> highlightPopover;
    std::unique_ptr<LinkPopover> selectPopover;
};
