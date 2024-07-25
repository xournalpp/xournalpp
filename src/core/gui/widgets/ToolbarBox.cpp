#include "ToolbarBox.h"

#include <algorithm>

#include "control/Control.h"
#include "gui/MainWindow.h"
#include "gui/dialog/toolbarCustomize/ToolItemGType.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include "gui/toolbarMenubar/model/ToolbarEntry.h"
#include "util/Assert.h"
#include "util/raii/GObjectSPtr.h"
#include "util/safe_casts.h"

G_BEGIN_DECLS
struct _ToolbarBox {
    GtkWidget widget;
    ToolbarBox* parent;
};

struct _ToolbarBoxClass {
    GtkWidgetClass parent_class;
};
G_END_DECLS

G_DEFINE_TYPE(_ToolbarBox, toolbarbox, GTK_TYPE_WIDGET);  // NOLINT // @suppress("Unused static function")

template <auto f, class... Args>
static void wrap(GtkWidget* w, Args... args) {
    (TOOLBAR_BOX(w)->parent->*f)(std::forward<Args>(args)...);
}

static void toolbarbox_class_init(_ToolbarBoxClass* klass) {
    GtkWidgetClass* widget_class = nullptr;

    widget_class = reinterpret_cast<GtkWidgetClass*>(klass);

    widget_class->snapshot = wrap<&ToolbarBox::snapshot>;
    widget_class->measure = wrap<&ToolbarBox::measure>;
    widget_class->size_allocate = wrap<&ToolbarBox::size_allocate>;

    gtk_widget_class_set_css_name(widget_class, "box");  // We want to catch themes tuning up box.toolbar
}

static void toolbarbox_init(_ToolbarBox* w) {
    w->parent = nullptr;
    gtk_widget_add_css_class(GTK_WIDGET(w), "toolbar");  // We want to catch themes tuning up box.toolbar
    gtk_widget_set_overflow(GTK_WIDGET(w), GTK_OVERFLOW_HIDDEN);
}

class ToolbarBox::Child final {
public:
    Child(xoj::util::WidgetSPtr widget, xoj::util::WidgetSPtr proxy): widget(widget), proxy(proxy) {}
    ~Child() {
        if (widget) {
            gtk_widget_unparent(widget.get());
        }
    }
    Child(const Child&) = delete;
    Child& operator=(const Child&) = delete;
    Child(Child&& o): widget(std::exchange(o.widget, nullptr)), proxy(std::exchange(o.proxy, nullptr)) {}
    Child& operator=(Child&& o) {
        /**
         * We should really do the following here
         *   // if (widget) {
         *   //     gtk_widget_unparent(widget.get());
         *   // }
         * but this causes a SegFault in gtk (cf https://gitlab.gnome.org/GNOME/gtk/-/issues/6699)
         * For now, leave this "memleak". This causes messages on exit:
         * "Finalizing _ToolbarBox 0x5a7499d1cfe0, but it still has children left: ..."
         */
        widget = std::exchange(o.widget, nullptr);
        proxy = std::exchange(o.proxy, nullptr);
        return *this;
    }

    xoj::util::WidgetSPtr widget = nullptr;
    xoj::util::WidgetSPtr proxy = nullptr;
};

/// Data required when editing the toolbar
struct ToolbarBox::EditingData {
    ToolMenuHandler* handler;
    ToolbarData* toolbarData;
    const char* toolbarName;  ///< Name in the config file

    /// Item currently being dragged over the toolbar
    xoj::util::WidgetSPtr tempItem;
    /// Index of the dragged item in the children vector
    ptrdiff_t tempItemPosition = -1;

    // DND event controllers - To be removed once editing is over
    xoj::util::GObjectSPtr<GtkDropTarget> target;  ///< Used for receiving items
    xoj::util::GObjectSPtr<GtkDragSource> source;  ///< Used for removing/moving items

    // Cached positioning info - for tempItem positioning or dragging items out of the toolbar
    /// The x (if horizontal) or y (if vertical) positions of the center of the visible children (in widget coords)
    std::vector<int> childrenCenter;
    /// The x (if horizontal) or y (if vertical) positions of the end of the visible children (in widget coords)
    std::vector<int> childrenEnd;

    bool rightToLeft;  ///< true if horizontal and in RTL language
};

ToolbarBox::ToolbarBox(const char* name, ToolbarSide side, int spacing):
        widget(TOOLBAR_BOX(g_object_new(toolbarbox_get_type(), nullptr)), xoj::util::adopt),
        overflowMenuButton(gtk_menu_button_new(), xoj::util::adopt),
        spacing(spacing),
        side(side),
        name(name) {
    widget->parent = this;
    gtk_widget_set_parent(overflowMenuButton.get(), getWidget());
    gtk_widget_set_can_focus(getWidget(), false);

    GtkWidget* popover = gtk_popover_new();
    gtk_widget_add_css_class(popover, "menu");
    gtk_popover_set_has_arrow(GTK_POPOVER(popover), false);
    gtk_widget_set_halign(popover, GTK_ALIGN_END);
    gtk_widget_set_valign(popover, GTK_ALIGN_END);

    gtk_menu_button_set_popover(GTK_MENU_BUTTON(overflowMenuButton.get()), popover);
    setMenuButtonDirection(GTK_MENU_BUTTON(overflowMenuButton.get()), side);
    gtk_widget_add_css_class(overflowMenuButton.get(), "overflow");

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    popoverBox = GTK_BOX(box);
    gtk_popover_set_child(GTK_POPOVER(popover), box);
}

static ToolbarSide fetchFromPlaceholder(GtkWidget* placeholder) {
    xoj_assert(GTK_IS_MENU_BUTTON(placeholder));
    switch (gtk_menu_button_get_direction(GTK_MENU_BUTTON(placeholder))) {
        case GTK_ARROW_DOWN:
            return ToolbarSide::TOP;
        case GTK_ARROW_UP:
            return ToolbarSide::BOTTOM;
        case GTK_ARROW_LEFT:
            return ToolbarSide::START;
        case GTK_ARROW_RIGHT:
            return ToolbarSide::END;
        default:
            g_warning("ToolbarBox::fetchFromPlaceholder() called on widget with GTK_ARROW_NONE");
            return ToolbarSide::TOP;
    }
}

ToolbarBox::ToolbarBox(const char* name, GtkWidget* placeholder): ToolbarBox(name, fetchFromPlaceholder(placeholder)) {
    xoj_assert(GTK_IS_BOX(gtk_widget_get_parent(placeholder)));
    GtkBox* parent = GTK_BOX(gtk_widget_get_parent(placeholder));
    gtk_box_insert_child_after(parent, getWidget(), placeholder);
    gtk_box_remove(parent, placeholder);
}

ToolbarBox::~ToolbarBox() { gtk_widget_unparent(overflowMenuButton.get()); }

GtkWidget* ToolbarBox::getWidget() const {
    xoj_assert(GTK_IS_WIDGET(widget.get()));
    return reinterpret_cast<GtkWidget*>(widget.get());
}

ToolbarSide ToolbarBox::getSide() const { return side; }

bool ToolbarBox::empty() const { return children.empty(); }

void ToolbarBox::clear() {
    children.clear();
    visibleChildren = 0;

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    popoverBox = GTK_BOX(box);
    gtk_popover_set_child(gtk_menu_button_get_popover(GTK_MENU_BUTTON(overflowMenuButton.get())), box);
}

void ToolbarBox::reserve(size_t n) { children.reserve(n); }

void ToolbarBox::append(xoj::util::WidgetSPtr w, xoj::util::WidgetSPtr proxy) {
    gtk_widget_set_parent(w.get(), getWidget());
    if (proxy) {
        gtk_widget_add_css_class(proxy.get(), "model");
        gtk_box_append(popoverBox, proxy.get());
    }
    this->children.emplace_back(std::move(w), std::move(proxy));
}

inline static void setIfNonNull(int* p, int value) {
    if (p) {
        *p = value;
    }
}

void ToolbarBox::measure(GtkOrientation orientation, int for_size, int* minimum, int* natural, int* minimum_baseline,
                         int* natural_baseline) const {
    setIfNonNull(minimum_baseline, -1);
    setIfNonNull(natural_baseline, -1);

    if (children.empty()) {
        setIfNonNull(minimum, 0);
        setIfNonNull(natural, 0);
        return;
    }

    if (orientation == to_Orientation(this->side)) {
        int min = 0;
        int nat = 0;
        for (const auto& c: children) {
            int minChild, natChild;
            gtk_widget_measure(c.widget.get(), orientation, -1, &minChild, &natChild, nullptr, nullptr);
            min += minChild;
            nat += natChild;
        }
        int spacings = strict_cast<int>(children.size() - 1) * spacing;

        // Ask for at least a third of the minimum. Overflow will go to the overflow menu
        setIfNonNull(minimum, (min + spacings) / 3);
        setIfNonNull(natural, nat + spacings);
    } else {
        int min = 0;
        int nat = 0;
        for (const auto& c: children) {
            int minChild, natChild;
            gtk_widget_measure(c.widget.get(), orientation, -1, &minChild, &natChild, nullptr, nullptr);
            min = std::max(min, minChild);
            nat = std::max(nat, natChild);
        }
        setIfNonNull(minimum, min);
        setIfNonNull(natural, nat);
    }
}

void ToolbarBox::size_allocate(int width, int height, int baseline) {
    auto orientation = to_Orientation(this->side);

    int staticDim = orientation == GTK_ORIENTATION_HORIZONTAL ? height : width;
    int dynamicDim = orientation == GTK_ORIENTATION_HORIZONTAL ? width : height;

    int min = -spacing;
    int nbExpanders = 0;
    std::vector<GtkRequestedSize> sizes;
    sizes.reserve(children.size());
    for (const auto& c: children) {
        auto& s = sizes.emplace_back(GtkRequestedSize{c.widget.get(), 0, 0});
        gtk_widget_measure(c.widget.get(), orientation, staticDim, &(s.minimum_size), &(s.natural_size), nullptr,
                           nullptr);
        min += s.minimum_size + spacing;

        if (gtk_widget_compute_expand(c.widget.get(), orientation)) {
            nbExpanders++;
        }

        if (min > dynamicDim) {
            // An overflow menu is required. We don't need to know the size of the items left
            break;
        }
    }

    const bool overflow = dynamicDim < min;
    GtkRequestedSize sizeMenuButton = {overflowMenuButton.get(), 0, 0};

    // The leftover space will be distributed amongst expanders
    int leftoverSpace = 0;

    if (!overflow) {
        // We have enough room to put all our widgets. Distribute the extra space between them.
        gtk_distribute_natural_allocation(dynamicDim - min, strict_cast<guint>(sizes.size()), sizes.data());
        gtk_widget_set_visible(overflowMenuButton.get(), false);
        visibleChildren = as_signed(children.size());
        leftoverSpace = dynamicDim - min;
    } else {
        gtk_widget_set_visible(overflowMenuButton.get(), true);
        gtk_widget_measure(overflowMenuButton.get(), orientation, staticDim, &(sizeMenuButton.minimum_size),
                           &(sizeMenuButton.natural_size), nullptr, nullptr);
        min += sizeMenuButton.minimum_size + spacing;

        // Remove last items so the overflow menu button fits
        while (min > dynamicDim && !sizes.empty()) {
            min -= sizes.back().minimum_size + spacing;
            if (gtk_widget_compute_expand(GTK_WIDGET(sizes.back().data), orientation)) {
                nbExpanders--;
            }
            sizes.pop_back();
        }

        // The Toolbar will only show the elements corresponding to an entry of `sizes`
        visibleChildren = as_signed(sizes.size());
        if (sizes.empty()) {
            g_warning(" ToolbarBox::size_allocate(): We don't have room for even one item. Abort!");
            return;
        }

        sizes.emplace_back(sizeMenuButton);
        leftoverSpace =
                gtk_distribute_natural_allocation(dynamicDim - min, strict_cast<guint>(sizes.size()), sizes.data());
        sizeMenuButton = sizes.back();
        sizes.pop_back();
    }

    int pixelsPerExpander = nbExpanders == 0 ? 0 : leftoverSpace / nbExpanders;
    int extraPixels = nbExpanders == 0 ? 0 : leftoverSpace % nbExpanders;  // Distributed 1 by 1 to the first expanders
    auto getSize = [&](const GtkRequestedSize& s) {
        return s.minimum_size + (gtk_widget_compute_expand(GTK_WIDGET(s.data), orientation) ?
                                         pixelsPerExpander + (extraPixels-- > 0 ? 1 : 0) :
                                         0);
    };

    if (editingData) {
        editingData->childrenCenter.clear();
        editingData->childrenEnd.clear();
    }
    auto recordSize = [&](int size, int pos, gpointer widget, bool rightToLeft = false) {
        if (editingData && widget && !gtk_widget_has_css_class(GTK_WIDGET(widget), "phantom")) {
            editingData->childrenCenter.emplace_back(pos + size / 2);
            editingData->childrenEnd.emplace_back(rightToLeft ? pos : pos + size);
        }
    };

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        if (gtk_widget_get_direction(getWidget()) == GTK_TEXT_DIR_LTR) {
            auto p = GRAPHENE_POINT_INIT_ZERO;
            int x = 0;
            for (const auto& s: sizes) {
                int size = getSize(s);
                recordSize(size, x, s.data);
                gtk_widget_allocate(GTK_WIDGET(s.data), size, height, -1, gsk_transform_translate(nullptr, &p));
                x += size + spacing;          // Sum as int to avoid rounding errors
                p.x = static_cast<float>(x);  // Shift the next widget
            }
            if (overflow) {
                // Flush the overflow button to the right
                p.x = static_cast<float>(width - sizeMenuButton.minimum_size);
                gtk_widget_allocate(overflowMenuButton.get(), sizeMenuButton.minimum_size, height, -1,
                                    gsk_transform_translate(nullptr, &p));
            }
        } else {  // Right-to-Left languages need the symmetric allocation
            auto p = GRAPHENE_POINT_INIT_ZERO;
            int x = width + spacing;  // The extra spacing will get substracted on first iteration
            for (const auto& s: sizes) {
                int size = getSize(s);
                x -= size + spacing;  // Sum as int to avoid rounding errors
                p.x = static_cast<float>(x);
                recordSize(size, x, s.data, true);
                gtk_widget_allocate(GTK_WIDGET(s.data), size, height, -1, gsk_transform_translate(nullptr, &p));
            }
            if (overflow) {
                // Flush the overflow button to the left
                gtk_widget_allocate(overflowMenuButton.get(), sizeMenuButton.minimum_size, height, -1, nullptr);
            }
        }
    } else {
        auto p = GRAPHENE_POINT_INIT_ZERO;
        int y = 0;
        for (const auto& s: sizes) {
            int size = getSize(s);
            recordSize(size, y, s.data);
            gtk_widget_allocate(GTK_WIDGET(s.data), width, size, -1, gsk_transform_translate(nullptr, &p));
            y += size + spacing;          // Sum as int to avoid rounding errors
            p.y = static_cast<float>(y);  // Shift the next widget
        }
        if (overflow) {
            // Flush the overflow button to the bottom
            p.y = static_cast<float>(height - sizeMenuButton.minimum_size);
            gtk_widget_allocate(overflowMenuButton.get(), width, sizeMenuButton.minimum_size, -1,
                                gsk_transform_translate(nullptr, &p));
        }
    }

    for (auto it = children.begin(); it < std::next(children.begin(), visibleChildren); ++it) {
        if (it->proxy) {
            gtk_widget_set_visible(it->proxy.get(), false);
        }
    }
    bool proxies = false;
    for (auto it = std::next(children.begin(), visibleChildren); it < children.end(); ++it) {
        // Move the overflowed widgets out of sight.
        // Using gtk_widget_set_visible(w, false) does not work: it causes the gtk_widget_measure to return 0, causing
        // bugs upon resizing
        // Todo(anyone): find a better solution
        auto p = GRAPHENE_POINT_INIT(-100000.f, 0.f);
        gtk_widget_allocate(it->widget.get(), 100, 100, -1, gsk_transform_translate(nullptr, &p));

        if (it->proxy) {
            gtk_widget_set_visible(it->proxy.get(), true);
            proxies = true;
        }
    }
    gtk_widget_set_sensitive(overflowMenuButton.get(), proxies);
}

void ToolbarBox::snapshot(GtkSnapshot* sn) const {
    for (auto it = children.begin(); it < std::next(children.begin(), visibleChildren); ++it) {
        gtk_widget_snapshot_child(getWidget(), it->widget.get(), sn);
    }
    if (gtk_widget_get_visible(overflowMenuButton.get())) {
        gtk_widget_snapshot_child(getWidget(), overflowMenuButton.get(), sn);
    }
}

void ToolbarBox::startEditing(ToolMenuHandler* handler) {
    this->editingData = std::make_unique<EditingData>();
    editingData->handler = handler;

    editingData->rightToLeft = to_Orientation(side) == GTK_ORIENTATION_HORIZONTAL &&
                               gtk_widget_get_direction(getWidget()) == GTK_TEXT_DIR_RTL;
    gtk_widget_add_css_class(getWidget(), "editing");
    gtk_widget_set_visible(getWidget(), true);  // Empty toolbar might be hidden

    // prepare drag & drop
    auto* target =
            gtk_drop_target_new(xoj::dnd::get_tool_item_gtype(), GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));
    xoj_assert(target);

    constexpr auto getInsertPosition = +[](ToolbarSide side, double x, double y, EditingData& data) {
        double pos = to_Orientation(side) == GTK_ORIENTATION_HORIZONTAL ? x : y;
        if (data.rightToLeft) {
            // childrenCenter is decreasing in this case
            auto it = std::upper_bound(data.childrenCenter.rbegin(), data.childrenCenter.rend(), floor_cast<int>(pos));
            return std::distance(it, data.childrenCenter.rend());
        } else {
            auto it = std::upper_bound(data.childrenCenter.begin(), data.childrenCenter.end(), floor_cast<int>(pos));
            return std::distance(data.childrenCenter.begin(), it);
        }
    };

    g_signal_connect(
            target, "enter", G_CALLBACK((+[](GtkDropTarget* target, gdouble x, gdouble y, gpointer) -> GdkDragAction {
                ToolbarBox* toolbar =
                        TOOLBAR_BOX(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(target)))->parent;
                xoj_assert(toolbar->editingData);
                auto& data = *(toolbar->editingData);

                GdkDrop* drop = gtk_drop_target_get_drop(target);
                if (!drop) {
                    g_warning("Drop::enter no drop");
                    return GdkDragAction(0);
                }
                auto* drag = gdk_drop_get_drag(drop);
                if (!drag) {
                    // The drag came from another app
                    g_debug("Drop::enter but not GdkDrag - the drag probably comes from another app");
                    return GdkDragAction(0);
                }
                auto* content = gdk_drag_get_content(drag);
                if (!content) {
                    g_message("Drop::enter no content");
                    return GdkDragAction(0);
                }
                GValue val = G_VALUE_INIT;
                g_value_init(&val, xoj::dnd::get_tool_item_gtype());
                if (!gdk_content_provider_get_value(content, &val, nullptr)) {
                    g_message("Drop::enter no value");
                    return GdkDragAction(0);
                }

                if (data.tempItem) {
                    g_warning("Drop::enter but already have a DragItem");
                    return GdkDragAction(0);
                }

                auto side = toolbar->getSide();
                auto [item, proxy] = data.handler->createItem(xoj::dnd::g_value_get_tool_item_string(&val), side);
                data.tempItem = item;

                g_message("Drop::enter: Created %s at %p", xoj::dnd::g_value_get_tool_item_string(&val), item.get());

                gtk_widget_add_css_class(item.get(), "phantom");
                gtk_widget_set_can_target(item.get(), false);  // Disable pointer interactions while editing
                gtk_widget_set_parent(item.get(), toolbar->getWidget());

                data.tempItemPosition = getInsertPosition(side, x, y, data);
                xoj_assert(as_unsigned(data.tempItemPosition) <=
                           toolbar->children.size());  // Could be == for end insertion
                toolbar->children.emplace(std::next(toolbar->children.begin(), data.tempItemPosition), std::move(item),
                                          std::move(proxy));

                return GDK_ACTION_COPY;
            })),
            nullptr);

    g_signal_connect(
            target, "motion", G_CALLBACK(+[](GtkDropTarget* target, gdouble x, gdouble y, gpointer) -> GdkDragAction {
                ToolbarBox* toolbar =
                        TOOLBAR_BOX(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(target)))->parent;
                xoj_assert(toolbar->editingData);
                auto& data = *(toolbar->editingData);

                if (!data.tempItem) {
                    g_warning("Drop::motion but no DragItem yet");
                    return GdkDragAction(0);
                }

                xoj_assert(data.tempItemPosition >= 0);
                xoj_assert(as_unsigned(data.tempItemPosition) < toolbar->children.size());
                xoj_assert(toolbar->children[as_unsigned(data.tempItemPosition)].widget.get() == data.tempItem.get());

                auto newPosition = getInsertPosition(toolbar->getSide(), x, y, data);

                xoj_assert(as_unsigned(newPosition) < toolbar->children.size());

                if (newPosition == data.tempItemPosition) {
                    // The item is already at the right place.
                    return GDK_ACTION_COPY;
                }
                if (newPosition < data.tempItemPosition) {
                    auto first = std::next(toolbar->children.begin(), newPosition);
                    auto middle = std::next(toolbar->children.begin(), data.tempItemPosition);
                    auto last = std::next(middle);
                    std::rotate(first, middle, last);
                } else {
                    auto first = std::next(toolbar->children.begin(), data.tempItemPosition);
                    auto middle = std::next(first);
                    auto last = std::next(toolbar->children.begin(), newPosition + 1);
                    std::rotate(first, middle, last);
                }
                data.tempItemPosition = newPosition;
                gtk_widget_queue_allocate(toolbar->getWidget());

                return GDK_ACTION_COPY;
            }),
            nullptr);

    g_signal_connect(
            target, "leave", G_CALLBACK(+[](GtkDropTarget* target, gpointer) {
                ToolbarBox* toolbar =
                        TOOLBAR_BOX(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(target)))->parent;
                xoj_assert(toolbar->editingData);
                auto& data = *(toolbar->editingData);

                if (!data.tempItem) {
                    g_warning("Drop::leave but no DragItem");
                    return;
                }

                xoj_assert(data.tempItemPosition >= 0);
                xoj_assert(as_unsigned(data.tempItemPosition) < toolbar->children.size());
                xoj_assert(toolbar->children[as_unsigned(data.tempItemPosition)].widget.get() == data.tempItem.get());

                toolbar->children.erase(std::next(toolbar->children.begin(), data.tempItemPosition));

                // If Child::operator=(Child&&) called gtk_widget_unparent() as it should (see comment there), we would
                // not need the following...
                gtk_widget_queue_resize(toolbar->getWidget());

                data.tempItemPosition = -1;
                data.tempItem.reset();
            }),
            nullptr);

    g_signal_connect(
            target, "drop",
            G_CALLBACK(+[](GtkDropTarget* target, const GValue* v, gdouble x, gdouble y, gpointer) -> gboolean {
                ToolbarBox* toolbar =
                        TOOLBAR_BOX(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(target)))->parent;
                xoj_assert(toolbar->editingData);
                auto& data = *(toolbar->editingData);

                if (!data.tempItem) {
                    g_warning("Drop::drop but no DragItem");
                    return false;
                }

                xoj_assert(data.tempItemPosition >= 0);
                xoj_assert(as_unsigned(data.tempItemPosition) < toolbar->children.size());
                xoj_assert(toolbar->children[as_unsigned(data.tempItemPosition)].widget.get() == data.tempItem.get());
                gtk_widget_remove_css_class(data.tempItem.get(), "phantom");

                // This is only required to refill data.childrenCenter and data.childrenEnd for further DND's
                gtk_widget_queue_allocate(toolbar->getWidget());

                data.tempItemPosition = -1;
                data.tempItem.reset();

                return true;
            }),
            nullptr);

    gtk_widget_add_controller(getWidget(), GTK_EVENT_CONTROLLER(target));
    editingData->target.reset(target, xoj::util::ref);

    constexpr auto getChildUnderPointer = +[](ToolbarSide side, double x, double y, EditingData& data) {
        double pos = to_Orientation(side) == GTK_ORIENTATION_HORIZONTAL ? x : y;
        if (data.rightToLeft) {
            auto it = std::upper_bound(data.childrenEnd.rbegin(), data.childrenEnd.rend(), round_cast<int>(pos));
            return it == data.childrenEnd.rbegin() ? -1 : std::distance(it, data.childrenEnd.rend());
        } else {
            auto it = std::upper_bound(data.childrenEnd.begin(), data.childrenEnd.end(), round_cast<int>(pos));
            return it == data.childrenEnd.end() ? -1 : std::distance(data.childrenEnd.begin(), it);
        }
    };

    auto* source = gtk_drag_source_new();
    gtk_drag_source_set_actions(source, GDK_ACTION_MOVE);
    g_signal_connect(
            source, "prepare",
            G_CALLBACK(+[](GtkDragSource* source, double x, double y, gpointer) -> GdkContentProvider* {
                ToolbarBox* toolbar =
                        TOOLBAR_BOX(gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(source)))->parent;
                xoj_assert(toolbar->editingData);
                auto& data = *(toolbar->editingData);

                auto pos = getChildUnderPointer(toolbar->getSide(), x, y, data);
                if (pos < 0) {
                    // The pointer is on no item
                    return nullptr;
                }
                xoj_assert(as_unsigned(pos) < toolbar->children.size());
                auto child = std::next(toolbar->children.begin(), pos);

                auto* prop = static_cast<std::string*>(
                        g_object_get_data(G_OBJECT(child->widget.get()), TOOLITEM_ID_PROPERTY));
                std::string id(std::move(*prop));  // The widget will get removed, so it does not need the data anymore

                auto icon = data.handler->createIcon(id.c_str());
                gtk_image_set_pixel_size(GTK_IMAGE(icon.get()), 24);
                gtk_drag_source_set_icon(source, gtk_widget_paintable_new(icon.get()), 12, 12);

                // Remove the dragged item
                toolbar->children.erase(child);
                // If Child::operator=(Child&&) called gtk_widget_unparent() as it should (see comment there), we would
                // not need the following...
                gtk_widget_queue_resize(toolbar->getWidget());

                return gdk_content_provider_new_typed(xoj::dnd::get_tool_item_gtype(), id.c_str());
            }),
            nullptr);

    gtk_widget_add_controller(getWidget(), GTK_EVENT_CONTROLLER(source));
    editingData->source.reset(source, xoj::util::ref);

    // Disable pointer interactions with the items
    for (auto&& c: children) {
        gtk_widget_set_can_target(c.widget.get(), false);
    }
    gtk_widget_set_can_target(overflowMenuButton.get(), false);
}

auto ToolbarBox::endEditing() -> ToolbarEntry {
    gtk_widget_remove_controller(getWidget(), GTK_EVENT_CONTROLLER(editingData->target.get()));
    gtk_widget_remove_controller(getWidget(), GTK_EVENT_CONTROLLER(editingData->source.get()));
    editingData.reset();

    gtk_widget_remove_css_class(getWidget(), "editing");

    // Refill/reorder the overflow menu
    for (auto it = children.begin(); it != children.end(); ++it) {
        if (it->proxy) {
            if (gtk_widget_get_parent(it->proxy.get()) == GTK_WIDGET(popoverBox)) {
                gtk_box_reorder_child_after(popoverBox, it->proxy.get(),
                                            gtk_widget_get_last_child(GTK_WIDGET(popoverBox)));
            } else {
                gtk_box_append(popoverBox, it->proxy.get());
            }
        }
    }

    // Reenable pointer interactions with the items
    for (auto&& c: children) {
        gtk_widget_set_can_target(c.widget.get(), true);
    }
    gtk_widget_set_can_target(overflowMenuButton.get(), true);

    if (children.empty()) {
        gtk_widget_set_visible(getWidget(), false);
    }

    ToolbarEntry model;
    model.setName(name);

    for (auto&& c: children) {
        auto* prop = static_cast<std::string*>(g_object_get_data(G_OBJECT(c.widget.get()), TOOLITEM_ID_PROPERTY));
        model.addItem(*prop);
    }

    return model;
}
