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

#include <XournalType.h>

#include <gtk/gtk.h>

class Control;

class SearchBar
{
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
	bool searchTextonCurrentPage(const char* text, int* occures, double* top);

private:
	Control* control;
	GtkCssProvider* cssTextFild;
};
