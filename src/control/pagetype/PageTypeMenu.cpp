#include "PageTypeMenu.h"

#include "PageTypeHandler.h"

#include "control/settings/PageTemplateSettings.h"
#include "control/settings/Settings.h"

#include <i18n.h>


PageTypeMenu::PageTypeMenu(PageTypeHandler* types, Settings* settings, bool showSpecial)
 : showSpecial(showSpecial),
   menu(gtk_menu_new()),
   types(types),
   settings(settings),
   ignoreEvents(false)
{
	XOJ_INIT_TYPE(PageTypeMenu);

	loadDefaultPage();
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

void PageTypeMenu::loadDefaultPage()
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	PageTemplateSettings model;
	model.parse(settings->getPageTemplate());
	selected = model.getPageInsertType();

	PageTypeInfo* baseMatch = this->types->getPageTypes()[0];
	for (PageTypeInfo* t : this->types->getPageTypes())
	{
		if (t->page.format == selected.format)
		{
			if (t->page.config == selected.config)
			{
				// 100% match
				entrySelected(t);
				return;
			}

			baseMatch = t;
		}
	}

	// None matched 100%
	entrySelected(baseMatch);
}

void PageTypeMenu::addMenuEntry(PageTypeInfo* t)
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	GtkWidget* entry = gtk_check_menu_item_new_with_label(t->name.c_str());
	gtk_widget_show(entry);
	gtk_container_add(GTK_CONTAINER(menu), entry);
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(entry), true);

	MenuCallbackInfo info;
	info.entry = entry;
	info.info = t;
	menuInfos.push_back(info);

	g_signal_connect(entry, "toggled", G_CALLBACK(
		+[](GtkWidget* togglebutton, PageTypeMenu* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, PageTypeMenu);

			if (self->ignoreEvents)
			{
				return;
			}

			for (MenuCallbackInfo& info : self->menuInfos)
			{
				if (info.entry == togglebutton)
				{
					self->entrySelected(info.info);
					break;
				}
			}

		}), this);
}

void PageTypeMenu::entrySelected(PageTypeInfo* t)
{
	ignoreEvents = true;
	for (MenuCallbackInfo& info : menuInfos)
	{
		bool enabled = info.info == t;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(info.entry), enabled);
	}
	ignoreEvents = false;

	selected = t->page;
}

void PageTypeMenu::initDefaultMenu()
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	bool special = false;
	for (PageTypeInfo* t : this->types->getPageTypes())
	{
		if (!showSpecial && t->page.isSpecial())
		{
			continue;
		}

		if (!special && t->page.isSpecial())
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

PageType PageTypeMenu::getSelected()
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	return selected;
}
