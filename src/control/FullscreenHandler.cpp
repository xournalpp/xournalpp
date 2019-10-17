#include "FullscreenHandler.h"

#include "Control.h"
#include "gui/MainWindow.h"

#include <StringUtils.h>

GtkWidget* gtk_invisible_new();

FullscreenHandler::FullscreenHandler(Settings* settings)
 : settings(settings)
{
}

FullscreenHandler::~FullscreenHandler()
{
}

bool FullscreenHandler::isFullscreen()
{
	return this->fullscreen;
}

void FullscreenHandler::hideWidget(MainWindow* win, string widgetName)
{
	if ("sidebarContents" == widgetName && settings->isSidebarVisible())
	{
		this->sidebarHidden = true;
		win->setSidebarVisible(false);

		return;
	}

	if ("mainMenubar" == widgetName)
	{
		// If the menu is hidden, shortcuts are not working anymore
		// therefore the menu is not hidden, it's displayed, but invisible
		// this costs 1px at the bottom, even if the preferred size is 0px,
		// 1px is used by GTK

		GtkWidget* mainMenubar = win->get("mainMenubar");
		GtkWidget* mainBox = win->get("mainBox");

		if (mainMenubar == nullptr || !gtk_widget_is_visible(mainMenubar))
		{
			// Menu not visible (global menu or something like this)
			return;
		}

		// Remove menu from parent
		gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(mainMenubar)), mainMenubar);

		GtkWidget* fix = gtk_invisible_new();

		gtk_widget_set_size_request(fix, 0, 0);
		gtk_fixed_put(GTK_FIXED(fix), mainMenubar, 0, 0);

		gtk_widget_show(fix);

		gtk_box_pack_end(GTK_BOX(mainBox), fix, false, false, 0);

		menubarHidden = true;

		return;
	}

	return;

	GtkWidget* w = win->get(widgetName);
	if (w == nullptr)
	{
		g_warning("Fullscreen: Try to hide \"%s\", but coulden't find it. Wrong entry in ~/"
				  CONFIG_DIR "/" SETTINGS_XML_FILE "?", widgetName.c_str());
		return;
	}

	if (gtk_widget_get_visible(w))
	{
		gtk_widget_hide(w);
		hiddenFullscreenWidgets.push_back(w);
	}
}

void FullscreenHandler::enableFullscreen(MainWindow* win)
{
	gtk_window_fullscreen((GtkWindow*) *win);

	string hideWidgets = settings->getFullscreenHideElements();
	for (string s : StringUtils::split(hideWidgets, ','))
	{
		hideWidget(win, s);
	}
}

void FullscreenHandler::disableFullscreen(MainWindow* win)
{
	gtk_window_unfullscreen((GtkWindow*) *win);

	for (GtkWidget* w : hiddenFullscreenWidgets)
	{
		gtk_widget_show(w);
	}
	hiddenFullscreenWidgets.clear();

	if (this->sidebarHidden)
	{
		this->sidebarHidden = false;
		win->setSidebarVisible(true);
	}

	if (this->menubarHidden)
	{
		GtkWidget* mainMenubar = win->get("mainMenubar");
		GtkWidget* mainBox = win->get("mainBox");

		GtkWidget* parent = gtk_widget_get_parent(mainMenubar);

		// Remove menu from parent
		gtk_container_remove(GTK_CONTAINER(parent), mainMenubar);
		gtk_box_pack_start(GTK_BOX(mainBox), mainMenubar, false, true, 0);


		GValue value = G_VALUE_INIT;
		g_value_init(&value, G_TYPE_INT);
		g_value_set_int(&value, 0);
		gtk_container_child_set_property(GTK_CONTAINER(mainBox), mainMenubar, "position", &value);

		// not needed, will be recreated next time
		gtk_widget_destroy(parent);

		menubarHidden = false;
	}
}

void FullscreenHandler::setFullscreen(MainWindow* win, bool enabled)
{
	if (enabled)
	{
		enableFullscreen(win);
	}
	else
	{
		disableFullscreen(win);
	}

	this->fullscreen = enabled;
}

///////////////////////////////////////////////////////////////////////////////
// Widget which allow to hide the menu, but let the shortcuts active
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
	GtkFixedClass parent_class;
} GtkInvisibleMenuClass;

typedef struct
{
	GtkFixed widget;
} GtkInvisibleMenu;

static void gtk_invisible_menu_get_preferred_width(GtkWidget *widget, gint *minimum, gint *natural)
{
	*minimum = 0;
	*natural = 0;
}

static void gtk_invisible_menu_get_preferred_height(GtkWidget *widget, gint *minimum, gint *natural)
{
	*minimum = 0;
	*natural = 0;
}

static gboolean gtk_invisible_menu_draw(GtkWidget *widget, cairo_t *cr)
{
	return false;
}

static void gtk_invisible_menu_class_init(GtkInvisibleMenuClass* klass)
{
	GtkWidgetClass* widget_class;

	widget_class = (GtkWidgetClass*) klass;
	widget_class->get_preferred_width = gtk_invisible_menu_get_preferred_width;
	widget_class->get_preferred_height = gtk_invisible_menu_get_preferred_height;
	widget_class->draw = gtk_invisible_menu_draw;
}

GType gtk_invisible_get_type()
{
	static GType gtk_invisible_menu_type = 0;

	if (!gtk_invisible_menu_type)
	{
		static const GTypeInfo gtk_inivisible_menu_info =
		{
			sizeof(GtkInvisibleMenuClass),
			// base initialize
			nullptr,
			// base finalize
			nullptr,
			// class initialize
			(GClassInitFunc) gtk_invisible_menu_class_init,
			// class finalize
			nullptr,
			// class data,
			nullptr,
			// instance size
			sizeof(GtkInvisibleMenu),
			// n_preallocs
			0,
			// instance init
			nullptr,
			// value table
			(const GTypeValueTable*) nullptr
		};

		gtk_invisible_menu_type = g_type_register_static(GTK_TYPE_FIXED,
		                                          "GtkInvisibleMenu",
		                                          &gtk_inivisible_menu_info,
		                                          (GTypeFlags) 0);
	}

	return gtk_invisible_menu_type;
}

GtkWidget* gtk_invisible_new()
{
	return GTK_WIDGET(g_object_new(gtk_invisible_get_type(), nullptr));
}
