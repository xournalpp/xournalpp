#include "SearchBar.h"

#include "control/Control.h"

#include <config.h>
#include <i18n.h>

SearchBar::SearchBar(Control* control)
 : control(control)
{
	MainWindow* win = control->getWindow();

	GtkWidget* close = win->get("buttonCloseSearch");
	g_signal_connect(close, "clicked", G_CALLBACK(buttonCloseSearchClicked), this);

	GtkWidget* next = win->get("btSearchForward");
	GtkWidget* previous = win->get("btSearchBack");
	g_signal_connect(next, "clicked", G_CALLBACK(+[](GtkButton* button, SearchBar* self) {
 self->searchNext();
	                 }),
	                 this);
	g_signal_connect(previous, "clicked", G_CALLBACK(+[](GtkButton* button, SearchBar* self) {
 self->searchPrevious();
	                 }),
	                 this);

	// TODO: When keybindings are implemented, handle previous search keybinding properly
	GtkWidget* searchTextField = win->get("searchTextField");
	g_signal_connect(searchTextField, "search-changed", G_CALLBACK(searchTextChangedCallback), this);
	// Enable next/previous search when pressing Enter / Shift+Enter
	g_signal_connect(searchTextField, "key-press-event",
	                 G_CALLBACK(+[](GtkWidget* entry, GdkEventKey* event, SearchBar* self) {
 if (event->keyval == GDK_KEY_Return)
		                 {
			                 if (event->state & GDK_SHIFT_MASK)
				                 self->searchPrevious();
			                 else
				                 self->searchNext();
			                 // Grab focus again since searching will take away focus
			                 gtk_widget_grab_focus(entry);
			                 return true;
		                 }
		                 return false;
	                 }),
	                 this);

	cssTextFild = gtk_css_provider_new();
	gtk_style_context_add_provider(gtk_widget_get_style_context(win->get("searchTextField")), GTK_STYLE_PROVIDER(cssTextFild), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

SearchBar::~SearchBar()
{
	this->control = nullptr;
}

bool SearchBar::searchTextonCurrentPage(const char* text, int* occures, double* top)
{
	int p = control->getCurrentPageNo();

	return control->searchTextOnPage(text, p, occures, top);
}

void SearchBar::search(const char* text)
{
	MainWindow* win = control->getWindow();
	GtkWidget* lbSearchState = win->get("lbSearchState");

	bool found = true;
	int occures = 0;

	if (*text != 0)
	{
		found = searchTextonCurrentPage(text, &occures, nullptr);
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
		searchTextonCurrentPage("", nullptr, nullptr);
		gtk_label_set_text(GTK_LABEL(lbSearchState), "");
	}

	if (found)
	{
		gtk_css_provider_load_from_data(cssTextFild, "GtkSearchEntry {}", -1, nullptr);
	}
	else
	{
		gtk_css_provider_load_from_data(cssTextFild, "GtkSearchEntry { color: #ff0000; }", -1, nullptr);
	}
}

void SearchBar::searchTextChangedCallback(GtkEntry* entry, SearchBar* searchBar)
{
	const char* text = gtk_entry_get_text(entry);
	searchBar->search(text);
}

void SearchBar::buttonCloseSearchClicked(GtkButton* button, SearchBar* searchBar)
{
	searchBar->showSearchBar(false);
}

void SearchBar::searchNext()
{
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
			gtk_label_set_text(GTK_LABEL(lbSearchState),
				(occures == 1
					? FC(_F("Text found once on page {1}") % (x + 1))
					: FC(_F("Text found {1} times on page {2}") % occures % (x + 1))
				)
			);
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
			gtk_label_set_text(GTK_LABEL(lbSearchState),
				(occures == 1
					? FC(_F("Text found once on page {1}") % (x + 1))
					: FC(_F("Text found {1} times on page {2}") % occures % (x + 1))
				)
			);
			return;
		}

		x--;
		if (x < 0)
		{
			x = count - 1;
		}
	}

	gtk_label_set_text(GTK_LABEL(lbSearchState), _("Text not found, searched on all pages"));
}

void SearchBar::showSearchBar(bool show)
{
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
			control->searchTextOnPage("", i, nullptr, nullptr);
		}
	}
}
