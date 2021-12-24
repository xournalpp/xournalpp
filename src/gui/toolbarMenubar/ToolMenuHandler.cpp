#include "ToolMenuHandler.h"

#include <utility>

#include <config-features.h>
#include <config.h>

#include "control/Actions.h"
#include "control/Control.h"
#include "control/PageBackgroundChangeController.h"
#include "gui/ToolitemDragDrop.h"
#include "model/ToolbarData.h"
#include "model/ToolbarModel.h"

#include "FontButton.h"
#include "MenuItem.h"
#include "StringUtils.h"
#include "ToolButton.h"
#include "ToolDrawCombocontrol.h"
#include "ToolPageLayer.h"
#include "ToolPageSpinner.h"
#include "ToolSelectCombocontrol.h"
#include "ToolZoomSlider.h"
#include "i18n.h"

ToolMenuHandler::ToolMenuHandler(Control* control, GladeGui* gui, GtkWindow* parent) {
    this->parent = parent;
    this->control = control;
    this->listener = control;
    this->zoom = control->getZoomControl();
    this->gui = gui;
    this->toolHandler = control->getToolHandler();
    this->tbModel = new ToolbarModel();

    // still owned by Control
    this->newPageType = control->getNewPageType();
    this->newPageType->addApplyBackgroundButton(control->getPageBackgroundChangeController(), false);

    // still owned by Control
    this->pageBackgroundChangeController = control->getPageBackgroundChangeController();

    initToolItems();
}

ToolMenuHandler::~ToolMenuHandler() {
    delete this->tbModel;
    this->tbModel = nullptr;

    // Owned by control
    this->pageBackgroundChangeController = nullptr;

    // Owned by control
    this->newPageType = nullptr;

    for (MenuItem* it: this->menuItems) {
        delete it;
        it = nullptr;
    }

    deleteAllToolbarItems();
}

void ToolMenuHandler::deleteAllToolbarItems() {
    this->freeDynamicToolbarItems();

    for (AbstractToolItem* it: this->toolItems) {
        delete it;
    }
    this->toolItems.clear();
    // the preaviusly foreach delete all the obj but not the private pointer.
    this->undoButton.clear();
    this->redoButton.clear();
    this->audioPausePlaybackButton.clear();
    this->audioStopPlaybackButton.clear();
    this->audioSeekBackwardsButton.clear();
    this->audioSeekForwardsButton.clear();
    this->toolPageSpinner.clear();
    this->toolPageLayer.clear();
    this->fontButton.clear();
}

void ToolMenuHandler::freeDynamicToolbarItems() {
    for (AbstractToolItem* it: this->toolItems) {
        it->setUsed(false);
    }

    for (ColorToolItem* it: this->toolbarColorItems) {
        delete it;
    }
    this->toolbarColorItems.clear();
}

void ToolMenuHandler::unloadToolbar(GtkWidget* toolbar) {
    for (int i = gtk_toolbar_get_n_items(GTK_TOOLBAR(toolbar)) - 1; i >= 0; i--) {
        GtkToolItem* tbItem = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), i);
        gtk_container_remove(GTK_CONTAINER(toolbar), GTK_WIDGET(tbItem));
    }

    gtk_widget_hide(toolbar);
}

// Load expext to start with items toolbar just empty
void ToolMenuHandler::load(ToolbarData* d, GtkWidget* toolbar, const char* toolbarName, bool horizontal) {
    int count = 0;

    for (ToolbarEntry* e: d->contents) {
        if (e->getName() == toolbarName) {
            for (ToolbarItem* dataItem: e->getItems()) {
                string name = dataItem->getName();

                // recognize previous name, V1.07 (Jan 2019) and earlier.
                if (name == "TWO_PAGES") {
                    name = "PAIRED_PAGES";
                }

                // recognize previous name, V1.08 (Feb 2019) and earlier.
                if (name == "RECSTOP") {
                    name = "AUDIO_RECORDING";
                }

                // recognize previous name, V1.0.19 (Dec 2020) and earlier
                if (name == "HILIGHTER") {
                    name = "HIGHLIGHTER";
                }

                // recognize previous name, V1.1.0+dev (Jan 2021) and earlier
                if (name == "DRAW_CIRCLE") {
                    name = "DRAW_ELLIPSE";
                }

                if (name == "SEPARATOR") {
                    GtkToolItem* it = gtk_separator_tool_item_new();
                    gtk_widget_show(GTK_WIDGET(it));
                    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), it, -1);

                    ToolitemDragDrop::attachMetadata(GTK_WIDGET(it), dataItem->getId(), TOOL_ITEM_SEPARATOR);

                    continue;
                }

                if (name == "SPACER") {
                    GtkToolItem* toolItem = gtk_separator_tool_item_new();
                    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolItem), false);
                    gtk_tool_item_set_expand(toolItem, true);
                    gtk_widget_show(GTK_WIDGET(toolItem));
                    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolItem, -1);

                    ToolitemDragDrop::attachMetadata(GTK_WIDGET(toolItem), dataItem->getId(), TOOL_ITEM_SPACER);

                    continue;
                }
                if (StringUtils::startsWith(name, "COLOR(") && name.length() == 15) {
                    string color = name.substr(6, 8);
                    if (!StringUtils::startsWith(color, "0x")) {
                        g_warning("Toolbar:COLOR(...) has to start with 0x, get color: %s", color.c_str());
                        continue;
                    }
                    count++;

                    color = color.substr(2);
                    auto c = Color(g_ascii_strtoull(color.c_str(), nullptr, 16));

                    auto* item = new ColorToolItem(listener, toolHandler, this->parent, c);
                    this->toolbarColorItems.push_back(item);

                    GtkToolItem* it = item->createItem(horizontal);
                    gtk_widget_show_all(GTK_WIDGET(it));
                    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), it, -1);

                    ToolitemDragDrop::attachMetadataColor(GTK_WIDGET(it), dataItem->getId(), c, item);

                    continue;
                }

                AbstractToolItem* item = addToolItemsfromName(name);

                if (item->isUsed()) {
                    g_warning("You can use the toolbar item \"%s\" only once!", item->getId().c_str());
                    continue;
                }
                item->setUsed(true);

                count++;
                GtkToolItem* it = item->createItem(horizontal);

                gtk_widget_show_all(GTK_WIDGET(it));
                gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(it), -1);

                ToolitemDragDrop::attachMetadata(GTK_WIDGET(it), dataItem->getId(), item);
            }
        }
    }

    // this tool are indispensable
    // if not present before, are add here without duplication
    initToolItems();

    this->connectSignalsOfToolItems();

    if (count == 0) {
        gtk_widget_hide(toolbar);
    } else {
        gtk_widget_show(toolbar);
    }
}

void ToolMenuHandler::removeColorToolItem(AbstractToolItem* it) {
    g_return_if_fail(it != nullptr);
    for (unsigned int i = 0; i < this->toolbarColorItems.size(); i++) {
        if (this->toolbarColorItems[i] == it) {
            this->toolbarColorItems.erase(this->toolbarColorItems.begin() + i);
            break;
        }
    }
    delete dynamic_cast<ColorToolItem*>(it);
}

void ToolMenuHandler::addColorToolItem(AbstractToolItem* it) {
    g_return_if_fail(it != nullptr);
    this->toolbarColorItems.push_back(dynamic_cast<ColorToolItem*>(it));
}

void ToolMenuHandler::setTmpDisabled(bool disabled) {
    for (AbstractToolItem* it: this->toolItems) {
        it->setTmpDisabled(disabled);
    }

    for (MenuItem* it: this->menuItems) {
        it->setTmpDisabled(disabled);
    }

    for (ColorToolItem* it: this->toolbarColorItems) {
        it->setTmpDisabled(disabled);
    }

    GtkWidget* menuViewSidebarVisible = gui->get("menuViewSidebarVisible");
    gtk_widget_set_sensitive(menuViewSidebarVisible, !disabled);
}

AbstractToolItem* ToolMenuHandler::addToolItem(AbstractToolItem* it) {
    this->toolItems.push_back(it);
    return it;
}

void ToolMenuHandler::registerMenupoint(GtkWidget* widget, ActionType type, ActionGroup group) {
    this->menuItems.push_back(new MenuItem(listener, widget, type, group));
}

ToolButton* ToolMenuHandler::initPenToolItem() {
    auto* tbPen = new ToolButton(listener, "PEN", ACTION_TOOL_PEN, GROUP_TOOL, true, "tool_pencil", _("Pen"));

    registerMenupoint(tbPen->registerPopupMenuEntry(_("standard"), "line-style-plain"), ACTION_TOOL_LINE_STYLE_PLAIN,
                      GROUP_LINE_STYLE);

    registerMenupoint(tbPen->registerPopupMenuEntry(_("dashed"), "line-style-dash"), ACTION_TOOL_LINE_STYLE_DASH,
                      GROUP_LINE_STYLE);

    registerMenupoint(tbPen->registerPopupMenuEntry(_("dash-/ dotted"), "line-style-dash-dot"),
                      ACTION_TOOL_LINE_STYLE_DASH_DOT, GROUP_LINE_STYLE);

    registerMenupoint(tbPen->registerPopupMenuEntry(_("dotted"), "line-style-dot"), ACTION_TOOL_LINE_STYLE_DOT,
                      GROUP_LINE_STYLE);

    addToolItem(tbPen);
    return tbPen;
}

ToolButton* ToolMenuHandler::initEraserToolItem() {
    auto* tbEraser =
            new ToolButton(listener, "ERASER", ACTION_TOOL_ERASER, GROUP_TOOL, true, "tool_eraser", _("Eraser"));

    registerMenupoint(tbEraser->registerPopupMenuEntry(_("standard")), ACTION_TOOL_ERASER_STANDARD, GROUP_ERASER_MODE);
    registerMenupoint(tbEraser->registerPopupMenuEntry(_("whiteout")), ACTION_TOOL_ERASER_WHITEOUT, GROUP_ERASER_MODE);
    registerMenupoint(tbEraser->registerPopupMenuEntry(_("delete stroke")), ACTION_TOOL_ERASER_DELETE_STROKE,
                      GROUP_ERASER_MODE);

    addToolItem(tbEraser);
    return tbEraser;
}

void ToolMenuHandler::signalConnectCallback(GtkBuilder* builder, GObject* object, const gchar* signalName,
                                            const gchar* handlerName, GObject* connectObject, GConnectFlags flags,
                                            ToolMenuHandler* self) {
    string actionName = handlerName;
    string groupName{};

    size_t pos = actionName.find(':');
    if (pos != string::npos) {
        groupName = actionName.substr(pos + 1);
        actionName = actionName.substr(0, pos);
    }

    ActionGroup group = GROUP_NOGROUP;
    ActionType action = ActionType_fromString(actionName);

    if (action == ACTION_NONE) {
        g_error("Unknown action name from glade file: \"%s\" / \"%s\"", signalName, handlerName);
        return;
    }

    if (!groupName.empty()) {
        group = ActionGroup_fromString(groupName);
    }

    if (GTK_IS_MENU_ITEM(object)) {
        for (AbstractToolItem* it: self->toolItems) {
            if (it->getActionType() == action) {
                // There is already a toolbar item -> attach menu to it
                it->setMenuItem(GTK_WIDGET(object));
                return;
            }
        }

        // There is no toolbar item -> register the menu only
        self->registerMenupoint(GTK_WIDGET(object), action, group);
    } else {
        g_error("Unsupported signal handler from glade file: \"%s\" / \"%s\"", signalName, handlerName);
    }
}

// Use GTK Stock icon
#define ADD_STOCK_ITEM(name, action, stockIcon, text) \
    addToolItem(new ToolButton(listener, name, action, stockIcon, text))

#define ADD_STOCK_ITEM_IF_NAME_EQ(nameVar, name, action, stockIcon, text)                   \
    do {                                                                                    \
        if (nameVar == name) {                                                              \
            AbstractToolItem* it = new ToolButton(listener, name, action, stockIcon, text); \
            addToolItem(it);                                                                \
            return it;                                                                      \
        }                                                                                   \
    } while (0)

// Use Custom loading Icon
#define ADD_CUSTOM_ITEM(name, action, icon, text) addToolItem(new ToolButton(listener, name, action, icon, text))

#define ADD_CUSTOM_ITEM_IF_NAME_EQ(nameVar, name, action, icon, text)                  \
    do {                                                                               \
        if (nameVar == name) {                                                         \
            AbstractToolItem* it = new ToolButton(listener, name, action, icon, text); \
            addToolItem(it);                                                           \
            return it;                                                                 \
        }                                                                              \
    } while (0)

// Use Custom loading Icon, toggle item
// switchOnly: You can select pen, eraser etc. but you cannot unselect pen.
#define ADD_CUSTOM_ITEM_TGL(name, action, group, switchOnly, icon, text) \
    addToolItem(new ToolButton(listener, name, action, group, switchOnly, icon, text))

#define ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(nameVar, name, action, group, switchOnly, icon, text)              \
    do {                                                                                                  \
        if (nameVar == name) {                                                                            \
            ADD_CUSTOM_ITEM_TGL(name, action, group, switchOnly, icon, text);                             \
            AbstractToolItem* it = new ToolButton(listener, name, action, group, switchOnly, icon, text); \
            addToolItem(it);                                                                              \
            return it;                                                                                    \
        }                                                                                                 \
    } while (0)

#define ADD_VECTOR_ToolButton_IF_NAME_EQ(vectorVar, nameVar, name, action, stockIcon, text) \
    do {                                                                                    \
        if (nameVar == name) {                                                              \
            ToolButton* it = new ToolButton(listener, name, action, stockIcon, _(text));    \
            this->vectorVar.push_back(it);                                                  \
            addToolItem(it);                                                                \
            return it;                                                                      \
        }                                                                                   \
    } while (0)

#define ADD_VECTOR_ToolButton_EXTENDED_IF_NAME_EQ(vectorVar, nameVar, name, action, group, toolToggleOnlyEnable,      \
                                                  stockIcon, text)                                                    \
    do {                                                                                                              \
        if (nameVar == name) {                                                                                        \
            ToolButton* it = new ToolButton(listener, name, action, group, toolToggleOnlyEnable, stockIcon, _(text)); \
            this->vectorVar.push_back(it);                                                                            \
            addToolItem(it);                                                                                          \
            return it;                                                                                                \
        }                                                                                                             \
    } while (0)

#define ADD_VECTOR_FontButton_IF_NAME_EQ(vectorVar, nameVar, name, action, text)   \
    do {                                                                           \
        if (nameVar == name) {                                                     \
            FontButton* it = new FontButton(listener, gui, name, action, _(text)); \
            this->vectorVar.push_back(it);                                         \
            addToolItem(it);                                                       \
            return it;                                                             \
        }                                                                          \
    } while (0)


void ToolMenuHandler::initToolItems() {
    // this is the indispensable tools
    if (undoButton.empty())
        addToolItemsfromName("UNDO");
    if (redoButton.empty())
        addToolItemsfromName("REDO");

    if (audioPausePlaybackButton.empty())
        addToolItemsfromName("AUDIO_PAUSE_PLAYBACK");
    if (audioStopPlaybackButton.empty())
        addToolItemsfromName("AUDIO_STOP_PLAYBACK");
    if (audioSeekBackwardsButton.empty())
        addToolItemsfromName("AUDIO_SEEK_BACKWARDS");
    if (audioSeekForwardsButton.empty())
        addToolItemsfromName("AUDIO_SEEK_FORWARDS");

    if (toolPageSpinner.empty())
        addToolItemsfromName("PAGE_SPIN");
    if (toolPageLayer.empty())
        addToolItemsfromName("LAYER");

    if (fontButton.empty())
        addToolItemsfromName("SELECT_FONT");
}

AbstractToolItem* ToolMenuHandler::addToolItemsfromName(string name) {
    // Items ordered by menu, if possible.
    // There are some entries which are not available in the menu, like the Zoom slider
    // All menu items without tool icon are not listed here - they are connected by Glade Signals

    // Menu File
    // ************************************************************************

    ADD_STOCK_ITEM_IF_NAME_EQ(name, "NEW", ACTION_NEW, "document-new", _("New Xournal"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "OPEN", ACTION_OPEN, "document-open", _("Open file"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "SAVE", ACTION_SAVE, "document-save", _("Save"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "PRINT", ACTION_PRINT, "document-print", _("Print"));

    // Menu Edit
    // ************************************************************************

    // Undo / Redo Texts are updated from code, therefore a reference is hold for this items

    ADD_VECTOR_ToolButton_IF_NAME_EQ(undoButton, name, "UNDO", ACTION_UNDO, "edit-undo", "Undo");
    ADD_VECTOR_ToolButton_IF_NAME_EQ(redoButton, name, "REDO", ACTION_REDO, "edit-redo", "Redo");


    ADD_STOCK_ITEM_IF_NAME_EQ(name, "CUT", ACTION_CUT, "edit-cut", _("Cut"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "COPY", ACTION_COPY, "edit-copy", _("Copy"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "PASTE", ACTION_PASTE, "edit-paste", _("Paste"));

    ADD_STOCK_ITEM_IF_NAME_EQ(name, "SEARCH", ACTION_SEARCH, "edit-find", _("Search"));

    ADD_STOCK_ITEM_IF_NAME_EQ(name, "DELETE", ACTION_DELETE, "edit-delete", _("Delete"));

    // Icon snapping.svg made by www.freepik.com from www.flaticon.com
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "ROTATION_SNAPPING", ACTION_ROTATION_SNAPPING, GROUP_SNAPPING, false,
                                   "snapping", _("Rotation Snapping"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "GRID_SNAPPING", ACTION_GRID_SNAPPING, GROUP_GRID_SNAPPING, false,
                                   "grid_snapping", _("Grid Snapping"));

    // Menu View
    // ************************************************************************

    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "PAIRED_PAGES", ACTION_VIEW_PAIRED_PAGES, GROUP_PAIRED_PAGES, false,
                                   "showpairedpages", _("Paired pages"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "PRESENTATION_MODE", ACTION_VIEW_PRESENTATION_MODE, GROUP_PRESENTATION_MODE,
                                   false, "presentation-mode", _("Presentation mode"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "FULLSCREEN", ACTION_FULLSCREEN, GROUP_FULLSCREEN, false, "fullscreen",
                                   _("Toggle fullscreen"));

    ADD_STOCK_ITEM_IF_NAME_EQ(name, "MANAGE_TOOLBAR", ACTION_MANAGE_TOOLBAR, "manage_toolbars", _("Manage Toolbars"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "CUSTOMIZE_TOOLBAR", ACTION_CUSTOMIZE_TOOLBAR, "customize_toolbars",
                              _("Customize Toolbars"));

    ADD_STOCK_ITEM_IF_NAME_EQ(name, "ZOOM_OUT", ACTION_ZOOM_OUT, "zoom-out", _("Zoom out"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "ZOOM_IN", ACTION_ZOOM_IN, "zoom-in", _("Zoom in"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "ZOOM_FIT", ACTION_ZOOM_FIT, GROUP_ZOOM_FIT, false, "zoom-fit-best",
                                   _("Zoom fit to screen"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "ZOOM_100", ACTION_ZOOM_100, "zoom-original", _("Zoom to 100%"));

    // Menu Navigation
    // ************************************************************************

    ADD_STOCK_ITEM_IF_NAME_EQ(name, "GOTO_FIRST", ACTION_GOTO_FIRST, "go-first", _("Go to first page"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "GOTO_BACK", ACTION_GOTO_BACK, "go-previous", _("Back"));
    ADD_CUSTOM_ITEM_IF_NAME_EQ(name, "GOTO_PAGE", ACTION_GOTO_PAGE, "goto", _("Go to page"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "GOTO_NEXT", ACTION_GOTO_NEXT, "go-next", _("Next"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "GOTO_LAST", ACTION_GOTO_LAST, "go-last", _("Go to last page"));

    ADD_STOCK_ITEM_IF_NAME_EQ(name, "GOTO_PREVIOUS_LAYER", ACTION_GOTO_PREVIOUS_LAYER, "go-previous",
                              _("Go to previous layer"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "GOTO_NEXT_LAYER", ACTION_GOTO_NEXT_LAYER, "go-next", _("Go to next layer"));
    ADD_STOCK_ITEM_IF_NAME_EQ(name, "GOTO_TOP_LAYER", ACTION_GOTO_TOP_LAYER, "go-top", _("Go to top layer"));

    ADD_CUSTOM_ITEM_IF_NAME_EQ(name, "GOTO_NEXT_ANNOTATED_PAGE", ACTION_GOTO_NEXT_ANNOTATED_PAGE, "nextAnnotatedPage",
                               _("Next annotated page"));

    // Menu Journal
    // ************************************************************************
    if (name == "INSERT_NEW_PAGE") {
        auto* tbInsertNewPage =
                new ToolButton(listener, "INSERT_NEW_PAGE", ACTION_NEW_PAGE_AFTER, "addPage", _("Insert page"));
        addToolItem(tbInsertNewPage);
        tbInsertNewPage->setPopupMenu(this->newPageType->getMenu());

        gtk_menu_item_set_submenu(GTK_MENU_ITEM(gui->get("menuJournalPaperBackground")),
                                  pageBackgroundChangeController->getMenu());

        return tbInsertNewPage;
    }

    ADD_CUSTOM_ITEM_IF_NAME_EQ(name, "DELETE_CURRENT_PAGE", ACTION_DELETE_PAGE, "delPage", _("Delete current page"));


    ADD_STOCK_ITEM_IF_NAME_EQ(name, "CHANGE_LAYER_NAME", ACTION_RENAME_LAYER, "changeLayer",
                              _("Change current layer name"));
    // Menu Tool
    // ************************************************************************

    if (name == "PEN") {
        return initPenToolItem();
    }

    if (name == "ERASER") {
        return initEraserToolItem();
    }

    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "HIGHLIGHTER", ACTION_TOOL_HIGHLIGHTER, GROUP_TOOL, true, "tool_highlighter",
                                   _("Highlighter"));

    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "TEXT", ACTION_TOOL_TEXT, GROUP_TOOL, true, "tool_text", _("Text"));
    ADD_CUSTOM_ITEM_IF_NAME_EQ(name, "MATH_TEX", ACTION_TEX, "tool_math_tex", _("Add/Edit Tex"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "IMAGE", ACTION_TOOL_IMAGE, GROUP_TOOL, true, "tool_image", _("Image"));
    ADD_CUSTOM_ITEM_IF_NAME_EQ(name, "DEFAULT_TOOL", ACTION_TOOL_DEFAULT, "default", _("Default Tool"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "SHAPE_RECOGNIZER", ACTION_SHAPE_RECOGNIZER, GROUP_RULER, false,
                                   "shape_recognizer", _("Shape Recognizer"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "DRAW_RECTANGLE", ACTION_TOOL_DRAW_RECT, GROUP_RULER, false, "rect-draw",
                                   _("Draw Rectangle"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "DRAW_ELLIPSE", ACTION_TOOL_DRAW_ELLIPSE, GROUP_RULER, false, "ellipse-draw",
                                   _("Draw Ellipse"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "DRAW_ARROW", ACTION_TOOL_DRAW_ARROW, GROUP_RULER, false, "arrow-draw",
                                   _("Draw Arrow"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "DRAW_COORDINATE_SYSTEM", ACTION_TOOL_DRAW_COORDINATE_SYSTEM, GROUP_RULER,
                                   false, "coordinate-system-draw", _("Draw coordinate system"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "RULER", ACTION_RULER, GROUP_RULER, false, "ruler", _("Ruler"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "DRAW_SPLINE", ACTION_TOOL_DRAW_SPLINE, GROUP_RULER, false, "spline-draw",
                                   _("Draw Spline"));

    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "SELECT_REGION", ACTION_TOOL_SELECT_REGION, GROUP_TOOL, true, "lasso",
                                   _("Select Region"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "SELECT_RECTANGLE", ACTION_TOOL_SELECT_RECT, GROUP_TOOL, true, "rect-select",
                                   _("Select Rectangle"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "SELECT_OBJECT", ACTION_TOOL_SELECT_OBJECT, GROUP_TOOL, true, "object-select",
                                   _("Select Object"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "VERTICAL_SPACE", ACTION_TOOL_VERTICAL_SPACE, GROUP_TOOL, true, "stretch",
                                   _("Vertical Space"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "PLAY_OBJECT", ACTION_TOOL_PLAY_OBJECT, GROUP_TOOL, true, "object-play",
                                   _("Play Object"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "HAND", ACTION_TOOL_HAND, GROUP_TOOL, true, "hand", _("Hand"));


    ADD_VECTOR_FontButton_IF_NAME_EQ(fontButton, name, "SELECT_FONT", ACTION_FONT_BUTTON_CHANGED, "Select Font");


    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "AUDIO_RECORDING", ACTION_AUDIO_RECORD, GROUP_AUDIO, false, "audio-record",
                                   _("Record Audio / Stop Recording"));

    ADD_VECTOR_ToolButton_EXTENDED_IF_NAME_EQ(audioPausePlaybackButton, name, "AUDIO_PAUSE_PLAYBACK",
                                              ACTION_AUDIO_PAUSE_PLAYBACK, GROUP_AUDIO, false, "audio-playback-pause",
                                              "Pause / Play");
    ADD_VECTOR_ToolButton_EXTENDED_IF_NAME_EQ(audioStopPlaybackButton, name, "AUDIO_STOP_PLAYBACK",
                                              ACTION_AUDIO_STOP_PLAYBACK, GROUP_AUDIO, false, "audio-playback-stop",
                                              "Stop");
    ADD_VECTOR_ToolButton_EXTENDED_IF_NAME_EQ(audioSeekBackwardsButton, name, "AUDIO_SEEK_BACKWARDS",
                                              ACTION_AUDIO_SEEK_BACKWARDS, GROUP_AUDIO, false, "audio-seek-backwards",
                                              "Back");
    ADD_VECTOR_ToolButton_EXTENDED_IF_NAME_EQ(audioSeekForwardsButton, name, "AUDIO_SEEK_FORWARDS",
                                              ACTION_AUDIO_SEEK_FORWARDS, GROUP_AUDIO, false, "audio-seek-forwards",
                                              "Forward");
    // Menu Help
    // ************************************************************************
    // All tools are registered by the Glade Signals


    ///////////////////////////////////////////////////////////////////////////


    // Footer tools
    // ************************************************************************

    if (name == "PAGE_SPIN") {
        ToolPageSpinner* it = new ToolPageSpinner(gui, listener, "PAGE_SPIN", ACTION_FOOTER_PAGESPIN);
        this->toolPageSpinner.push_back(it);
        addToolItem(it);
        return it;
    }


    if (name == "ZOOM_SLIDER") {
        auto* toolZoomSlider = new ToolZoomSlider(listener, "ZOOM_SLIDER", ACTION_FOOTER_ZOOM_SLIDER, zoom);
        addToolItem(toolZoomSlider);
        return toolZoomSlider;
    }

    if (name == "LAYER") {
        ToolPageLayer* it =
                new ToolPageLayer(control->getLayerController(), gui, listener, "LAYER", ACTION_FOOTER_LAYER);
        this->toolPageLayer.push_back(it);
        addToolItem(it);
        return it;
    }


    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "TOOL_FILL", ACTION_TOOL_FILL, GROUP_FILL, false, "fill", _("Fill"));


    // Non-menu items
    // ************************************************************************

    // Color item - not in the menu

    if (name == "COLOR_SELECT") {
        return addToolItem(new ColorToolItem(listener, toolHandler, this->parent, Color{0xff0000U}, true));
    }

    if (name == "SELECT") {
        return addToolItem(new ToolSelectCombocontrol(this, listener, "SELECT"));
    }
    if (name == "DRAW") {
        return addToolItem(new ToolDrawCombocontrol(this, listener, "DRAW"));
    }

    // General tool configuration - working for every tool which support it
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "VERY_FINE", ACTION_SIZE_VERY_FINE, GROUP_SIZE, true, "thickness_very_fine",
                                   _("Very Fine"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "FINE", ACTION_SIZE_FINE, GROUP_SIZE, true, "thickness_fine", _("Fine"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "MEDIUM", ACTION_SIZE_MEDIUM, GROUP_SIZE, true, "thickness_medium",
                                   _("Medium"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "THICK", ACTION_SIZE_THICK, GROUP_SIZE, true, "thickness_thick", _("Thick"));
    ADD_CUSTOM_ITEM_TGL_IF_NAME_EQ(name, "VERY_THICK", ACTION_SIZE_VERY_THICK, GROUP_SIZE, true, "thickness_very_thick",
                                   _("Very Thick"));

    // If arrive here, the variable name have to contain an error
    g_error("You try to add an item not exist \"%s\"", name.c_str());
    return nullptr;
}

void ToolMenuHandler::connectSignalsOfToolItems() {
    gtk_builder_connect_signals_full(gui->getBuilder(), reinterpret_cast<GtkBuilderConnectFunc>(signalConnectCallback),
                                     this);
}

void ToolMenuHandler::setFontButtonFont(XojFont& font) {
    for (FontButton* it: this->fontButton) {
        it->setFont(font);
    }
}

auto ToolMenuHandler::getFontButtonFont() -> XojFont {
    // every object shuld be set at the same value
    return this->fontButton.front()->getFont();
}

void ToolMenuHandler::showFontSelectionDlg() {
    // every object shuld be set at the same value
    this->fontButton.front()->showFontDialog();
}

void ToolMenuHandler::setUndoDescription(const string& description) {
    for (ToolButton* it: this->undoButton) {
        it->updateDescription(description);
    }
    gtk_menu_item_set_label(GTK_MENU_ITEM(gui->get("menuEditUndo")), description.c_str());
}

void ToolMenuHandler::setRedoDescription(const string& description) {
    for (ToolButton* it: this->redoButton) {
        it->updateDescription(description);
    }
    gtk_menu_item_set_label(GTK_MENU_ITEM(gui->get("menuEditRedo")), description.c_str());
}

auto ToolMenuHandler::getPageSpinner() -> SpinPageAdapter* {
    // every object shuld be set at the same value
    return this->toolPageSpinner.front()->getPageSpinner();
}

void ToolMenuHandler::setPageText(const string& text) {
    for (ToolPageSpinner* it: this->toolPageSpinner) {
        it->setText(text);
    }
}

auto ToolMenuHandler::getModel() -> ToolbarModel* { return this->tbModel; }

auto ToolMenuHandler::isColorInUse(Color color) -> bool {
    for (ColorToolItem* it: this->toolbarColorItems) {
        if (it->getColor() == color) {
            return true;
        }
    }

    return false;
}

auto ToolMenuHandler::getToolItems() -> vector<AbstractToolItem*>* { return &this->toolItems; }

void ToolMenuHandler::disableAudioPlaybackButtons() {
    setAudioPlaybackPaused(false);

    for (ToolButton* it: this->audioPausePlaybackButton) {
        it->enable(false);
    }
    for (ToolButton* it: this->audioStopPlaybackButton) {
        it->enable(false);
    }
    for (ToolButton* it: this->audioSeekBackwardsButton) {
        it->enable(false);
    }
    for (ToolButton* it: this->audioSeekForwardsButton) {
        it->enable(false);
    }


    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioPausePlayback")), false);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioStopPlayback")), false);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioSeekForwards")), false);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioSeekBackwards")), false);
}

void ToolMenuHandler::enableAudioPlaybackButtons() {

    for (ToolButton* it: this->audioPausePlaybackButton) {
        it->enable(true);
    }
    for (ToolButton* it: this->audioStopPlaybackButton) {
        it->enable(true);
    }
    for (ToolButton* it: this->audioSeekBackwardsButton) {
        it->enable(true);
    }
    for (ToolButton* it: this->audioSeekForwardsButton) {
        it->enable(true);
    }

    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioPausePlayback")), true);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioStopPlayback")), true);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioSeekForwards")), true);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioSeekBackwards")), true);
}

void ToolMenuHandler::setAudioPlaybackPaused(bool paused) {
    for (ToolButton* it: this->audioPausePlaybackButton) {
        it->setActive(paused);
    }
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gui->get("menuAudioPausePlayback")), paused);
}
