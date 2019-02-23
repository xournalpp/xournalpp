#include "MainWindowToolbarMenu.h"
#include "MainWindow.h"

#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include "gui/toolbarMenubar/model/ToolbarData.h"
#include "gui/toolbarMenubar/model/ToolbarModel.h"

class MenuSelectToolbarData
{
public:
	MenuSelectToolbarData(MainWindowToolbarMenu* tbm, GtkWidget* item, ToolbarData* d, int index)
	{
		this->tbm = tbm;
		this->item = item;
		this->d = d;
		this->index = index;
	}

	MainWindowToolbarMenu* tbm;
	GtkWidget* item;
	ToolbarData* d;
	int index;
};


MainWindowToolbarMenu::MainWindowToolbarMenu(MainWindow* win)
 : win(win)
{
	XOJ_INIT_TYPE(MainWindowToolbarMenu);
}

MainWindowToolbarMenu::~MainWindowToolbarMenu()
{
	XOJ_CHECK_TYPE(MainWindowToolbarMenu);

	menuitems.clear();
	freeToolMenu();

	XOJ_RELEASE_TYPE(MainWindowToolbarMenu);
}

void MainWindowToolbarMenu::freeToolMenu()
{
	XOJ_CHECK_TYPE(MainWindowToolbarMenu);

	for (MenuSelectToolbarData* data : toolbarMenuData)
	{
		delete data;
	}
	this->toolbarMenuData.clear();
}

void MainWindowToolbarMenu::setTmpDisabled(bool disabled)
{
	XOJ_CHECK_TYPE(MainWindowToolbarMenu);

	for (MenuSelectToolbarData* data : this->toolbarMenuData)
	{
		gtk_widget_set_sensitive(data->item, !disabled);
	}
}

void MainWindowToolbarMenu::selectToolbar(Settings* settings, ToolMenuHandler* toolbar)
{
	XOJ_CHECK_TYPE(MainWindowToolbarMenu);

	selectedToolbar = NULL;

	string selectedId = settings->getSelectedToolbar();

	for (ToolbarData* d : *toolbar->getModel()->getToolbars())
	{
		if (selectedToolbar == NULL)
		{
			selectedToolbar = d;
		}

		if (selectedId == d->getId())
		{
			selectedToolbar = d;
			break;
		}
	}
}

ToolbarData* MainWindowToolbarMenu::getSelectedToolbar()
{
	XOJ_CHECK_TYPE(MainWindowToolbarMenu);

	return selectedToolbar;
}

void MainWindowToolbarMenu::removeOldElements(GtkMenuShell* menubar)
{
	XOJ_CHECK_TYPE(MainWindowToolbarMenu);

	for (GtkWidget* w : menuitems)
	{
		gtk_container_remove(GTK_CONTAINER(menubar), w);
	}
	menuitems.clear();

	freeToolMenu();
}

void MainWindowToolbarMenu::tbSelectMenuitemActivated(GtkCheckMenuItem* checkmenuitem, MenuSelectToolbarData* data)
{
	data->tbm->menuClicked(checkmenuitem, data);
}

void MainWindowToolbarMenu::menuClicked(GtkCheckMenuItem* menuitem, MenuSelectToolbarData* data)
{
	XOJ_CHECK_TYPE(MainWindowToolbarMenu);

	if (!gtk_check_menu_item_get_active(menuitem))
	{
		// Ignore disabled menus
		return;
	}

	win->toolbarSelected(data->d);

	for (int i = 0; i < (int)this->toolbarMenuData.size(); i++)
	{
		if (data->index == i)
		{
			continue;
		}

		GtkWidget* w = this->toolbarMenuData[i]->item;

		if (GTK_IS_CHECK_MENU_ITEM(w))
		{
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), false);
		}
	}
}

void MainWindowToolbarMenu::addToolbarMenuEntry(ToolbarData* d, GtkMenuShell* menubar, int& menuPos)
{
	XOJ_CHECK_TYPE(MainWindowToolbarMenu);

	GtkWidget* item = gtk_check_menu_item_new_with_label(d->getName().c_str());
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), true);
	gtk_widget_show(item);
	gtk_menu_shell_insert(menubar, item, menuPos);

	menuitems.push_back(item);

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), d == selectedToolbar);

	MenuSelectToolbarData* data = new MenuSelectToolbarData(this, item, d, toolbarMenuData.size());
	toolbarMenuData.push_back(data);

	g_signal_connect(item, "toggled", G_CALLBACK(tbSelectMenuitemActivated), data);

	if (inPredefinedSection && !d->isPredefined())
	{
		GtkWidget* separator = gtk_separator_menu_item_new();
		gtk_widget_show(separator);
		gtk_menu_shell_insert(menubar, separator, menuPos++);

		inPredefinedSection = false;
		menuitems.push_back(separator);
	}
}

void MainWindowToolbarMenu::updateToolbarMenu(GtkMenuShell* menubar, Settings* settings, ToolMenuHandler* toolbar)
{
	XOJ_CHECK_TYPE(MainWindowToolbarMenu);

	selectToolbar(settings, toolbar);
	removeOldElements(menubar);

	inPredefinedSection = true;

	int menuPos = 0;
	for (ToolbarData* d : *toolbar->getModel()->getToolbars())
	{
		addToolbarMenuEntry(d, menubar, menuPos);
		menuPos++;
	}
}
