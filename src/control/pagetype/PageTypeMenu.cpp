#include "PageTypeMenu.h"

#include "PageTypeHandler.h"

#include "control/settings/PageTemplateSettings.h"
#include "control/settings/Settings.h"
#include "view/background/MainBackgroundPainter.h"

#include <i18n.h>

PageTypeMenuChangeListener::~PageTypeMenuChangeListener() {}


#define PREVIEW_COLUMNS 3


PageTypeMenu::PageTypeMenu(PageTypeHandler* types, Settings* settings, bool showPreview, bool showSpecial)
 : showSpecial(showSpecial),
   menu(gtk_menu_new()),
   types(types),
   settings(settings),
   ignoreEvents(false),
   listener(NULL),
   menuX(0),
   menuY(0),
   backgroundPainter(NULL),
   showPreview(showPreview)
{
	XOJ_INIT_TYPE(PageTypeMenu);

	initDefaultMenu();
	loadDefaultPage();
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
	setSelected(model.getPageInsertType());
}

cairo_surface_t* PageTypeMenu::createPreviewImage(PageType pt)
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	int previewWidth = 100;
	int previewHeight = 141;
	double zoom = 0.5;

	PageRef page = new XojPage(previewWidth / zoom, previewHeight / zoom);

	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, previewWidth, previewHeight);
	cairo_t* cr = cairo_create(surface);
	cairo_scale(cr, zoom, zoom);

	backgroundPainter->paint(pt, cr, page);

	cairo_identity_matrix(cr);

	cairo_set_line_width(cr, 2);
	cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
	cairo_move_to(cr, 0, 0);
	cairo_line_to(cr, previewWidth, 0);
	cairo_line_to(cr, previewWidth, previewHeight);
	cairo_line_to(cr, 0, previewHeight);
	cairo_line_to(cr, 0, 0);
	cairo_stroke(cr);

	cairo_destroy(cr);
	return surface;
}

void PageTypeMenu::addMenuEntry(PageTypeInfo* t)
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	bool special = t->page.isSpecial();
	bool showImg = !special && showPreview;

	GtkWidget* entry = NULL;
	if (showImg)
	{
		cairo_surface_t* img = createPreviewImage(t->page);
		GtkWidget* preview = gtk_image_new_from_surface(img);
		entry = gtk_check_menu_item_new();

		GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

		gtk_container_add(GTK_CONTAINER(box), preview);
		gtk_container_add(GTK_CONTAINER(box), gtk_label_new(t->name.c_str()));

		gtk_container_add(GTK_CONTAINER(entry), box);
		gtk_widget_show_all(entry);
	}
	else
	{
		entry = gtk_check_menu_item_new_with_label(t->name.c_str());
		gtk_widget_show(entry);
		gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(entry), true);
	}

	if (showPreview)
	{
		if (special)
		{
			if (menuX != 0)
			{
				menuX = 0;
				menuY++;
			}

			gtk_menu_attach(GTK_MENU(menu), entry, menuX, menuX + PREVIEW_COLUMNS, menuY, menuY + 1);
			menuY++;
		}
		else
		{
			gtk_menu_attach(GTK_MENU(menu), entry, menuX, menuX + 1, menuY, menuY + 1);
			menuX++;
			if (menuX >= PREVIEW_COLUMNS)
			{
				menuX = 0;
				menuY++;
			}
		}
	}
	else
	{
		gtk_container_add(GTK_CONTAINER(menu), entry);
	}

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

	if (listener != NULL)
	{
		listener->pageSelected(t);
	}
}

void PageTypeMenu::setSelected(PageType selected)
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	for (MenuCallbackInfo& info : menuInfos)
	{
		if (info.info->page == selected)
		{
			entrySelected(info.info);
			break;
		}
	}
}

void PageTypeMenu::setListener(PageTypeMenuChangeListener* listener)
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	this->listener = listener;
}

void PageTypeMenu::hideCopyPage()
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	for (MenuCallbackInfo& info : menuInfos)
	{
		if (info.info->page.format == ":copy")
		{
			gtk_widget_hide(info.entry);
			break;
		}
	}
}

void PageTypeMenu::initDefaultMenu()
{
	XOJ_CHECK_TYPE(PageTypeMenu);

	this->backgroundPainter = new MainBackgroundPainter();
	this->backgroundPainter->setLineWidthFactor(2);

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

			if (showPreview)
			{
				if (menuX != 0)
				{
					menuX = 0;
					menuY++;
				}

				gtk_menu_attach(GTK_MENU(menu), separator, menuX, menuX + PREVIEW_COLUMNS, menuY, menuY + 1);
				menuY++;
			}
			else
			{
				gtk_container_add(GTK_CONTAINER(menu), separator);
			}
		}
		addMenuEntry(t);
	}

	delete this->backgroundPainter;
	this->backgroundPainter = NULL;
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
