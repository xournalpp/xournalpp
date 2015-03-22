/*
 * Xournal++
 *
 * Handles the searchbar and search events
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SEARCHBAR_H__
#define __SEARCHBAR_H__

#include <gtk/gtk.h>

#include <XournalType.h>

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
	static void buttonPreviousSearchClicked(GtkButton* button,
											SearchBar* searchBar);

	void searchNext();
	void searchPrevious();

	void search(const char* text);
	bool searchTextonCurrentPage(const char* text, int* occures, double* top);

private:
	XOJ_TYPE_ATTRIB;

	Control* control;
	GdkColor defaultColor;
};

#endif /* __SEARCHBAR_H__ */
