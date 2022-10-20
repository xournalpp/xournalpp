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

    /**
     * @brief Searches the entire document until a match is found, starting from `page = next(currentPage)` and
     * iterating through the pages via page = next(page). The search stops after the first page with at least one match.
     * All the match on that page are stored in XojPageView::search of the corresponding page. The current page is not
     * search!
     * @param next The parameter `next` must be convertible to size_t(size_t) and satisfy the following assertions
     *              * Iterating from page = next(currentPage) by page = next(page) must reach page == currentPage at
     * some point.
     *              * If page is a valid page number, then so is next(page).
     */
    template <class Fun>
    void search(Fun next) const;

    /**
     * @brief Named specialization of search(), where next(page) = (page + 1) % pageCount
     */
    void searchNext() const;
    /**
     * @brief Named specialization of search(), where next(page) = (page + pageCount - 1) % pageCount
     */
    void searchPrevious() const;

    void search(const char* text);
    bool searchTextonCurrentPage(const char* text, size_t* occurrences, double* yOfUpperMostMatch);

private:
    Control* control;
    GtkCssProvider* cssTextFild;
};
