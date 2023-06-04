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
    void createPopover();
    std::string toLinkMarkup(std::string url);
    void positionPopover(XojPageView* pageView, Link* element, GtkPopover* popover);

private:
    XournalView* view;
    Control* control;
    GtkWidget* documentWidget;
    Link* linkElement = nullptr;

    GtkPopover* linkPopoverHighlight = nullptr;
    GtkWidget* linkPopoverLabelHightlight = nullptr;
    GtkPopover* linkPopoverSelect = nullptr;
    GtkWidget* linkPopoverLabelSelect = nullptr;
    Link* highlightedLink = nullptr;
    Link* selectedLink = nullptr;
    static constexpr int POPOVER_PADDING = 2;
};
