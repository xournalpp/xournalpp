/*
 * Xournal++
 *
 * Control to callibrate the zoom to fit the display DPI
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#pragma once

#include <vector>

#include <glib-object.h>  // for G_TYPE_CHECK_INSTANCE_TYPE, G_TYPE_CHECK_IN...
#include <glib.h>         // for gint, G_BEGIN_DECLS, G_END_DECLS
#include <gtk/gtk.h>      // for GtkWidget, GtkWidgetClass

#include "gui/toolbarMenubar/ToolbarSide.h"
#include "util/raii/GObjectSPtr.h"

struct _ToolbarBox;
struct _ToolbarBoxClass;

G_BEGIN_DECLS

#define TOOLBAR_BOX(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, toolbarbox_get_type(), _ToolbarBox)
#define TOOLBAR_BOX_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, toolbarbox_get_type(), _ToolbarBoxClass)
#define IS_TOOLBAR_BOX(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, toolbarbox_get_type())

GType toolbarbox_get_type(void);

G_END_DECLS

class ToolbarBox final {
public:
    ToolbarBox(ToolbarSide side, int spacing = 2);
    /// Create a toolbar replacing the placeholder
    ToolbarBox(GtkWidget* placeholder);
    ~ToolbarBox();

    GtkWidget* getWidget() const;
    ToolbarSide getSide() const;

    bool empty() const;
    void clear();

    /**
     * @param widget The button in the toolbar
     * @param proxy Proxy menu item in case of overflow
     */
    void append(GtkWidget* widget, GtkWidget* proxy);

public:
    void measure(GtkOrientation orientation, int for_size, int* minimum, int* natural, int* minimum_baseline,
                 int* natural_baseline) const;
    void size_allocate(int width, int height, int baseline);
    void snapshot(GtkSnapshot* sn) const;

private:
    class Child final {
    public:
        Child(GtkWidget* widget, GtkWidget* proxy);
        ~Child();
        Child(const Child&) = delete;
        Child& operator=(const Child&) = delete;
        Child(Child&&);
        Child& operator=(Child&&);
        GtkWidget* widget = nullptr;
        GtkWidget* proxy = nullptr;
    };

    xoj::util::GObjectSPtr<_ToolbarBox> widget;
    std::vector<Child> children;
    xoj::util::WidgetSPtr overflowMenuButton;
    GtkBox* popoverBox;

    ptrdiff_t visibleChildren = 0;
    int spacing;       ///< Spacing between children
    ToolbarSide side;  ///< Side of the window the toolbar is on
};
