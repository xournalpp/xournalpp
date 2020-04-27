#include "SidebarPreviewPages.h"

#include <memory>

#include "control/Control.h"
#include "control/PdfCache.h"
#include "gui/sidebar/previews/base/SidebarToolbar.h"
#include "undo/CopyUndoAction.h"
#include "undo/SwapUndoAction.h"

#include "SidebarPreviewPageEntry.h"
#include "i18n.h"

SidebarPreviewPages::SidebarPreviewPages(Control* control, GladeGui* gui, SidebarToolbar* toolbar):
        SidebarPreviewBase(control, gui, toolbar), contextMenu(gui->get("sidebarPreviewContextMenu")) {
    // Connect the context menu actions
    const std::map<std::string, SidebarActions> ctxMenuActions = {
            {"sidebarPreviewDuplicate", SIDEBAR_ACTION_COPY},
            {"sidebarPreviewDelete", SIDEBAR_ACTION_DELETE},
            {"sidebarPreviewMoveUp", SIDEBAR_ACTION_MOVE_UP},
            {"sidebarPreviewMoveDown", SIDEBAR_ACTION_MOVE_DOWN},
            {"sidebarPreviewNewBefore", SIDEBAR_ACTION_NEW_BEFORE},
            {"sidebarPreviewNewAfter", SIDEBAR_ACTION_NEW_AFTER},
    };

    for (const auto& pair: ctxMenuActions) {
        GtkWidget* const entry = gui->get(pair.first);
        g_assert(entry != nullptr);

        // Unfortunately, we need a fairly complicated mechanism to keep track
        // of which action we want to execute.
        using Data = SidebarPreviewPages::ContextMenuData;
        auto userdata = std::make_unique<Data>(Data{this->toolbar, pair.second});

        const auto callback =
                G_CALLBACK(+[](GtkMenuItem* item, Data* data) { data->toolbar->runAction(data->actions); });
        const gulong signalId = g_signal_connect(entry, "activate", callback, userdata.get());
        g_object_ref(entry);
        this->contextMenuSignals.emplace_back(entry, signalId, std::move(userdata));

        if (pair.first == "sidebarPreviewMoveDown") {
            this->contextMenuMoveDown = entry;
        } else if (pair.first == "sidebarPreviewMoveUp") {
            this->contextMenuMoveUp = entry;
        }
    }
    g_assert(this->contextMenuMoveDown != nullptr);
    g_assert(this->contextMenuMoveUp != nullptr);
}

SidebarPreviewPages::~SidebarPreviewPages() {
    for (const auto& signalTuple: this->contextMenuSignals) {
        GtkWidget* const widget = std::get<0>(signalTuple);
        const guint handlerId = std::get<1>(signalTuple);
        if (g_signal_handler_is_connected(widget, handlerId)) {
            g_signal_handler_disconnect(widget, handlerId);
        }
        g_object_unref(widget);
    }
}

auto SidebarPreviewPages::getName() -> string { return _("Page Preview"); }

auto SidebarPreviewPages::getIconName() -> string { return "sidebar-page-preview"; }

/**
 * Called when an action is performed
 */
void SidebarPreviewPages::actionPerformed(SidebarActions action) {
    switch (action) {
        case SIDEBAR_ACTION_MOVE_UP: {
            Document* doc = control->getDocument();
            PageRef swappedPage = control->getCurrentPage();
            if (!swappedPage || doc->getPageCount() <= 1) {
                return;
            }

            doc->lock();
            size_t page = doc->indexOf(swappedPage);
            PageRef otherPage = doc->getPage(page - 1);
            if (page != npos) {
                doc->deletePage(page);
                doc->insertPage(swappedPage, page - 1);
            }
            doc->unlock();

            UndoRedoHandler* undo = control->getUndoRedoHandler();
            undo->addUndoAction(std::make_unique<SwapUndoAction>(page - 1, true, swappedPage, otherPage));

            control->firePageDeleted(page);
            control->firePageInserted(page - 1);
            control->firePageSelected(page - 1);

            control->getScrollHandler()->scrollToPage(page - 1);
            break;
        }
        case SIDEBAR_ACTION_MOVE_DOWN: {
            Document* doc = control->getDocument();
            PageRef swappedPage = control->getCurrentPage();
            if (!swappedPage || doc->getPageCount() <= 1) {
                return;
            }

            doc->lock();
            size_t page = doc->indexOf(swappedPage);
            PageRef otherPage = doc->getPage(page + 1);
            if (page != npos) {
                doc->deletePage(page);
                doc->insertPage(swappedPage, page + 1);
            }
            doc->unlock();

            UndoRedoHandler* undo = control->getUndoRedoHandler();
            undo->addUndoAction(std::make_unique<SwapUndoAction>(page, false, swappedPage, otherPage));

            control->firePageDeleted(page);
            control->firePageInserted(page + 1);
            control->firePageSelected(page + 1);

            control->getScrollHandler()->scrollToPage(page + 1);
            break;
        }
        case SIDEBAR_ACTION_COPY: {
            Document* doc = control->getDocument();
            PageRef currentPage = control->getCurrentPage();
            if (!currentPage) {
                return;
            }

            doc->lock();
            size_t page = doc->indexOf(currentPage);

            auto newPage = PageRef(currentPage->clone());
            doc->insertPage(newPage, page + 1);
            doc->unlock();

            UndoRedoHandler* undo = control->getUndoRedoHandler();
            undo->addUndoAction(std::make_unique<CopyUndoAction>(newPage, page + 1));

            control->firePageInserted(page + 1);
            control->firePageSelected(page + 1);

            control->getScrollHandler()->scrollToPage(page + 1);
            break;
        }
        case SIDEBAR_ACTION_DELETE:
            control->deletePage();
            break;
        case SIDEBAR_ACTION_NEW_BEFORE:
            control->insertNewPage(control->getCurrentPageNo());
            break;
        case SIDEBAR_ACTION_NEW_AFTER:
            control->insertNewPage(control->getCurrentPageNo() + 1);
            break;
        default:
            break;
        case SIDEBAR_ACTION_NONE:
            break;
    }
}

void SidebarPreviewPages::updatePreviews() {
    Document* doc = this->getControl()->getDocument();
    doc->lock();
    size_t len = doc->getPageCount();

    if (this->previews.size() == len) {
        doc->unlock();
        return;
    }

    for (SidebarPreviewBaseEntry* p: this->previews) {
        delete p;
    }
    this->previews.clear();

    for (size_t i = 0; i < len; i++) {
        SidebarPreviewBaseEntry* p = new SidebarPreviewPageEntry(this, doc->getPage(i));
        this->previews.push_back(p);
        gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);
    }

    layout();
    doc->unlock();
}

void SidebarPreviewPages::pageSizeChanged(size_t page) {
    if (page == npos || page >= this->previews.size()) {
        return;
    }
    SidebarPreviewBaseEntry* p = this->previews[page];
    p->updateSize();
    p->repaint();

    layout();
}

void SidebarPreviewPages::pageChanged(size_t page) {
    if (page == npos || page >= this->previews.size()) {
        return;
    }

    SidebarPreviewBaseEntry* p = this->previews[page];
    p->repaint();
}

void SidebarPreviewPages::pageDeleted(size_t page) {
    if (page >= previews.size()) {
        return;
    }

    delete previews[page];
    previews.erase(previews.begin() + page);

    // Unselect page, to prevent double selection displaying
    unselectPage();

    layout();
}

void SidebarPreviewPages::pageInserted(size_t page) {
    Document* doc = control->getDocument();
    doc->lock();

    SidebarPreviewBaseEntry* p = new SidebarPreviewPageEntry(this, doc->getPage(page));

    doc->unlock();

    this->previews.insert(this->previews.begin() + page, p);

    gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);

    // Unselect page, to prevent double selection displaying
    unselectPage();

    layout();
}

/**
 * Unselect the last selected page, if any
 */
void SidebarPreviewPages::unselectPage() {
    for (SidebarPreviewBaseEntry* p: this->previews) {
        p->setSelected(false);
    }
}

void SidebarPreviewPages::pageSelected(size_t page) {
    if (this->selectedEntry != npos && this->selectedEntry < this->previews.size()) {
        this->previews[this->selectedEntry]->setSelected(false);
    }
    this->selectedEntry = page;

    if (this->selectedEntry != npos && this->selectedEntry < this->previews.size()) {
        SidebarPreviewBaseEntry* p = this->previews[this->selectedEntry];
        p->setSelected(true);
        scrollToPreview(this);

        int actions = 0;
        if (page != 0 && !this->previews.empty()) {
            actions |= SIDEBAR_ACTION_MOVE_UP;
        }

        if (page != this->previews.size() - 1 && !this->previews.empty()) {
            actions |= SIDEBAR_ACTION_MOVE_DOWN;
        }

        if (!this->previews.empty()) {
            actions |= SIDEBAR_ACTION_COPY;
        }

        if (this->previews.size() > 1) {
            actions |= SIDEBAR_ACTION_DELETE;
        }

        this->toolbar->setHidden(false);
        this->toolbar->setButtonEnabled(static_cast<SidebarActions>(actions));

        gtk_widget_set_sensitive(this->contextMenuMoveUp, actions & SIDEBAR_ACTION_MOVE_UP);
        gtk_widget_set_sensitive(this->contextMenuMoveDown, actions & SIDEBAR_ACTION_MOVE_DOWN);
    }
}

void SidebarPreviewPages::openPreviewContextMenu() {
    gtk_menu_popup(GTK_MENU(this->contextMenu), nullptr, nullptr, nullptr, nullptr, 3, gtk_get_current_event_time());
}
