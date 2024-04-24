#include "SearchBar.h"

#include <string>  // for allocator, string

#include <gdk/gdk.h>         // for GdkEventKey, GDK_SHIFT_MASK
#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Return
#include <glib-object.h>     // for G_CALLBACK, g_signal_connect
#include <glib.h>            // for g_free, g_strdup_printf

#include "control/Control.h"           // for Control
#include "control/ScrollHandler.h"     // for ScrollHandler
#include "control/zoom/ZoomControl.h"  // for ZoomControl
#include "gui/MainWindow.h"            // for MainWindow
#include "model/Document.h"            // for Document
#include "util/PlaceholderString.h"    // for PlaceholderString
#include "util/i18n.h"                 // for _, FC, _F

SearchBar::SearchBar(Control* control): control(control) {
    MainWindow* win = control->getWindow();

    GtkWidget* close = win->get("buttonCloseSearch");
    g_signal_connect(close, "clicked", G_CALLBACK(buttonCloseSearchClicked), this);

    GtkWidget* next = win->get("btSearchForward");
    GtkWidget* previous = win->get("btSearchBack");
    g_signal_connect(next, "clicked",
                     G_CALLBACK(+[](GtkButton*, gpointer d) { static_cast<SearchBar*>(d)->searchNext(); }), this);
    g_signal_connect(previous, "clicked",
                     G_CALLBACK(+[](GtkButton*, gpointer d) { static_cast<SearchBar*>(d)->searchPrevious(); }), this);

    GtkWidget* searchTextField = win->get("searchTextField");
    g_signal_connect(searchTextField, "search-changed", G_CALLBACK(searchTextChangedCallback), this);
    g_signal_connect(searchTextField, "stop-search",
                     G_CALLBACK(+[](GtkSearchEntry*, gpointer d) { static_cast<SearchBar*>(d)->showSearchBar(false); }),
                     this);
    g_signal_connect(searchTextField, "next-match", G_CALLBACK(+[](GtkSearchEntry* e, gpointer d) {
                         static_cast<SearchBar*>(d)->searchNext();
                         // Grab focus again since searching will take away focus
                         gtk_widget_grab_focus(GTK_WIDGET(e));
                     }),
                     this);
    g_signal_connect(searchTextField, "previous-match", G_CALLBACK(+[](GtkSearchEntry* e, gpointer d) {
                         static_cast<SearchBar*>(d)->searchPrevious();
                         // Grab focus again since searching will take away focus
                         gtk_widget_grab_focus(GTK_WIDGET(e));
                     }),
                     this);

    GtkEventController* ctrl = gtk_shortcut_controller_new();
    gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(ctrl),
                                         gtk_shortcut_new(gtk_keyval_trigger_new(GDK_KEY_Return, GdkModifierType(0)),
                                                          gtk_signal_action_new("next-match")));
    gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(ctrl),
                                         gtk_shortcut_new(gtk_keyval_trigger_new(GDK_KEY_Return, GDK_SHIFT_MASK),
                                                          gtk_signal_action_new("previous-match")));
    gtk_widget_add_controller(searchTextField, ctrl);

    cssTextFild = gtk_css_provider_new();
    gtk_style_context_add_provider(gtk_widget_get_style_context(win->get("searchTextField")),
                                   GTK_STYLE_PROVIDER(cssTextFild), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

SearchBar::~SearchBar() { this->control = nullptr; }

auto SearchBar::searchTextonCurrentPage(const char* text, size_t index, size_t* occurrences, XojPdfRectangle* matchRect)
        -> bool {
    size_t p = control->getCurrentPageNo();
    this->page = p;

    return control->searchTextOnPage(text, p, index, occurrences, matchRect);
}

void SearchBar::search(const char* text) {
    MainWindow* win = control->getWindow();
    GtkWidget* lbSearchState = win->get("lbSearchState");

    bool found = true;
    this->indexInPage = 0;

    if (*text != 0) {
        found = searchTextonCurrentPage(text, 1, &this->occurrences, nullptr);
        if (found) {
            if (occurrences == 1) {
                gtk_label_set_text(GTK_LABEL(lbSearchState), _("Text found once on this page"));
            } else {
                char* msg = g_strdup_printf(_("Text found %zu times on this page"), occurrences);
                gtk_label_set_text(GTK_LABEL(lbSearchState), msg);
                g_free(msg);
            }
        } else {
            gtk_label_set_text(GTK_LABEL(lbSearchState), _("Text not found"));
        }
    } else {
        searchTextonCurrentPage("", 1, nullptr, nullptr);
        gtk_label_set_text(GTK_LABEL(lbSearchState), "");
    }

    if (found) {
        gtk_css_provider_load_from_data(cssTextFild, "GtkSearchEntry {}", -1);
    } else {
        gtk_css_provider_load_from_data(cssTextFild, "GtkSearchEntry { color: #ff0000; }", -1);
    }
}

void SearchBar::searchTextChangedCallback(GtkSearchEntry* entry, SearchBar* searchBar) {
    const char* text = gtk_editable_get_text(GTK_EDITABLE(entry));
    searchBar->search(text);
}

void SearchBar::buttonCloseSearchClicked(GtkButton* button, SearchBar* searchBar) { searchBar->showSearchBar(false); }

template <class Fun>
void SearchBar::search(Fun next) {

    MainWindow* win = control->getWindow();
    GtkWidget* searchTextField = win->get("searchTextField");
    const char* text = gtk_editable_get_text(GTK_EDITABLE(searchTextField));
    GtkWidget* lbSearchState = win->get("lbSearchState");
    if (*text == 0) {
        return;
    }
    const size_t originalPage = page;

    XojPdfRectangle matchRect = XojPdfRectangle();
    // Search backwards through the pages, wrapping around if needed.
    for (;;) {
        next(text);
        const bool found = control->searchTextOnPage(text, page, indexInPage, &occurrences, &matchRect);

        if (found) {
            control->getScrollHandler()->scrollToPage(page, matchRect);
            control->getScrollHandler();
            gtk_label_set_text(GTK_LABEL(lbSearchState),
                               (occurrences == 1 ? FC(_F("Text found once on page {1}") % (page + 1)) :
                                                   FC(_F("Text found {1} times on page {2} ({3} of {1})") %
                                                      occurrences % (page + 1) % indexInPage)));
            return;
        }
        if (page == originalPage) {
            gtk_label_set_text(GTK_LABEL(lbSearchState), _("Text not found, searched on all pages"));
            return;
        }
    }
}

void SearchBar::searchNext() {
    size_t pageCount = control->getDocument()->getPageCount();
    search([&](const char* text) {
        indexInPage++;
        if (indexInPage > occurrences) {
            control->searchTextOnPage(text, page, 1, &occurrences, nullptr);  // clear the active marker
            page++;
            if (page >= pageCount) {
                page = 0;
            }
            indexInPage = 1;
        }
    });
}

void SearchBar::searchPrevious() {
    size_t pageCount = control->getDocument()->getPageCount();
    search([&](const char* text) {
        indexInPage--;
        if (indexInPage == 0 || indexInPage >= occurrences) {
            control->searchTextOnPage(text, page, 1, &occurrences, nullptr);  // clear the active marker
            page--;
            if (page > pageCount) {
                page = pageCount - 1;
            }
            control->searchTextOnPage(text, page, 1, &occurrences, nullptr);
            indexInPage = occurrences;
        }
    });
}

void SearchBar::showSearchBar(bool show) {
    MainWindow* win = control->getWindow();
    GtkWidget* searchBar = win->get("searchBar");

    if (show) {
        GtkWidget* searchTextField = win->get("searchTextField");
        gtk_widget_show(searchBar);
        gtk_widget_grab_focus(searchTextField);
        this->indexInPage = 0;
    } else {
        gtk_widget_hide(searchBar);
        const size_t pageCount = control->getDocument()->getPageCount();
        for (size_t i = pageCount - 1; i < pageCount; i--) {
            control->searchTextOnPage("", i, 0, nullptr, nullptr);
        }
    }
}
