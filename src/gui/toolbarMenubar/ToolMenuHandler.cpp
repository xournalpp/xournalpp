#include "ToolMenuHandler.h"

#include "FontButton.h"
#include "MenuItem.h"
#include "ToolButton.h"
#include "ToolDrawCombocontrol.h"
#include "ToolSelectCombocontrol.h"
#include "ToolPageLayer.h"
#include "ToolPageSpinner.h"
#include "ToolZoomSlider.h"

#include "control/Actions.h"
#include "control/Control.h"
#include "control/PageBackgroundChangeController.h"
#include "gui/ToolitemDragDrop.h"
#include "model/ToolbarData.h"
#include "model/ToolbarModel.h"

#include <StringUtils.h>

#include <config.h>
#include <config-features.h>
#include <i18n.h>

ToolMenuHandler::ToolMenuHandler(Control* control, GladeGui* gui, GtkWindow* parent)
{
	XOJ_INIT_TYPE(ToolMenuHandler);

	this->parent = parent;
	this->control = control;
	this->listener = control;
	this->zoom = control->getZoomControl();
	this->gui = gui;
	this->toolHandler = control->getToolHandler();
	this->undoButton = NULL;
	this->redoButton = NULL;
	this->toolPageSpinner = NULL;
	this->toolPageLayer = NULL;
	this->tbModel = new ToolbarModel();

	// still owned by Control
	this->newPageType = control->getNewPageType();
	this->newPageType->addApplyBackgroundButton(control->getPageBackgroundChangeController(), false);

	// still owned by Control
	this->pageBackgroundChangeController = control->getPageBackgroundChangeController();

	if (control->getSettings()->isDarkTheme())
	{
		gui->setThemePath("dark");
	}

	initToolItems();
}

ToolMenuHandler::~ToolMenuHandler()
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	delete this->tbModel;
	this->tbModel = NULL;

	// Owned by control
	this->pageBackgroundChangeController = NULL;

	// Owned by control
	this->newPageType = NULL;

	for (MenuItem* it : this->menuItems)
	{
		delete it;
		it = NULL;
	}

	freeDynamicToolbarItems();

	for (AbstractToolItem* it : this->toolItems)
	{
		delete it;
		it = NULL;
	}

	XOJ_RELEASE_TYPE(ToolMenuHandler);
}

void ToolMenuHandler::freeDynamicToolbarItems()
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	for (AbstractToolItem* it : this->toolItems)
	{
		it->setUsed(false);
	}

	for (ColorToolItem* it : this->toolbarColorItems)
	{
		delete it;
	}
	this->toolbarColorItems.clear();
}

void ToolMenuHandler::unloadToolbar(GtkWidget* toolbar)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	for (int i = gtk_toolbar_get_n_items(GTK_TOOLBAR(toolbar)) - 1; i >= 0; i--)
	{
		GtkToolItem* tbItem = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), i);
		gtk_container_remove(GTK_CONTAINER(toolbar), GTK_WIDGET(tbItem));
	}

	gtk_widget_hide(toolbar);
}

void ToolMenuHandler::load(ToolbarData* d, GtkWidget* toolbar, const char* toolbarName, bool horizontal)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	int count = 0;

	for (ToolbarEntry* e : d->contents)
	{
		if (e->getName() == toolbarName)
		{
			for (ToolbarItem* dataItem : e->getItems())
			{
				string name = dataItem->getName();

				if (name == "SEPARATOR")
				{
					GtkToolItem* it = gtk_separator_tool_item_new();
					gtk_widget_show(GTK_WIDGET(it));
					gtk_toolbar_insert(GTK_TOOLBAR(toolbar), it, -1);

					ToolitemDragDrop::attachMetadata(GTK_WIDGET(it), dataItem->getId(), TOOL_ITEM_SEPARATOR);

					continue;
				}

				if (name == "SPACER")
				{
					GtkToolItem* toolItem = gtk_separator_tool_item_new();
					gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolItem), false);
					gtk_tool_item_set_expand(toolItem, true);
					gtk_widget_show(GTK_WIDGET(toolItem));
					gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolItem, -1);

					ToolitemDragDrop::attachMetadata(GTK_WIDGET(toolItem), dataItem->getId(), TOOL_ITEM_SPACER);

					continue;
				}
				if (StringUtils::startsWith(name, "COLOR(") && name.length() == 15)
				{
					string color = name.substr(6, 8);
					if (!StringUtils::startsWith(color, "0x"))
					{
						g_warning("Toolbar:COLOR(...) has to start with 0x, get color: %s", color.c_str());
						continue;
					}
					count++;

					color = color.substr(2);
					gint c = g_ascii_strtoll(color.c_str(), NULL, 16);

					ColorToolItem* item = new ColorToolItem(listener, toolHandler, this->parent, c);
					this->toolbarColorItems.push_back(item);

					GtkToolItem* it = item->createItem(horizontal);
					gtk_widget_show_all(GTK_WIDGET(it));
					gtk_toolbar_insert(GTK_TOOLBAR(toolbar), it, -1);

					ToolitemDragDrop::attachMetadataColor(GTK_WIDGET(it), dataItem->getId(), c, item);

					continue;
				}

				bool found = false;
				for (AbstractToolItem* item : this->toolItems)
				{
					if (name == item->getId())
					{
						if (item->isUsed())
						{
							g_warning("You can use the toolbar item \"%s\" only once!", item->getId().c_str());
							found = true;
							continue;
						}
						item->setUsed(true);

						count++;
						GtkToolItem* it = item->createItem(horizontal);
						gtk_widget_show_all(GTK_WIDGET(it));
						gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(it), -1);

						ToolitemDragDrop::attachMetadata(GTK_WIDGET(it), dataItem->getId(), item);

						found = true;
						break;
					}
				}
				if (!found)
				{
					g_warning("Toolbar item \"%s\" not found!", name.c_str());
				}
			}

			break;
		}
	}

	if (count == 0)
	{
		gtk_widget_hide(toolbar);
	}
	else
	{
		gtk_widget_show(toolbar);
	}
}

void ToolMenuHandler::removeColorToolItem(AbstractToolItem* it)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	g_return_if_fail(it != NULL);
	for (unsigned int i = 0; i < this->toolbarColorItems.size(); i++)
	{
		if (this->toolbarColorItems[i] == it)
		{
			this->toolbarColorItems.erase(this->toolbarColorItems.begin() + i);
			break;
		}
	}
	delete (ColorToolItem*) it;
}

void ToolMenuHandler::addColorToolItem(AbstractToolItem* it)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	g_return_if_fail(it != NULL);
	this->toolbarColorItems.push_back((ColorToolItem*) it);
}

void ToolMenuHandler::setTmpDisabled(bool disabled)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	for (AbstractToolItem* it : this->toolItems)
	{
		it->setTmpDisabled(disabled);
	}

	for (MenuItem* it : this->menuItems)
	{
		it->setTmpDisabled(disabled);
	}

	for (ColorToolItem* it : this->toolbarColorItems)
	{
		it->setTmpDisabled(disabled);
	}

	GtkWidget* menuViewSidebarVisible = gui->get("menuViewSidebarVisible");
	gtk_widget_set_sensitive(menuViewSidebarVisible, !disabled);
}

void ToolMenuHandler::addToolItem(AbstractToolItem* it)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	this->toolItems.push_back(it);
}

void ToolMenuHandler::registerMenupoint(GtkWidget* widget, ActionType type, ActionGroup group)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	this->menuItems.push_back(new MenuItem(listener, widget, type, group));
}

void ToolMenuHandler::initPenToolItem()
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	ToolButton* tbPen = new ToolButton(listener, gui, "PEN", ACTION_TOOL_PEN, GROUP_TOOL, true, "tool_pencil.svg", _("Pen"));

	registerMenupoint(tbPen->registerPopupMenuEntry(_("standard"), "line-style-plain.svg"),
			ACTION_TOOL_LINE_STYLE_PLAIN, GROUP_LINE_STYLE);

	registerMenupoint(tbPen->registerPopupMenuEntry(_("dashed"), "line-style-dash.svg"),
			ACTION_TOOL_LINE_STYLE_DASH, GROUP_LINE_STYLE);

	registerMenupoint(tbPen->registerPopupMenuEntry(_("dash-/ doted"), "line-style-dash-dot.svg"),
			ACTION_TOOL_LINE_STYLE_DASH_DOT, GROUP_LINE_STYLE);

	registerMenupoint(tbPen->registerPopupMenuEntry(_("dotted"), "line-style-dot.svg"),
			ACTION_TOOL_LINE_STYLE_DOT, GROUP_LINE_STYLE);

	addToolItem(tbPen);
}

void ToolMenuHandler::initEraserToolItem()
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	ToolButton* tbEraser = new ToolButton(listener, gui, "ERASER", ACTION_TOOL_ERASER, GROUP_TOOL, true,
										  "tool_eraser.svg", _("Eraser"));

	registerMenupoint(tbEraser->registerPopupMenuEntry(_("standard")), ACTION_TOOL_ERASER_STANDARD, GROUP_ERASER_MODE);
	registerMenupoint(tbEraser->registerPopupMenuEntry(_("whiteout")), ACTION_TOOL_ERASER_WHITEOUT, GROUP_ERASER_MODE);
	registerMenupoint(tbEraser->registerPopupMenuEntry(_("delete stroke")), ACTION_TOOL_ERASER_DELETE_STROKE, GROUP_ERASER_MODE);

	addToolItem(tbEraser);
}

void ToolMenuHandler::signalConnectCallback(GtkBuilder* builder, GObject* object, const gchar* signalName,
		const gchar* handlerName, GObject* connectObject, GConnectFlags flags, ToolMenuHandler* self)
{
	XOJ_CHECK_TYPE_OBJ(self, ToolMenuHandler);

	string actionName = handlerName;
	string groupName = "";

	size_t pos = actionName.find(':');
	if (pos != string::npos)
	{
		groupName = actionName.substr(pos + 1);
		actionName = actionName.substr(0, pos);
	}

	ActionGroup group = GROUP_NOGROUP;
	ActionType action = ActionType_fromString(actionName);

	if (action == ACTION_NONE)
	{
		g_error("Unknown action name from glade file: «%s» / «%s»", signalName, handlerName);
		return;
	}

	if (groupName != "")
	{
		group = ActionGroup_fromString(groupName);
	}

	if (GTK_IS_MENU_ITEM(object))
	{
		for (AbstractToolItem* it : self->toolItems)
		{
			if (it->getActionType() == action)
			{
				// There is already a toolbar item -> attach menu to it
				it->setMenuItem(GTK_WIDGET(object));
				return;
			}
		}

		// There is no toolbar item -> register the menu only
		self->registerMenupoint(GTK_WIDGET(object), action, group);
	}
	else
	{
		g_error("Unsupported signal handler from glade file: «%s» / «%s»", signalName, handlerName);
	}
}

// Use GTK Stock icon
#define ADD_STOCK_ITEM(name, action, stockIcon, text) addToolItem(new ToolButton(listener, name, action, stockIcon, text))

// Use Custom loading Icon
#define ADD_CUSTOM_ITEM(name, action, icon, text) addToolItem(new ToolButton(listener, gui, name, action, icon, text))

// Use Custom loading Icon, toggle item
// switchOnly: You can select pen, eraser etc. but you cannot unselect pen.
#define ADD_CUSTOM_ITEM_TGL(name, action, group, switchOnly, icon, text) addToolItem(new ToolButton(listener, gui, name, action, group, switchOnly, icon, text))

void ToolMenuHandler::initToolItems()
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	// Items ordered by menu, if possible.
	// There are some entries which are not available in the menu, like the Zoom slider
	// All menu items without tool icon are not listed here - they are connected by Glade Signals

	// Menu File
	// ************************************************************************

	ADD_STOCK_ITEM("NEW", ACTION_NEW, "document-new", _("New Xournal"));
	ADD_STOCK_ITEM("OPEN", ACTION_OPEN, "document-open", _("Open file"));
	ADD_STOCK_ITEM("SAVE", ACTION_SAVE, "document-save", _("Save"));

	// Menu Edit
	// ************************************************************************

	// Undo / Redo Texts are updated from code, therefore a reference is hold for this items
	undoButton = new ToolButton(listener, "UNDO", ACTION_UNDO, "edit-undo", _("Undo"));
	redoButton = new ToolButton(listener, "REDO", ACTION_REDO, "edit-redo", _("Redo"));
	addToolItem(undoButton);
	addToolItem(redoButton);

	ADD_STOCK_ITEM("CUT", ACTION_CUT, "edit-cut", _("Cut"));
	ADD_STOCK_ITEM("COPY", ACTION_COPY, "edit-copy", _("Copy"));
	ADD_STOCK_ITEM("PASTE", ACTION_PASTE, "edit-paste", _("Paste"));

	ADD_STOCK_ITEM("SEARCH", ACTION_SEARCH, "edit-find", _("Search"));

	ADD_STOCK_ITEM("DELETE", ACTION_DELETE, "edit-delete", _("Delete"));

	// Icon snapping.svg made by www.freepik.com from www.flaticon.com
	ADD_CUSTOM_ITEM_TGL("ROTATION_SNAPPING", ACTION_ROTATION_SNAPPING, GROUP_SNAPPING, false, "snapping.svg", _("Rotation Snapping"));
	ADD_CUSTOM_ITEM_TGL("GRID_SNAPPING", ACTION_GRID_SNAPPING, GROUP_GRID_SNAPPING, false, "grid_snapping.svg", _("Grid Snapping"));

	// Menu View
	// ************************************************************************

	ADD_CUSTOM_ITEM_TGL("TWO_PAGES", ACTION_VIEW_TWO_PAGES, GROUP_TWOPAGES, false, "showtwopages.svg", _("Two pages"));
	ADD_CUSTOM_ITEM_TGL("PRESENTATION_MODE", ACTION_VIEW_PRESENTATION_MODE, GROUP_PRESENTATION_MODE, false, "showtwopages.svg", _("Presentation mode"));
	ADD_CUSTOM_ITEM_TGL("FULLSCREEN", ACTION_FULLSCREEN, GROUP_FULLSCREEN, false, "fullscreen.svg", _("Toggle fullscreen"));

	ADD_STOCK_ITEM("ZOOM_OUT", ACTION_ZOOM_OUT, "zoom-out", _("Zoom out"));
	ADD_STOCK_ITEM("ZOOM_IN", ACTION_ZOOM_IN, "zoom-in", _("Zoom in"));
	ADD_STOCK_ITEM("ZOOM_FIT", ACTION_ZOOM_FIT, "zoom-fit-best", _("Zoom fit to screen"));
	ADD_STOCK_ITEM("ZOOM_100", ACTION_ZOOM_100, "zoom-original", _("Zoom to 100%"));

	// Menu Navigation
	// ************************************************************************

	ADD_STOCK_ITEM("GOTO_FIRST", ACTION_GOTO_FIRST, "go-first", _("Go to first page"));
	ADD_STOCK_ITEM("GOTO_BACK", ACTION_GOTO_BACK, "go-previous", _("Back"));
	ADD_CUSTOM_ITEM("GOTO_PAGE", ACTION_GOTO_PAGE, "goto.svg", _("Go to page"));
	ADD_STOCK_ITEM("GOTO_NEXT", ACTION_GOTO_NEXT, "go-next", _("Next"));
	ADD_STOCK_ITEM("GOTO_LAST", ACTION_GOTO_LAST, "go-last", _("Go to last page"));

	ADD_STOCK_ITEM("GOTO_PREVIOUS_LAYER", ACTION_GOTO_PREVIOUS_LAYER, "go-previous", _("Go to previous layer"));
	ADD_STOCK_ITEM("GOTO_NEXT_LAYER", ACTION_GOTO_NEXT_LAYER, "go-next", _("Go to next layer"));
	ADD_STOCK_ITEM("GOTO_TOP_LAYER", ACTION_GOTO_TOP_LAYER, "go-top", _("Go to top layer"));

	ADD_CUSTOM_ITEM("GOTO_NEXT_ANNOTATED_PAGE", ACTION_GOTO_NEXT_ANNOTATED_PAGE, "nextAnnotatedPage.svg", _("Next annotated page"));

	// Menu Journal
	// ************************************************************************

	ToolButton* tbInsertNewPage = new ToolButton(listener, gui, "INSERT_NEW_PAGE", ACTION_NEW_PAGE_AFTER, "addPage.svg", _("Insert page"));
	addToolItem(tbInsertNewPage);
	tbInsertNewPage->setPopupMenu(this->newPageType->getMenu());

	ADD_CUSTOM_ITEM("DELETE_CURRENT_PAGE", ACTION_DELETE_PAGE, "delPage.svg", _("Delete current page"));

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(gui->get("menuJournalPaperBackground")), pageBackgroundChangeController->getMenu());

	// Menu Tool
	// ************************************************************************

	initPenToolItem();
	initEraserToolItem();

	ADD_CUSTOM_ITEM_TGL("HILIGHTER", ACTION_TOOL_HILIGHTER, GROUP_TOOL, true, "tool_highlighter.svg", _("Highlighter"));
	ADD_CUSTOM_ITEM_TGL("TEXT", ACTION_TOOL_TEXT, GROUP_TOOL, true, "tool_text.svg", _("Text"));
	ADD_CUSTOM_ITEM_TGL("IMAGE", ACTION_TOOL_IMAGE, GROUP_TOOL, true, "tool_image.svg", _("Image"));
	ADD_CUSTOM_ITEM("DEFAULT_TOOL", ACTION_TOOL_DEFAULT, "default.svg", _("Default Tool"));

	ADD_CUSTOM_ITEM_TGL("SHAPE_RECOGNIZER", ACTION_SHAPE_RECOGNIZER, GROUP_RULER, false, "shape_recognizer.svg", _("Shape Recognizer"));
	ADD_CUSTOM_ITEM_TGL("DRAW_RECTANGLE", ACTION_TOOL_DRAW_RECT, GROUP_RULER, false, "rect-draw.svg", _("Draw Rectangle"));
	ADD_CUSTOM_ITEM_TGL("DRAW_CIRCLE", ACTION_TOOL_DRAW_CIRCLE, GROUP_RULER, false, "circle-draw.svg", _("Draw Circle"));
	ADD_CUSTOM_ITEM_TGL("DRAW_ARROW", ACTION_TOOL_DRAW_ARROW, GROUP_RULER, false, "arrow-draw.svg", _("Draw Arrow"));
	ADD_CUSTOM_ITEM_TGL("DRAW_COORDINATE_SYSTEM", ACTION_TOOL_DRAW_COORDINATE_SYSTEM, GROUP_RULER, false, "coordinate-system-draw.svg", _("Draw coordinate system"));
	ADD_CUSTOM_ITEM_TGL("RULER", ACTION_RULER, GROUP_RULER, false, "ruler.svg", _("Ruler"));

	ADD_CUSTOM_ITEM_TGL("SELECT_REGION", ACTION_TOOL_SELECT_REGION, GROUP_TOOL, true, "lasso.svg", _("Select Region"));
	ADD_CUSTOM_ITEM_TGL("SELECT_RECTANGLE", ACTION_TOOL_SELECT_RECT, GROUP_TOOL, true, "rect-select.svg", _("Select Rectangle"));
	ADD_CUSTOM_ITEM_TGL("SELECT_OBJECT", ACTION_TOOL_SELECT_OBJECT, GROUP_TOOL, true, "object-select.svg", _("Select Object"));
	ADD_CUSTOM_ITEM_TGL("VERTICAL_SPACE", ACTION_TOOL_VERTICAL_SPACE, GROUP_TOOL, true, "stretch.svg", _("Vertical Space"));
	ADD_CUSTOM_ITEM_TGL("PLAY_OBJECT", ACTION_TOOL_PLAY_OBJECT, GROUP_TOOL, true, "object-play.svg", _("Play Object"));
	ADD_CUSTOM_ITEM_TGL("HAND", ACTION_TOOL_HAND, GROUP_TOOL, true, "hand.svg", _("Hand"));

	fontButton = new FontButton(listener, gui, "SELECT_FONT", ACTION_FONT_BUTTON_CHANGED, _("Select Font"));
	addToolItem(fontButton);

	ADD_CUSTOM_ITEM_TGL("RECSTOP", ACTION_RECSTOP, GROUP_REC, false, "rec.svg", _("Rec / Stop"));

	// Menu Help
	// ************************************************************************
	// All tools are registered by the Glade Signals


	///////////////////////////////////////////////////////////////////////////


	// Footer tools
	// ************************************************************************
	toolPageSpinner = new ToolPageSpinner(gui, listener, "PAGE_SPIN", ACTION_FOOTER_PAGESPIN);
	addToolItem(toolPageSpinner);

	ToolZoomSlider* toolZoomSlider = new ToolZoomSlider(listener, "ZOOM_SLIDER", ACTION_FOOTER_ZOOM_SLIDER, zoom);
	addToolItem(toolZoomSlider);

	toolPageLayer = new ToolPageLayer(control->getLayerController(), gui, listener, "LAYER", ACTION_FOOTER_LAYER);
	addToolItem(toolPageLayer);

	ADD_CUSTOM_ITEM_TGL("TOOL_FILL", ACTION_TOOL_FILL, GROUP_FILL, false, "fill.svg", _("Fill"));


	// Non-menu items
	// ************************************************************************

	// Color item - not in the menu
	addToolItem(new ColorToolItem(listener, toolHandler, this->parent, 0xff0000, true));

	addToolItem(new ToolSelectCombocontrol(this, listener, gui, "SELECT"));
	addToolItem(new ToolDrawCombocontrol(this, listener, gui, "DRAW"));

	// General tool configuration - working for every tool which support it
	ADD_CUSTOM_ITEM_TGL("FINE", ACTION_SIZE_FINE, GROUP_SIZE, true, "thickness_thin.svg", _("Thin"));
	ADD_CUSTOM_ITEM_TGL("MEDIUM", ACTION_SIZE_MEDIUM, GROUP_SIZE, true, "thickness_medium.svg", _("Medium"));
	ADD_CUSTOM_ITEM_TGL("THICK", ACTION_SIZE_THICK, GROUP_SIZE, true, "thickness_thick.svg", _("Thick"));


	// now connect all Glade Signals
	gtk_builder_connect_signals_full(gui->getBuilder(), (GtkBuilderConnectFunc)signalConnectCallback, this);
}

void ToolMenuHandler::setFontButtonFont(XojFont& font)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	this->fontButton->setFont(font);
}

XojFont ToolMenuHandler::getFontButtonFont()
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	return this->fontButton->getFont();
}

void ToolMenuHandler::showFontSelectionDlg()
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	this->fontButton->showFontDialog();
}

void ToolMenuHandler::setUndoDescription(string description)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	this->undoButton->updateDescription(description);
	gtk_menu_item_set_label(GTK_MENU_ITEM(gui->get("menuEditUndo")), description.c_str());
}

void ToolMenuHandler::setRedoDescription(string description)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	this->redoButton->updateDescription(description);
	gtk_menu_item_set_label(GTK_MENU_ITEM(gui->get("menuEditRedo")), description.c_str());
}

SpinPageAdapter* ToolMenuHandler::getPageSpinner()
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	return this->toolPageSpinner->getPageSpinner();
}

void ToolMenuHandler::setPageText(string text)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	this->toolPageSpinner->setText(text);
}

ToolbarModel* ToolMenuHandler::getModel()
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	return this->tbModel;
}

bool ToolMenuHandler::isColorInUse(int color)
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	for (ColorToolItem* it : this->toolbarColorItems)
	{
		if (it->getColor() == color)
		{
			return true;
		}
	}

	return false;
}

vector<AbstractToolItem*>* ToolMenuHandler::getToolItems()
{
	XOJ_CHECK_TYPE(ToolMenuHandler);

	return &this->toolItems;
}
