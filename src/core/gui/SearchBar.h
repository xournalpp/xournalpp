/*
 * Xournal++
 *
 * Handles the searchbar and search events
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>             // for GtkButton, GtkEntry
#include <gtk/gtkcssprovider.h>  // for GtkCssProvider

class Control;

class SearchBar {
public:
    SearchBar(Control* control);
    virtual ~SearchBar();

    void showSearchBar(bool show);

private:
    static void buttonCloseSearchClicked(GtkButton* button, SearchBar* searchBar);
    static void searchTextChangedCallback(GtkEntry* entry, SearchBar* searchBar);

    static void buttonNextSearchClicked(GtkButton* button, SearchBar* searchBar);
    static void buttonPreviousSearchClicked(GtkButton* button, SearchBar* searchBar);

    void searchNext();
    void searchPrevious();

    void search(const char* text);
    bool searchTextonCurrentPage(const char* text, size_t* occurrences, double* yOfUpperMostMatch);

private:
    Control* control;
    GtkCssProvider* cssTextFild;
};
