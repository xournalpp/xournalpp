#include "SidebarLayout.h"

#include <algorithm>  // for max
#include <vector>     // for vector

#include <gtk/gtk.h>  // for GTK_FIXED, gtk_fixed_move

#include "util/gtk4_helper.h"
#include "util/safe_casts.h"  // for as_unsigned

#include "SidebarPreviewBase.h"       // for SidebarPreviewBase
#include "SidebarPreviewBaseEntry.h"  // for SidebarPreviewBaseEntry

class SidebarRow {
public:
    explicit SidebarRow(int width) {
        this->width = width;
        this->currentWidth = 0;
    }

    ~SidebarRow() { clear(); }


    auto isSpaceFor(SidebarPreviewBaseEntry* p) -> bool {
        if (this->list.empty()) {
            return true;
        }

        if (this->currentWidth + p->getWidth() < width) {
            return true;
        }
        return false;
    }

    void add(SidebarPreviewBaseEntry* p) {
        this->list.push_back(p);
        this->currentWidth += p->getWidth();
    }

    void clear() {
        this->list.clear();
        this->currentWidth = 0;
    }

    auto getCount() -> size_t { return this->list.size(); }

    auto getWidth() const -> int { return this->currentWidth; }

    auto placeAt(int y, GtkFixed* layout) -> int {
        int height = 0;
        int x = 0;

        for (SidebarPreviewBaseEntry* p: this->list) { height = std::max(height, p->getHeight()); }


        for (SidebarPreviewBaseEntry* p: this->list) {
            int currentY = (height - p->getHeight()) / 2;

            gtk_fixed_move(layout, p->getWidget(), x, y + currentY);

            x += p->getWidth();
        }


        return height;
    }

private:
    int width;
    int currentWidth;

    std::vector<SidebarPreviewBaseEntry*> list;
};

void SidebarLayout::layout(SidebarPreviewBase* sidebar) {
    int y = 0;
    int width = 0;

    int sidebarWidth = gtk_widget_get_width(sidebar->scrollableBox.get());

    SidebarRow row(sidebarWidth);
    GtkFixed* w = sidebar->miniaturesContainer.get();

    for (auto& p: sidebar->previews) {
        if (row.isSpaceFor(p.get())) {
            row.add(p.get());
        } else {
            y += row.placeAt(y, w);

            width = std::max(width, row.getWidth());

            row.clear();
            row.add(p.get());
        }
    }

    if (row.getCount() != 0) {
        y += row.placeAt(y, w);

        width = std::max(width, row.getWidth());

        row.clear();
    }

    gtk_widget_set_size_request(GTK_WIDGET(w), width, y);
}
