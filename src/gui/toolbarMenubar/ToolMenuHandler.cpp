/*
 * Xournal Extended
 *
 * Part of the customizeable toolbars
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glade/glade-xml.h>
#include "../../gettext.h"

#include "ToolItems.h"
#include <config.h>

#include "../../control/Actions.h"
#include "../widgets/SelectColor.h"
#include "ToolMenuHandler.h"
#include "../../toolbar.ini.h"

String ToolbarData::getName() {
	return this->name;
}

void ToolbarData::load(GKeyFile * config, const char * group) {
	gsize length = 0;
	gchar ** keys = g_key_file_get_keys(config, group, &length, NULL);
	if (keys == NULL) {
		return;
	}

	gchar * name = g_key_file_get_locale_string(config, group, "name", NULL, NULL);
	if (name != NULL) {
		this->name = name;
		g_free(name);
	}

	for (gsize i = 0; i < length; i++) {
		if (strcmp(keys[i], "name") == 0 || strncmp(keys[i], "name[", 5) == 0) {
			continue;
		}

		ToolbarEntry e;
		gsize keyLen = 0;
		e.name = keys[i];
		gchar ** list = g_key_file_get_string_list(config, group, keys[i], &keyLen, NULL);

		for (gsize x = 0; x < keyLen; x++) {
			String s = list[x];
			e.entries.push_back(s.trim());
		}

		contents.push_back(e);

		g_strfreev(list);
	}

	g_strfreev(keys);
}

ToolMenuHandler::ToolMenuHandler(ActionHandler * listener, ZoomControl * zoom, GladeGui * gui,
		ToolHandler * toolHandler) {
	this->toolbars = NULL;
	this->toolItems = NULL;
	this->toolbarColorItems = NULL;
	this->menuItems = NULL;
	this->listener = listener;
	this->zoom = zoom;
	this->gui = gui;
	this->toolHandler = toolHandler;
	this->undoButton = NULL;
	this->redoButton = NULL;
	this->toolPageSpinner = NULL;
	this->toolPageLayer = NULL;

	initToolItems();
}

ToolMenuHandler::~ToolMenuHandler() {
	for (GList * l = toolbars; l != NULL; l = l->next) {
		delete (ToolbarData*) l->data;
	}

	g_list_free(toolbars);
	toolbars = NULL;
}

void ToolMenuHandler::parseGroup(GKeyFile * config, const char * group) {
	ToolbarData * data = new ToolbarData();
	data->name = group;

	data->load(config, group);

	this->toolbars = g_list_append(this->toolbars, data);
}

bool ToolMenuHandler::parse(const char * file) {
	GKeyFile * config = g_key_file_new();
	g_key_file_set_list_separator(config, ',');
	if (!g_key_file_load_from_file(config, file, G_KEY_FILE_NONE, NULL)) {
		g_key_file_free(config);
		return false;
	}

	gsize lenght = 0;
	gchar ** groups = g_key_file_get_groups(config, &lenght);

	for (gsize i = 0; i < lenght; i++) {
		if (strcmp(groups[i], "general") == 0) {
			continue;
		}
		parseGroup(config, groups[i]);
	}

	g_strfreev(groups);
	g_key_file_free(config);
	return true;
}

/**
 * A new file should be writen
 */
bool ToolMenuHandler::shouldRecreate() {
	// TODO: version prüfen
	return false;
}

bool ToolMenuHandler::writeDefault(const char * file) {
	FILE * fp = fopen(file, "w");

	if (!fp) {
		return false;
	}

	fwrite(TOOLBAR_INI, 1, strlen(TOOLBAR_INI), fp);

	fclose(fp);
	return true;
}

ListIterator<ToolbarData *> ToolMenuHandler::iterator() {
	return ListIterator<ToolbarData *> (this->toolbars);
}

void ToolMenuHandler::freeToolbar() {
	for (GList * l = toolbarColorItems; l != NULL; l = l->next) {
		delete (ColorToolItem *) l->data;
	}
	g_list_free(toolbarColorItems);
	toolbarColorItems = NULL;

	for (GList * l = this->toolItems; l != NULL; l = l->next) {
		AbstractToolItem * item = (AbstractToolItem *) l->data;
		item->setUsed(false);
	}
}

void ToolMenuHandler::unloadToolbar(GtkWidget * toolbar) {
	for (int i = gtk_toolbar_get_n_items(GTK_TOOLBAR(toolbar)) - 1; i >= 0; i--) {
		GtkToolItem * tbItem = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), i);
		gtk_container_remove(GTK_CONTAINER(toolbar), GTK_WIDGET(tbItem));
	}

	gtk_widget_hide(toolbar);
}

void ToolMenuHandler::load(ToolbarData * d, GtkWidget * toolbar, const char * toolbarName, bool horizontal) {
	std::vector<ToolbarEntry>::iterator it;

	int count = 0;

	for (it = d->contents.begin(); it != d->contents.end(); it++) {
		ToolbarEntry & e = *it;

		if (e.name.equals(toolbarName)) {
			std::vector<String>::iterator itItem;
			for (itItem = e.entries.begin(); itItem != e.entries.end(); itItem++) {
				String name = *itItem;

				if (name.equals("SEPARATOR")) {
					GtkToolItem* toolItem = gtk_tool_item_new();
					gtk_widget_show(GTK_WIDGET(toolItem));
					gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolItem, -1);

					GtkWidget * separator = NULL;
					if (horizontal) {
						separator = gtk_vseparator_new();
					} else {
						separator = gtk_hseparator_new();
					}
					gtk_widget_show(separator);
					gtk_container_add(GTK_CONTAINER (toolItem), separator);

					continue;
				}

				if (name.equals("SPACER")) {
					GtkToolItem* toolItem = gtk_separator_tool_item_new();
					gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolItem), false);
					gtk_tool_item_set_expand(toolItem, true);
					gtk_widget_show(GTK_WIDGET(toolItem));
					gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolItem, -1);

					continue;
				}
				if (name.startsWith("COLOR(") && name.length() == 15) {
					String color = name.substring(6, 8);
					if (!color.startsWith("0x")) {
						g_warning("Toolbar:COLOR(...) has to start with 0x, get color: %s", color.c_str());
						continue;
					}
					count++;

					color = color.substring(2);
					gint c = g_ascii_strtoll(color.c_str(), NULL, 16);

					ColorToolItem * item = new ColorToolItem("", listener, toolHandler, c);
					toolbarColorItems = g_list_append(toolbarColorItems, item);

					GtkToolItem * it = item->createItem(horizontal);
					gtk_widget_show_all(GTK_WIDGET(it));
					gtk_toolbar_insert(GTK_TOOLBAR(toolbar), it, -1);

					continue;
				}

				bool found = false;
				for (GList * l = this->toolItems; l != NULL; l = l->next) {
					AbstractToolItem * item = (AbstractToolItem *) l->data;
					if (name.equals(item->getId())) {
						if (item->isUsed()) {
							g_warning("You can use the toolbar item \"%s\" only once!", item->getId().c_str());
							found = true;
							continue;
						}
						item->setUsed(true);

						count++;
						GtkToolItem * it = item->createItem(horizontal);
						gtk_widget_show_all(GTK_WIDGET(it));
						gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(it), -1);

						found = true;
						break;
					}
				}
				if (!found) {
					g_warning("Toolbar item \"%s\" not found!", name.c_str());
				}
			}

			break;
		}
	}

	if (count == 0) {
		gtk_widget_hide(toolbar);
	} else {
		gtk_widget_show(toolbar);
	}
}

void ToolMenuHandler::setTmpDisabled(bool disabled) {
	for (GList * l = this->toolItems; l != NULL; l = l->next) {
		AbstractToolItem * it = (AbstractToolItem *) l->data;
		it->setTmpDisabled(disabled);
	}
	for (GList * l = this->menuItems; l != NULL; l = l->next) {
		MenuItem * it = (MenuItem *) l->data;
		it->setTmpDisabled(disabled);
	}
	for (GList * l = toolbarColorItems; l != NULL; l = l->next) {
		ColorToolItem * it = (ColorToolItem *) l->data;
		it->setTmpDisabled(disabled);
	}

	GtkWidget * menuViewSidebarVisible = gui->get("menuViewSidebarVisible");
	gtk_widget_set_sensitive(menuViewSidebarVisible, !disabled);

}

void ToolMenuHandler::addToolItem(AbstractToolItem * it) {
	this->toolItems = g_list_append(this->toolItems, it);
}

void ToolMenuHandler::registerMenupoint(GtkWidget * widget, ActionType type) {
	MenuItem * it = new MenuItem(listener, widget, type);
	this->menuItems = g_list_append(this->menuItems, it);
}

void ToolMenuHandler::registerMenupoint(GtkWidget * widget, ActionType type, ActionGroup group) {
	MenuItem * it = new MenuItem(listener, widget, type, group);
	this->menuItems = g_list_append(this->menuItems, it);
}

void ToolMenuHandler::initEraserToolItem() {

	ToolButton * tbEraser = new ToolButton(listener, gui, "ERASER", ACTION_TOOL_ERASER, GROUP_TOOL, "tool_eraser.png",
			_("Eraser"), gui->get("menuToolsEraser"));
	GtkWidget * eraserPopup = gtk_menu_new();

	GtkWidget *eraserPopupStandard = gtk_check_menu_item_new_with_label(_("standard"));
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(eraserPopupStandard), true);
	gtk_widget_show(eraserPopupStandard);
	gtk_container_add(GTK_CONTAINER (eraserPopup), eraserPopupStandard);
	registerMenupoint(gui->get("eraserStandard"), ACTION_TOOL_ERASER_STANDARD, GROUP_ERASER_MODE);
	registerMenupoint(eraserPopupStandard, ACTION_TOOL_ERASER_STANDARD, GROUP_ERASER_MODE);

	GtkWidget *eraserPopupWhiteout = gtk_check_menu_item_new_with_label(_("whiteout"));
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(eraserPopupWhiteout), true);
	gtk_widget_show(eraserPopupWhiteout);
	gtk_container_add(GTK_CONTAINER (eraserPopup), eraserPopupWhiteout);
	registerMenupoint(gui->get("eraserWhiteout"), ACTION_TOOL_ERASER_WHITEOUT, GROUP_ERASER_MODE);
	registerMenupoint(eraserPopupWhiteout, ACTION_TOOL_ERASER_WHITEOUT, GROUP_ERASER_MODE);

	GtkWidget *eraserPopupDeleteStroke = gtk_check_menu_item_new_with_label(_("delete stroke"));
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(eraserPopupDeleteStroke), true);
	gtk_widget_show(eraserPopupDeleteStroke);
	gtk_container_add(GTK_CONTAINER (eraserPopup), eraserPopupDeleteStroke);
	registerMenupoint(gui->get("eraserDeleteStrokes"), ACTION_TOOL_ERASER_DELETE_STROKE, GROUP_ERASER_MODE);
	registerMenupoint(eraserPopupDeleteStroke, ACTION_TOOL_ERASER_DELETE_STROKE, GROUP_ERASER_MODE);

	tbEraser->setPopupMenu(eraserPopup);

	addToolItem(tbEraser);
}

void ToolMenuHandler::initToolItems() {
	addToolItem(new ToolButton(listener, "SAVE", ACTION_SAVE, GTK_STOCK_SAVE, _("Save"), gui->get("menuFileSave")));
	addToolItem(new ToolButton(listener, gui, "NEW", ACTION_NEW, "documentNew.png", _("New Xournal"), gui->get(
			"menuFileNew")));

	addToolItem(new ToolButton(listener, "OPEN", ACTION_OPEN, GTK_STOCK_OPEN, _("Open file"), gui->get("menuFileOpen")));

	addToolItem(new ToolButton(listener, "CUT", ACTION_CUT, GTK_STOCK_CUT, _("Cut"), gui->get("menuEditCut")));
	addToolItem(new ToolButton(listener, "COPY", ACTION_COPY, GTK_STOCK_COPY, _("Copy"), gui->get("menuEditCopy")));
	addToolItem(new ToolButton(listener, "PASTE", ACTION_PASTE, GTK_STOCK_PASTE, _("Paste"), gui->get("menuEditPaste")));

	addToolItem(new ToolButton(listener, "SEARCH", ACTION_SEARCH, GTK_STOCK_FIND, _("Search"), gui->get(
			"menuEditSearch")));

	undoButton = new ToolButton(listener, "UNDO", ACTION_UNDO, GTK_STOCK_UNDO, _("Undo"), gui->get("menuEditUndo"));
	redoButton = new ToolButton(listener, "REDO", ACTION_REDO, GTK_STOCK_REDO, _("Redo"), gui->get("menuEditRedo"));
	addToolItem(undoButton);
	addToolItem(redoButton);

	addToolItem(new ToolButton(listener, "GOTO_FIRST", ACTION_GOTO_FIRST, GTK_STOCK_GOTO_FIRST, _("Go to first page"),
			gui->get("menuViewFirstPage")));
	addToolItem(new ToolButton(listener, "GOTO_BACK", ACTION_GOTO_BACK, GTK_STOCK_GO_BACK, _("Back"), gui->get(
			"menuNavigationPreviousPage")));
	addToolItem(new ToolButton(listener, "GOTO_NEXT", ACTION_GOTO_NEXT, GTK_STOCK_GO_FORWARD, _("Next"), gui->get(
			"menuNavigationNextPage")));
	addToolItem(new ToolButton(listener, "GOTO_LAST", ACTION_GOTO_LAST, GTK_STOCK_GOTO_LAST, _("Go to last page"),
			gui->get("menuNavigationLastPage")));

	addToolItem(new ToolButton(listener, gui, "GOTO_NEXT_ANNOTATED_PAGE", ACTION_GOTO_NEXT_ANNOTATED_PAGE,
			"nextAnnotatedPage.png", _("Next annotated Page"), gui->get("menuNavigationNextAnnotatedPage")));

	addToolItem(new ToolButton(listener, "ZOOM_OUT", ACTION_ZOOM_OUT, GTK_STOCK_ZOOM_OUT, _("Zoom out"), gui->get(
			"menuViewZoomOut")));
	addToolItem(new ToolButton(listener, "ZOOM_IN", ACTION_ZOOM_IN, GTK_STOCK_ZOOM_IN, _("Zoom in"), gui->get(
			"menuViewZoomIn")));
	addToolItem(new ToolButton(listener, "ZOOM_FIT", ACTION_ZOOM_FIT, GTK_STOCK_ZOOM_FIT, _("Zoom fit to screen"),
			gui->get("menuViewZoomFit")));
	addToolItem(new ToolButton(listener, "ZOOM_100", ACTION_ZOOM_100, GTK_STOCK_ZOOM_100, _("Zoom to 100%"), gui->get(
			"menuViewZoom100")));

	addToolItem(new ToolButton(listener, gui, "FULLSCREEN", ACTION_FULLSCREEN, GROUP_FULLSCREEN, "fullscreen.png",
			_("Toggle fullscreen"), gui->get("menuViewFullScreen")));

	addToolItem(new ColorToolItem("COLOR_SELECT", listener, toolHandler, 0xff0000, true));

	addToolItem(new ToolButton(listener, gui, "PEN", ACTION_TOOL_PEN, GROUP_TOOL, "tool_pencil.png", _("Pen"),
			gui->get("menuToolsPen")));

	initEraserToolItem();

	addToolItem(new ToolSelectCombocontrol(this, listener, gui, "SELECT"));

	ToolButton * tbInsertNewPage = new ToolButton(listener, gui, "INSERT_NEW_PAGE", ACTION_NEW_PAGE_AFTER,
			"addPage.svg", _("Insert page"));
	addToolItem(tbInsertNewPage);
	GtkWidget * newPagePopup = gtk_menu_new();

	GtkWidget *newPagePopupPlain = gtk_check_menu_item_new_with_label(_("Plain"));
	gtk_widget_show(newPagePopupPlain);
	gtk_container_add(GTK_CONTAINER (newPagePopup), newPagePopupPlain);
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(newPagePopupPlain), true);
	registerMenupoint(newPagePopupPlain, ACTION_NEW_PAGE_PLAIN, GROUP_PAGE_INSERT_TYPE);
	registerMenupoint(gui->get("menuJournalNewPageAfterPlain"), ACTION_NEW_PAGE_PLAIN, GROUP_PAGE_INSERT_TYPE);

	GtkWidget *newPagePopupLined = gtk_check_menu_item_new_with_label(_("Lined"));
	gtk_widget_show(newPagePopupLined);
	gtk_container_add(GTK_CONTAINER (newPagePopup), newPagePopupLined);
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(newPagePopupLined), true);
	registerMenupoint(newPagePopupLined, ACTION_NEW_PAGE_LINED, GROUP_PAGE_INSERT_TYPE);
	registerMenupoint(gui->get("menuJournalNewPageAfterLined"), ACTION_NEW_PAGE_LINED, GROUP_PAGE_INSERT_TYPE);

	GtkWidget *newPagePopupRuled = gtk_check_menu_item_new_with_label(_("Ruled"));
	gtk_widget_show(newPagePopupRuled);
	gtk_container_add(GTK_CONTAINER (newPagePopup), newPagePopupRuled);
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(newPagePopupRuled), true);
	registerMenupoint(newPagePopupRuled, ACTION_NEW_PAGE_RULED, GROUP_PAGE_INSERT_TYPE);
	registerMenupoint(gui->get("menuJournalNewPageAfterRuled"), ACTION_NEW_PAGE_RULED, GROUP_PAGE_INSERT_TYPE);

	GtkWidget *newPagePopupGraph = gtk_check_menu_item_new_with_label(_("Graph"));
	gtk_widget_show(newPagePopupGraph);
	gtk_container_add(GTK_CONTAINER (newPagePopup), newPagePopupGraph);
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(newPagePopupGraph), true);
	registerMenupoint(newPagePopupGraph, ACTION_NEW_PAGE_GRAPH, GROUP_PAGE_INSERT_TYPE);
	registerMenupoint(gui->get("menuJournalNewPageAfterGraph"), ACTION_NEW_PAGE_GRAPH, GROUP_PAGE_INSERT_TYPE);

	GtkWidget *newPagePopupCopyCurrent = gtk_check_menu_item_new_with_label(_("Copy current"));
	gtk_widget_show(newPagePopupCopyCurrent);
	gtk_container_add(GTK_CONTAINER (newPagePopup), newPagePopupCopyCurrent);
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(newPagePopupCopyCurrent), true);
	registerMenupoint(newPagePopupCopyCurrent, ACTION_NEW_PAGE_COPY, GROUP_PAGE_INSERT_TYPE);
	registerMenupoint(gui->get("menuJournalNewPageAfterCopy"), ACTION_NEW_PAGE_COPY, GROUP_PAGE_INSERT_TYPE);

	GtkWidget *newPagePopupWithPDFBackground = gtk_check_menu_item_new_with_label(_("With PDF Background"));
	gtk_widget_show(newPagePopupWithPDFBackground);
	gtk_container_add(GTK_CONTAINER (newPagePopup), newPagePopupWithPDFBackground);
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(newPagePopupWithPDFBackground), true);
	registerMenupoint(newPagePopupWithPDFBackground, ACTION_NEW_PAGE_PDF_BACKGROUND, GROUP_PAGE_INSERT_TYPE);
	registerMenupoint(gui->get("menuJournalNewPageAfterWithPdf"), ACTION_NEW_PAGE_PDF_BACKGROUND,
			GROUP_PAGE_INSERT_TYPE);

	tbInsertNewPage->setPopupMenu(newPagePopup);

	addToolItem(new ToolButton(listener, gui, "HILIGHTER", ACTION_TOOL_HILIGHTER, GROUP_TOOL, "tool_highlighter.png",
			_("Hilighter"), gui->get("menuToolsHighlighter")));
	addToolItem(new ToolButton(listener, gui, "TEXT", ACTION_TOOL_TEXT, GROUP_TOOL, "tool_text.png", _("Text"),
			gui->get("menuToolsText")));
	addToolItem(new ToolButton(listener, gui, "IMAGE", ACTION_TOOL_IMAGE, GROUP_TOOL, "tool_image.png", _("Image"),
			gui->get("menuToolsImage")));

	addToolItem(new ToolButton(listener, gui, "SELECT_REGION", ACTION_TOOL_SELECT_REGION, GROUP_TOOL, "lasso.png",
			_("Select Region"), gui->get("menuToolsSelectRegion")));
	addToolItem(new ToolButton(listener, gui, "SELECT_RECTANGLE", ACTION_TOOL_SELECT_RECT, GROUP_TOOL,
			"rect-select.png", _("Select Rectangle"), gui->get("menuToolsSelectRectangle")));
	addToolItem(new ToolButton(listener, gui, "VERTICAL_SPACE", ACTION_TOOL_VERTICAL_SPACE, GROUP_TOOL, "stretch.png",
			_("Vertical Space"), gui->get("menuToolsVerticalSpace")));
	addToolItem(new ToolButton(listener, gui, "HAND", ACTION_TOOL_HAND, GROUP_TOOL, "hand.png", _("Hand"), gui->get(
			"menuToolsHand")));

	addToolItem(new ToolButton(listener, gui, "SHAPE_RECOGNIZER", ACTION_SHAPE_RECOGNIZER, GROUP_SHAPE_RECOGNIZER,
			"shape_recognizer.png", _("Shape Recognizer"), gui->get("menuToolsShapeRecognizer")));
	addToolItem(new ToolButton(listener, gui, "RULER", ACTION_RULER, GROUP_RULER, "ruler.png", _("Ruler"), gui->get(
			"menuToolsRuler")));

	addToolItem(new ToolButton(listener, gui, "FINE", ACTION_SIZE_FINE, GROUP_SIZE, "thickness_thin.png", _("Thin")));
	addToolItem(new ToolButton(listener, gui, "MEDIUM", ACTION_SIZE_MEDIUM, GROUP_SIZE, "thickness_medium.png",
			_("Medium")));
	addToolItem(new ToolButton(listener, gui, "THICK", ACTION_SIZE_THICK, GROUP_SIZE, "thickness_thick.png", _("Thik")));

	addToolItem(new FontButton(listener, gui, "SELECT_FONT", ACTION_SELECT_FONT, _("Select Font")));

	// Footer tools
	toolPageSpinner = new ToolPageSpinner(listener, "PAGE_SPIN", ACTION_FOOTER_PAGESPIN);
	addToolItem(toolPageSpinner);

	ToolZoomSlider * toolZoomSlider = new ToolZoomSlider(listener, "ZOOM_SLIDER", ACTION_FOOTER_ZOOM_SLIDER, zoom);
	addToolItem(toolZoomSlider);

	addToolItem(new ToolButton(listener, gui, "TWO_PAGES", ACTION_VIEW_TWO_PAGES, GROUP_TWOPAGES, "showtwopages.png",
			_("Two pages"), gui->get("menuViewTwoPages")));

	toolPageLayer = new ToolPageLayer(listener, "LAYER", ACTION_FOOTER_LAYER);
	addToolItem(toolPageLayer);

	registerMenupoint(gui->get("menuEditSettings"), ACTION_SETTINGS);
	registerMenupoint(gui->get("menuFileAnnotate"), ACTION_ANNOTATE_PDF);

	registerMenupoint(gui->get("menuFileSaveAs"), ACTION_SAVE_AS);
	registerMenupoint(gui->get("menuFileExportPdf"), ACTION_EXPORT_AS_PDF);
	registerMenupoint(gui->get("menuFileExportAs"), ACTION_EXPORT_AS);

	registerMenupoint(gui->get("menuDocumentProperties"), ACTION_DOCUMENT_PROPERTIES);
	registerMenupoint(gui->get("menuFilePrint"), ACTION_PRINT);

	registerMenupoint(gui->get("menuFileQuit"), ACTION_QUIT);

	registerMenupoint(gui->get("menuJournalNewLayer"), ACTION_NEW_LAYER);
	registerMenupoint(gui->get("menuJournalDeleteLayer"), ACTION_DELETE_LAYER);
	registerMenupoint(gui->get("menuJournalNewPageBefore"), ACTION_NEW_PAGE_BEFORE);
	registerMenupoint(gui->get("menuJournalNewPageAfter"), ACTION_NEW_PAGE_AFTER);
	registerMenupoint(gui->get("menuJournalNewPageAtEnd"), ACTION_NEW_PAGE_AT_END);
	registerMenupoint(gui->get("menuDeletePage"), ACTION_DELETE_PAGE);
	registerMenupoint(gui->get("menuJournalPaperFormat"), ACTION_PAPER_FORMAT);
	registerMenupoint(gui->get("menuJournalPaperColor"), ACTION_PAPER_BACKGROUND_COLOR);

	registerMenupoint(gui->get("menuJournalPaperPlain"), ACTION_SET_PAPER_BACKGROUND_PLAIN);
	registerMenupoint(gui->get("menuJournalPaperBackgroundLined"), ACTION_SET_PAPER_BACKGROUND_LINED);
	registerMenupoint(gui->get("menuJournalPaperRuled"), ACTION_SET_PAPER_BACKGROUND_RULED);
	registerMenupoint(gui->get("menuJournalPaperGraph"), ACTION_SET_PAPER_BACKGROUND_GRAPH);
	registerMenupoint(gui->get("menuJournalPaperImage"), ACTION_SET_PAPER_BACKGROUND_IMAGE);
	registerMenupoint(gui->get("menuJournalPaperPdf"), ACTION_SET_PAPER_BACKGROUND_PDF);

	registerMenupoint(gui->get("menuEditDelete"), ACTION_DELETE);

	registerMenupoint(gui->get("menuNavigationPreviousAnnotatedPage"), ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE);

	registerMenupoint(gui->get("eraserFine"), ACTION_TOOL_ERASER_SIZE_FINE, GROUP_ERASER_SIZE);
	registerMenupoint(gui->get("eraserMedium"), ACTION_TOOL_ERASER_SIZE_MEDIUM, GROUP_ERASER_SIZE);
	registerMenupoint(gui->get("eraserThick"), ACTION_TOOL_ERASER_SIZE_THICK, GROUP_ERASER_SIZE);

	registerMenupoint(gui->get("penthicknessVeryFine"), ACTION_TOOL_PEN_SIZE_VERY_THIN, GROUP_PEN_SIZE);
	registerMenupoint(gui->get("penthicknessFine"), ACTION_TOOL_PEN_SIZE_FINE, GROUP_PEN_SIZE);
	registerMenupoint(gui->get("penthicknessMedium"), ACTION_TOOL_PEN_SIZE_MEDIUM, GROUP_PEN_SIZE);
	registerMenupoint(gui->get("penthicknessThick"), ACTION_TOOL_PEN_SIZE_THICK, GROUP_PEN_SIZE);
	registerMenupoint(gui->get("penthicknessVeryThick"), ACTION_TOOL_PEN_SIZE_VERY_THICK, GROUP_PEN_SIZE);

	registerMenupoint(gui->get("highlighterFine"), ACTION_TOOL_HILIGHTER_SIZE_FINE, GROUP_HILIGHTER_SIZE);
	registerMenupoint(gui->get("highlighterMedium"), ACTION_TOOL_HILIGHTER_SIZE_MEDIUM, GROUP_HILIGHTER_SIZE);
	registerMenupoint(gui->get("highlighterThick"), ACTION_TOOL_HILIGHTER_SIZE_THICK, GROUP_HILIGHTER_SIZE);

	registerMenupoint(gui->get("menuToolsTextFont"), ACTION_SELECT_FONT);

	// Menu Help
	registerMenupoint(gui->get("menuHelpAbout"), ACTION_ABOUT);
	registerMenupoint(gui->get("menuHelpHelp"), ACTION_HELP);

}

ToolSelectCombocontrol::ToolSelectCombocontrol(ToolMenuHandler * th, ActionHandler * handler, GladeGui *gui, String id) :
	ToolButton(handler, gui, id, ACTION_TOOL_SELECT_RECT, GROUP_TOOL, "rect-select.png", _("Select Rectangle")) {

	this->labelWidget = NULL;

	GtkWidget * popup = gtk_menu_new();

	GtkWidget * menuItem;

	this->iconSelectRect = gui->loadIconPixbuf("rect-select.png");
	this->iconSelectRgion = gui->loadIconPixbuf("lasso.png");
	this->iconSelectObject = gui->loadIconPixbuf("object-select.png");
	g_object_ref(this->iconSelectRect);
	g_object_ref(this->iconSelectRgion);
	g_object_ref(this->iconSelectObject);

	menuItem = gtk_image_menu_item_new_with_label(_("Select Rectangle"));
	gtk_container_add(GTK_CONTAINER(popup), menuItem);
	th->registerMenupoint(menuItem, ACTION_TOOL_SELECT_RECT, GROUP_TOOL);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), gui->loadIcon("rect-select.png"));
	gtk_widget_show_all(menuItem);

	menuItem = gtk_image_menu_item_new_with_label(_("Select Region"));
	gtk_container_add(GTK_CONTAINER(popup), menuItem);
	th->registerMenupoint(menuItem, ACTION_TOOL_SELECT_REGION, GROUP_TOOL);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), gui->loadIcon("lasso.png"));
	gtk_widget_show_all(menuItem);

	menuItem = gtk_image_menu_item_new_with_label(_("Select Object"));
	gtk_container_add(GTK_CONTAINER(popup), menuItem);
	th->registerMenupoint(menuItem, ACTION_TOOL_SELECT_OBJECT, GROUP_TOOL);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), gui->loadIcon("object-select.png"));
	gtk_widget_show_all(menuItem);

	setPopupMenu(popup);
}

void ToolMenuHandler::setUndoDescription(String description) {
	undoButton->updateDescription(description);
	gtk_menu_item_set_label(GTK_MENU_ITEM(gui->get("menuEditUndo")), description.c_str());
}

void ToolMenuHandler::setRedoDescription(String description) {
	redoButton->updateDescription(description);
	gtk_menu_item_set_label(GTK_MENU_ITEM(gui->get("menuEditRedo")), description.c_str());
}

GtkWidget * ToolMenuHandler::getPageSpinner() {
	return toolPageSpinner->getPageSpinner();
}

void ToolMenuHandler::setPageText(String text) {
	toolPageSpinner->setText(text);
}

int ToolMenuHandler::getSelectedLayer() {
	return toolPageLayer->getSelectedLayer();
}

void ToolMenuHandler::setLayerCount(int count, int selected) {
	toolPageLayer->setLayerCount(count, selected);
}

