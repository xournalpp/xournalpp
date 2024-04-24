#include "ToolPageSpinner.h"

#include <algorithm>  // for find
#include <utility>    // for move

#include <glib-object.h>  // for g_object_ref_sink

#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "gui/widgets/SpinPageAdapter.h"          // for SpinPageAdapter
#include "util/Assert.h"                          // for xoj_assert
#include "util/Util.h"
#include "util/i18n.h"         // for FS, _, _F, C_F

class ToolPageSpinner::Instance final {
public:
    Instance(ToolPageSpinner* handler, GtkOrientation orientation): handler(handler), orientation(orientation) {}

    xoj::util::WidgetSPtr makeWidget() {
        GtkWidget* spinner = gtk_spin_button_new_with_range(0, 1, 1);
        gtk_orientable_set_orientation(GTK_ORIENTABLE(spinner), orientation);
        this->spinner.setWidget(spinner);  // takes ownership of spinner reference
        this->spinner.addListener(handler->getListener());

        GtkWidget* numberLabel = gtk_label_new("");
        GtkWidget* pageLabel = gtk_label_new(_("Page"));
        if (orientation == GTK_ORIENTATION_HORIZONTAL) {
            gtk_widget_set_valign(pageLabel, GTK_ALIGN_BASELINE);
            gtk_widget_set_margin_start(pageLabel, 7);
            gtk_widget_set_margin_end(pageLabel, 7);
            gtk_widget_set_valign(spinner, GTK_ALIGN_BASELINE);
            gtk_widget_set_valign(numberLabel, GTK_ALIGN_BASELINE);
            gtk_widget_set_margin_start(numberLabel, 7);
            gtk_widget_set_margin_end(numberLabel, 7);
        } else {
            gtk_widget_set_halign(pageLabel, GTK_ALIGN_CENTER);
            gtk_widget_set_margin_top(pageLabel, 7);
            gtk_widget_set_margin_bottom(pageLabel, 7);
            gtk_widget_set_halign(spinner, GTK_ALIGN_CENTER);
            gtk_widget_set_halign(numberLabel, GTK_ALIGN_CENTER);
            gtk_widget_set_valign(spinner, GTK_ALIGN_BASELINE);
            gtk_widget_set_valign(numberLabel, GTK_ALIGN_BASELINE);
            gtk_label_set_justify(GTK_LABEL(numberLabel), GTK_JUSTIFY_CENTER);
            gtk_widget_set_margin_top(numberLabel, 7);
            gtk_widget_set_margin_bottom(numberLabel, 7);
        }
        this->numberLabel = GTK_LABEL(numberLabel);

        xoj::util::WidgetSPtr item(gtk_box_new(orientation, 1), xoj::util::adopt);
        GtkBox* box = GTK_BOX(item.get());
        gtk_box_append(box, pageLabel);
        gtk_box_append(box, spinner);
        gtk_box_append(box, numberLabel);

        return item;
    }

    void disconnect() { handler = nullptr; }

private:
    ToolPageSpinner* handler;
    GtkOrientation orientation;
    SpinPageAdapter spinner;
    GtkLabel* numberLabel;

public:
    void setPageInfo(size_t currentPage, size_t pageCount, size_t pdfPage) {
        if (pageCount == 0) {
            spinner.setMinMaxPage(0, 0);
            spinner.setPage(0);
        } else {
            spinner.setMinMaxPage(1, pageCount);
            spinner.setPage(currentPage + 1);
        }

        updateLabel(pageCount, pdfPage);
    }
    ToolPageSpinner* getHandler() const { return handler; }

private:
    void updateLabel(const size_t pageCount, const size_t pdfPage) {
        std::string ofString = FS(C_F("Page {pagenumber} \"of {pagecount}\"", " of {1}") % pageCount);
        if (pdfPage != npos) {
            if (this->orientation == GTK_ORIENTATION_HORIZONTAL) {
                ofString += std::string(", ") + FS(_F("PDF Page {1}") % (pdfPage + 1));
            } else {
                ofString += std::string("\n") + FS(_F("PDF {1}") % (pdfPage + 1));
            }
        }
        gtk_label_set_text(this->numberLabel, ofString.c_str());
    }
};

ToolPageSpinner::ToolPageSpinner(std::string id, IconNameHelper iconNameHelper, SpinPageListener* listener):
        AbstractToolItem(std::move(id), Category::NAVIGATION), iconNameHelper(iconNameHelper), listener(listener) {}

ToolPageSpinner::~ToolPageSpinner() {
    for (auto* i: instances) {
        i->disconnect();
    }
}

void ToolPageSpinner::setPageInfo(size_t currentPage, size_t pageCount, size_t pdfPage) {
    for (auto* i: instances) {
        i->setPageInfo(currentPage, pageCount, pdfPage);
    }
}

auto ToolPageSpinner::getToolDisplayName() const -> std::string { return _("Page number"); }

auto ToolPageSpinner::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(iconNameHelper.iconName("page-spinner").c_str());
}

auto ToolPageSpinner::createItem(ToolbarSide side) -> Widgetry {
    auto data = std::make_unique<Instance>(this, to_Orientation(side));
    auto item = data->makeWidget();

    this->instances.emplace_back(data.get());  // Keep a ref for callback propagation

    // Destroy *data if the widget is destroyed
    g_object_weak_ref(
            G_OBJECT(item.get()),
            +[](gpointer d, GObject*) {
                auto* data = static_cast<Instance*>(d);
                if (auto* h = data->getHandler(); h) {
                    auto& instances = h->instances;
                    if (auto it = std::find(instances.begin(), instances.end(), data); it != instances.end()) {
                        instances.erase(it);
                    }
                }
                delete data;
            },
            data.release());

    return {std::move(item), nullptr};
}
