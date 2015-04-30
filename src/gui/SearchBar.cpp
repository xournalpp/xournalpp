#include "SearchBar.h"
#include "control/Control.h"
#include <string.h>

#include <config.h>
#include <glib/gi18n-lib.h>

SearchBar::SearchBar(Control* control)
{
	XOJ_INIT_TYPE(SearchBar);

	this->control = control;

	MainWindow* win = control->getWindow();

	GtkWidget* close = win->get("buttonCloseSearch");
	g_signal_connect(close, "clicked", G_CALLBACK(buttonCloseSearchClicked), this);

	GtkWidget* next = win->get("btSearchForward");
	g_signal_connect(next, "clicked", G_CALLBACK(buttonNextSearchClicked), this);

	GtkWidget* previous = win->get("btSearchBack");
	g_signal_connect(previous, "clicked", G_CALLBACK(buttonPreviousSearchClicked), this);

	GtkWidget* searchTextField = win->get("searchTextField");
	g_signal_connect(searchTextField, "changed", G_CALLBACK(searchTextChangedCallback), this);

	defaultColor = searchTextField->style->base[GTK_STATE_NORMAL];
}

SearchBar::~SearchBar()
{
	XOJ_CHECK_TYPE(SearchBar);

	this->control = NULL;

	XOJ_RELEASE_TYPE(SearchBar);
}

bool SearchBar::searchTextonCurrentPage(const char* text, int* occures, double* top)
{
	XOJ_CHECK_TYPE(SearchBar);

	int p = control->getCurrentPageNo();

	return control->searchTextOnPage(text, p, occures, top);
}

void SearchBar::search(const char* text)
{
	XOJ_CHECK_TYPE(SearchBar);

	MainWindow* win = control->getWindow();
	GdkColor color = { 0, 0xff00, 0xc000, 0xc000 };
	GtkWidget* searchTextField = win->get("searchTextField");

	GtkWidget* lbSearchState = win->get("lbSearchState");

	bool found = true;
	int occures = 0;

	if (*text != 0)
	{
		found = searchTextonCurrentPage(text, &occures, NULL);
		if (found)
		{
			if (occures == 1)
			{
				gtk_label_set_text(GTK_LABEL(lbSearchState), _("Text found on this page"));
			}
			else
			{
				char* msg = g_strdup_printf(_("Text %i times found on this page"), occures);
				gtk_label_set_text(GTK_LABEL(lbSearchState), msg);
				g_free(msg);
			}
		}
		else
		{
			gtk_label_set_text(GTK_LABEL(lbSearchState), _("Text not found"));
		}
	}
	else
	{
		searchTextonCurrentPage(NULL, NULL, NULL);
		gtk_label_set_text(GTK_LABEL(lbSearchState), "");
	}

	if (found)
	{
		gtk_widget_modify_base(searchTextField, GTK_STATE_NORMAL, &defaultColor);
	}
	else
	{
		gtk_widget_modify_base(searchTextField, GTK_STATE_NORMAL, &color);
	}
}

void SearchBar::searchTextChangedCallback(GtkEntry* entry, SearchBar* searchBar)
{
	XOJ_CHECK_TYPE_OBJ(searchBar, SearchBar);

	const char* text = gtk_entry_get_text(entry);
	searchBar->search(text);
}

void SearchBar::buttonCloseSearchClicked(GtkButton* button, SearchBar* searchBar)
{
	XOJ_CHECK_TYPE_OBJ(searchBar, SearchBar);

	searchBar->showSearchBar(false);
}

void SearchBar::searchNext()
{
	XOJ_CHECK_TYPE(SearchBar);

	int page = control->getCurrentPageNo();
	int count = control->getDocument()->getPageCount();
	if (count < 2)
	{
		// Nothing to do
		return;
	}

	MainWindow* win = control->getWindow();
	int x = page + 1;
	GtkWidget* searchTextField = win->get("searchTextField");
	const char* text = gtk_entry_get_text(GTK_ENTRY(searchTextField));
	GtkWidget* lbSearchState = win->get("lbSearchState");
	if (*text == 0)
	{
		return;
	}

	if (x >= count)
	{
		x = 0;
	}

	double top = 0;
	int occures = 0;

	while (x != page)
	{

		bool found = control->searchTextOnPage(text, x, &occures, &top);
		if (found)
		{
			control->getScrollHandler()->scrollToPage(x, top);
			char* msg;
			if (occures == 1)
			{
				msg = g_strdup_printf(_("Text once found on page %i"), x + 1);
			}
			else
			{
				msg = g_strdup_printf(_("Text %i times found on page %i"), occures, x + 1);
			}
			gtk_label_set_text(GTK_LABEL(lbSearchState), msg);
			g_free(msg);
			return;
		}

		x++;
		if (x >= count)
		{
			x = 0;
		}
	}

	gtk_label_set_text(GTK_LABEL(lbSearchState), _("Text not found, searched on all pages"));
}

void SearchBar::searchPrevious()
{
	XOJ_CHECK_TYPE(SearchBar);

	int page = control->getCurrentPageNo();
	int count = control->getDocument()->getPageCount();
	if (count < 2)
	{
		// Nothing to do
		return;
	}

	MainWindow* win = control->getWindow();
	int x = page - 1;
	GtkWidget* searchTextField = win->get("searchTextField");
	const char* text = gtk_entry_get_text(GTK_ENTRY(searchTextField));
	GtkWidget* lbSearchState = win->get("lbSearchState");
	if (*text == 0)
	{
		return;
	}

	if (x < 0)
	{
		x = count - 1;
	}

	double top = 0;
	int occures = 0;

	while (x != page)
	{

		bool found = control->searchTextOnPage(text, x, &occures, &top);
		if (found)
		{
			control->getScrollHandler()->scrollToPage(x, top);
			char* msg;
			if (occures == 1)
			{
				msg = g_strdup_printf(_("Text once found on page %i"), x + 1);
			}
			else
			{
				msg = g_strdup_printf(_("Text %i times found on page %i"), occures, x + 1);
			}
			gtk_label_set_text(GTK_LABEL(lbSearchState), msg);
			g_free(msg);
			return;
		}

		x--;
		if (x < 0)
		{
			x = count - 1;
		}
	}

	gtk_label_set_text(GTK_LABEL(lbSearchState),
					_("Text not found, searched on all pages"));
}

void SearchBar::buttonNextSearchClicked(GtkButton* button, SearchBar* searchBar)
{
	XOJ_CHECK_TYPE_OBJ(searchBar, SearchBar);

	searchBar->searchNext();
}

void SearchBar::buttonPreviousSearchClicked(GtkButton* button, SearchBar* searchBar)
{
	XOJ_CHECK_TYPE_OBJ(searchBar, SearchBar);

	searchBar->searchPrevious();
}

void SearchBar::showSearchBar(bool show)
{
	XOJ_CHECK_TYPE(SearchBar);

	MainWindow* win = control->getWindow();
	GtkWidget* searchBar = win->get("searchBar");

	if (show)
	{
		GtkWidget* searchTextField = win->get("searchTextField");
		gtk_widget_grab_focus(searchTextField);
		gtk_widget_show_all(searchBar);
	}
	else
	{
		gtk_widget_hide(searchBar);
		for (int i = control->getDocument()->getPageCount() - 1; i >= 0; i--)
		{
			control->searchTextOnPage("", i, NULL, NULL);
		}
	}
}
