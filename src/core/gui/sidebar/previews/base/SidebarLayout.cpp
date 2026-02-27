#include "SidebarLayout.h"

#include <algorithm>  // for max
#include <vector>     // for vector

#include <gtk/gtk.h>  // for GTK_FIXED, gtk_fixed_move

#include "util/safe_casts.h"  // for as_unsigned

#include "SidebarPreviewBase.h"       // for SidebarPreviewBase
#include "SidebarPreviewBaseEntry.h"  // for SidebarPreviewBaseEntry

constexpr int MARGIN = 5;  ///< Margin in px

class SidebarRow {
public:
    explicit SidebarRow(int width) {
        this->width = width;
        this->currentWidth = 0;
    }

    ~SidebarRow() { clear(); }

    /// Gets the size the widget wants to have (even before the widget is ever realized)
    static int getWidthOf(SidebarPreviewBaseEntry* e) {
        int res = 0;
        gtk_widget_measure(e->getWidget(), GTK_ORIENTATION_HORIZONTAL, -1, nullptr, &res, nullptr, nullptr);
        return res;
    }
    static int getHeightOf(SidebarPreviewBaseEntry* e) {
        int res = 0;
        gtk_widget_measure(e->getWidget(), GTK_ORIENTATION_VERTICAL, -1, nullptr, &res, nullptr, nullptr);
        return res;
    }

    auto isSpaceFor(SidebarPreviewBaseEntry* p) -> bool {
        if (this->entries.empty()) {
            return true;
        }

        if (this->currentWidth + getWidthOf(p) + 2 * MARGIN < width) {
            return true;
        }
        return false;
    }

    void add(SidebarPreviewBaseEntry* p) {
        this->entries.push_back(p);
        this->currentWidth += getWidthOf(p) + 2 * MARGIN;
    }

    void clear() {
        this->entries.clear();
        this->currentWidth = 0;
    }

    auto getCount() -> size_t { return this->entries.size(); }

    auto getWidth() const -> int { return this->currentWidth; }

    auto placeAt(int y, GtkFixed* layout) -> int {
        int height = 0;
        int x = MARGIN;

        for (SidebarPreviewBaseEntry* p: this->entries) {
            height = std::max(height, getHeightOf(p));
        }


        for (SidebarPreviewBaseEntry* p: this->entries) {
            int currentY = (height - getHeightOf(p)) / 2 + y;

            gtk_fixed_move(layout, p->getWidget(), x, currentY);

            x += getWidthOf(p) + 2 * MARGIN;

            p->setVerticalPosition({currentY, currentY + getHeightOf(p)});
        }


        return height;
    }

private:
    int width;
    int currentWidth;

    std::vector<SidebarPreviewBaseEntry*> entries;
};

void SidebarLayout::layout(SidebarPreviewBase* sidebar) {
    int y = MARGIN;
    int width = 0;

    int sidebarWidth = gtk_widget_get_width(sidebar->scrollableBox.get());

    SidebarRow row(sidebarWidth);
    GtkFixed* w = sidebar->miniaturesContainer.get();

    for (auto& p: sidebar->previews) {
        if (row.isSpaceFor(p.get())) {
            row.add(p.get());
        } else {
            y += row.placeAt(y, w) + 2 * MARGIN;

            width = std::max(width, row.getWidth());

            row.clear();
            row.add(p.get());
        }
    }

    if (row.getCount() != 0) {
        y += row.placeAt(y, w) + MARGIN;

        width = std::max(width, row.getWidth());

        row.clear();
    } else {
        // We added 2*MARGIN below the last row.
        y -= MARGIN;
    }

    gtk_widget_set_size_request(GTK_WIDGET(w), width, y);
}
