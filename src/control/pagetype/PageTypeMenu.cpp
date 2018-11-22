#include "PageTypeMenu.h"

#include "PageTypeHandler.h"

#include <i18n.h>


PageTypeMenu::PageTypeMenu(PageTypeHandler* types)
 : menu(gtk_menu_new()),
   types(types)
{
	XOJ_INIT_TYPE(PageTypeMenu);

	initDefaultMenu();
}

PageTypeMenu::~PageTypeMenu()
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	/**
	 * The menu is used from the GUI
	 * Therefore the menu is not freed here, this will be done in the GUI
	 */
	menu = NULL;

	XOJ_RELEASE_TYPE(PageTypeMenu);
}

void PageTypeMenu::addMenuEntry(PageTypeInfo* t)
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	GtkWidget* entry = gtk_check_menu_item_new_with_label(t->name.c_str());
	gtk_widget_show(entry);
	gtk_container_add(GTK_CONTAINER(menu), entry);
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(entry), true);
}

void PageTypeMenu::initDefaultMenu()
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	bool special = false;
	for (PageTypeInfo* t : this->types->getPageTypes())
	{
		if (!special && t->page.format.at(0) == ':')
		{
			special = true;
			GtkWidget* separator = gtk_separator_menu_item_new();
			gtk_widget_show(separator);
			gtk_container_add(GTK_CONTAINER(menu), separator);
		}
		addMenuEntry(t);
	}
}

GtkWidget* PageTypeMenu::getMenu()
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	return menu;
}
