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

ToolMenuHandler::ToolMenuHandler(Control* control, GladeGui* gui, GtkWindow* parent):
        parent(parent),
        control(control),
        listener(control),
        zoom(control->getZoomControl()),
        gui(gui),
        toolHandler(control->getToolHandler()),
        iconNameHelper(control->getSettings()) {

    this->tbModel = new ToolbarModel();
    // still owned by Control
    this->newPageType = control->getNewPageType();
    this->newPageType->addApplyBackgroundButton(control->getPageBackgroundChangeController(), false,
                                                ApplyPageTypeSource::SELECTED);

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

    freeDynamicToolbarItems();

    for (AbstractToolItem* it: this->toolItems) {
        delete it;
        it = nullptr;
    }
}

void ToolMenuHandler::freeDynamicToolbarItems() {
    for (AbstractToolItem* it: this->toolItems) { it->setUsed(false); }

    for (ColorToolItem* it: this->toolbarColorItems) { delete it; }
    this->toolbarColorItems.clear();
}

void ToolMenuHandler::unloadToolbar(GtkWidget* toolbar) {
    for (int i = gtk_toolbar_get_n_items(GTK_TOOLBAR(toolbar)) - 1; i >= 0; i--) {
        GtkToolItem* tbItem = gtk_toolbar_get_nth_item(GTK_TOOLBAR(toolbar), i);
        gtk_container_remove(GTK_CONTAINER(toolbar), GTK_WIDGET(tbItem));
    }

    gtk_widget_hide(toolbar);
}

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

                bool found = false;
                for (AbstractToolItem* item: this->toolItems) {
                    if (name == item->getId()) {
                        if (item->isUsed()) {
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
    for (AbstractToolItem* it: this->toolItems) { it->setTmpDisabled(disabled); }

    for (MenuItem* it: this->menuItems) { it->setTmpDisabled(disabled); }

    for (ColorToolItem* it: this->toolbarColorItems) { it->setTmpDisabled(disabled); }

    GtkWidget* menuViewSidebarVisible = gui->get("menuViewSidebarVisible");
    gtk_widget_set_sensitive(menuViewSidebarVisible, !disabled);
}

void ToolMenuHandler::addToolItem(AbstractToolItem* it) { this->toolItems.push_back(it); }

void ToolMenuHandler::registerMenupoint(GtkWidget* widget, ActionType type, ActionGroup group) {
    this->menuItems.push_back(new MenuItem(listener, widget, type, group));
}

void ToolMenuHandler::initPenToolItem() {
    auto* tbPen = new ToolButton(listener, "PEN", ACTION_TOOL_PEN, GROUP_TOOL, true, iconName("tool-pencil"), _("Pen"));

    registerMenupoint(tbPen->registerPopupMenuEntry(_("standard"), iconName("line-style-plain")),
                      ACTION_TOOL_LINE_STYLE_PLAIN, GROUP_LINE_STYLE);

    registerMenupoint(tbPen->registerPopupMenuEntry(_("dashed"), iconName("line-style-dash")),
                      ACTION_TOOL_LINE_STYLE_DASH, GROUP_LINE_STYLE);

    registerMenupoint(tbPen->registerPopupMenuEntry(_("dash-/ dotted"), iconName("line-style-dash-dot")),
                      ACTION_TOOL_LINE_STYLE_DASH_DOT, GROUP_LINE_STYLE);

    registerMenupoint(tbPen->registerPopupMenuEntry(_("dotted"), iconName("line-style-dot")),
                      ACTION_TOOL_LINE_STYLE_DOT, GROUP_LINE_STYLE);

    addToolItem(tbPen);
}

void ToolMenuHandler::initEraserToolItem() {
    auto* tbEraser = new ToolButton(listener, "ERASER", ACTION_TOOL_ERASER, GROUP_TOOL, true, iconName("tool-eraser"),
                                    _("Eraser"));

    registerMenupoint(tbEraser->registerPopupMenuEntry(_("standard")), ACTION_TOOL_ERASER_STANDARD, GROUP_ERASER_MODE);
    registerMenupoint(tbEraser->registerPopupMenuEntry(_("whiteout")), ACTION_TOOL_ERASER_WHITEOUT, GROUP_ERASER_MODE);
    registerMenupoint(tbEraser->registerPopupMenuEntry(_("delete stroke")), ACTION_TOOL_ERASER_DELETE_STROKE,
                      GROUP_ERASER_MODE);

    addToolItem(tbEraser);
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

void ToolMenuHandler::initToolItems() {

    // Use GTK Stock icon
    auto addStockItem = [=](std::string name, ActionType action, std::string stockIcon, std::string text) {
        addToolItem(new ToolButton(listener, name, action, stockIcon, text));
    };

    // Use Custom loading Icon
    auto addCustomItem = [=](std::string name, ActionType action, const char* icon, std::string text) {
        addToolItem(new ToolButton(listener, name, action, iconName(icon), text));
    };

    // Use Custom loading Icon, toggle item
    // switchOnly: You can select pen, eraser etc. but you cannot unselect pen.
    auto addCustomItemTgl = [=](std::string name, ActionType action, ActionGroup group, bool switchOnly,
                                const char* icon, std::string text) {
        addToolItem(new ToolButton(listener, name, action, group, switchOnly, iconName(icon), text));
    };

    // Use Stock Icon, toggle item
    // switchOnly: You can select pen, eraser etc. but you cannot unselect pen.
    auto addStockItemTgl = [=](std::string name, ActionType action, ActionGroup group, bool switchOnly,
                               std::string stockIcon, std::string text) {
        addToolItem(new ToolButton(listener, name, action, group, switchOnly, stockIcon, text));
    };

    // Items ordered by menu, if possible.
    // There are some entries which are not available in the menu, like the Zoom slider
    // All menu items without tool icon are not listed here - they are connected by Glade Signals

    // Menu File
    // ************************************************************************

    addCustomItem("NEW", ACTION_NEW, "document-new", _("New Xournal"));
    addCustomItem("OPEN", ACTION_OPEN, "document-open", _("Open file"));
    addCustomItem("SAVE", ACTION_SAVE, "document-save", _("Save"));
    addStockItem("PRINT", ACTION_PRINT, "document-print", _("Print"));

    // Menu Edit
    // ************************************************************************

    // Undo / Redo Texts are updated from code, therefore a reference is hold for this items
    undoButton = new ToolButton(listener, "UNDO", ACTION_UNDO, iconName("edit-undo"), _("Undo"));
    redoButton = new ToolButton(listener, "REDO", ACTION_REDO, iconName("edit-redo"), _("Redo"));
    addToolItem(undoButton);
    addToolItem(redoButton);

    addCustomItem("CUT", ACTION_CUT, "edit-cut", _("Cut"));
    addCustomItem("COPY", ACTION_COPY, "edit-copy", _("Copy"));
    addCustomItem("PASTE", ACTION_PASTE, "edit-paste", _("Paste"));

    addStockItem("SEARCH", ACTION_SEARCH, "edit-find", _("Search"));

    addStockItem("DELETE", ACTION_DELETE, "edit-delete", _("Delete"));

    addCustomItemTgl("ROTATION_SNAPPING", ACTION_ROTATION_SNAPPING, GROUP_SNAPPING, false, "snapping-rotation",
                     _("Rotation Snapping"));
    addCustomItemTgl("GRID_SNAPPING", ACTION_GRID_SNAPPING, GROUP_GRID_SNAPPING, false, "snapping-grid",
                     _("Grid Snapping"));

    // Menu View
    // ************************************************************************

    addCustomItemTgl("PAIRED_PAGES", ACTION_VIEW_PAIRED_PAGES, GROUP_PAIRED_PAGES, false, "show-paired-pages",
                     _("Paired pages"));
    addCustomItemTgl("PRESENTATION_MODE", ACTION_VIEW_PRESENTATION_MODE, GROUP_PRESENTATION_MODE, false,
                     "presentation-mode", _("Presentation mode"));
    addCustomItemTgl("FULLSCREEN", ACTION_FULLSCREEN, GROUP_FULLSCREEN, false, "fullscreen", _("Toggle fullscreen"));

    addCustomItem("MANAGE_TOOLBAR", ACTION_MANAGE_TOOLBAR, "toolbars-manage", _("Manage Toolbars"));
    addCustomItem("CUSTOMIZE_TOOLBAR", ACTION_CUSTOMIZE_TOOLBAR, "toolbars-customize", _("Customize Toolbars"));

    addStockItem("ZOOM_OUT", ACTION_ZOOM_OUT, "zoom-out", _("Zoom out"));
    addStockItem("ZOOM_IN", ACTION_ZOOM_IN, "zoom-in", _("Zoom in"));
    addStockItemTgl("ZOOM_FIT", ACTION_ZOOM_FIT, GROUP_ZOOM_FIT, false, "zoom-fit-best", _("Zoom fit to screen"));
    addStockItem("ZOOM_100", ACTION_ZOOM_100, "zoom-original", _("Zoom to 100%"));

    // Menu Navigation
    // ************************************************************************

    addStockItem("GOTO_FIRST", ACTION_GOTO_FIRST, "go-first", _("Go to first page"));
    addStockItem("GOTO_BACK", ACTION_GOTO_BACK, "go-previous", _("Back"));
    addCustomItem("GOTO_PAGE", ACTION_GOTO_PAGE, "go-to", _("Go to page"));
    addStockItem("GOTO_NEXT", ACTION_GOTO_NEXT, "go-next", _("Next"));
    addStockItem("GOTO_LAST", ACTION_GOTO_LAST, "go-last", _("Go to last page"));

    addStockItem("GOTO_PREVIOUS_LAYER", ACTION_GOTO_PREVIOUS_LAYER, "go-previous", _("Go to previous layer"));
    addStockItem("GOTO_NEXT_LAYER", ACTION_GOTO_NEXT_LAYER, "go-next", _("Go to next layer"));
    addStockItem("GOTO_TOP_LAYER", ACTION_GOTO_TOP_LAYER, "go-top", _("Go to top layer"));

    addCustomItem("GOTO_NEXT_ANNOTATED_PAGE", ACTION_GOTO_NEXT_ANNOTATED_PAGE, "page-annotated-next",
                  _("Next annotated page"));

    // Menu Journal
    // ************************************************************************

    auto* tbInsertNewPage = new ToolButton(listener, "INSERT_NEW_PAGE", ACTION_NEW_PAGE_AFTER,
                                           iconName("page-add").c_str(), _("Insert page"));
    addToolItem(tbInsertNewPage);
    tbInsertNewPage->setPopupMenu(this->newPageType->getMenu());

    addCustomItem("DELETE_CURRENT_PAGE", ACTION_DELETE_PAGE, "page-delete", _("Delete current page"));

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(gui->get("menuJournalPaperBackground")),
                              pageBackgroundChangeController->getMenu());

    // Menu Tool
    // ************************************************************************

    initPenToolItem();
    initEraserToolItem();

    addCustomItemTgl("HIGHLIGHTER", ACTION_TOOL_HIGHLIGHTER, GROUP_TOOL, true, "tool-highlighter", _("Highlighter"));

    addCustomItemTgl("TEXT", ACTION_TOOL_TEXT, GROUP_TOOL, true, "tool-text", _("Text"));
    addCustomItem("MATH_TEX", ACTION_TEX, "tool-math-tex", _("Add/Edit Tex"));
    addCustomItemTgl("IMAGE", ACTION_TOOL_IMAGE, GROUP_TOOL, true, "tool-image", _("Image"));
    addCustomItem("DEFAULT_TOOL", ACTION_TOOL_DEFAULT, "default", _("Default Tool"));
    addCustomItemTgl("SHAPE_RECOGNIZER", ACTION_SHAPE_RECOGNIZER, GROUP_RULER, false, "shape-recognizer",
                     _("Shape Recognizer"));
    addCustomItemTgl("DRAW_RECTANGLE", ACTION_TOOL_DRAW_RECT, GROUP_RULER, false, "draw-rect", _("Draw Rectangle"));
    addCustomItemTgl("DRAW_ELLIPSE", ACTION_TOOL_DRAW_ELLIPSE, GROUP_RULER, false, "draw-ellipse", _("Draw Ellipse"));
    addCustomItemTgl("DRAW_ARROW", ACTION_TOOL_DRAW_ARROW, GROUP_RULER, false, "draw-arrow", _("Draw Arrow"));
    addCustomItemTgl("DRAW_COORDINATE_SYSTEM", ACTION_TOOL_DRAW_COORDINATE_SYSTEM, GROUP_RULER, false,
                     "draw-coordinate-system", _("Draw coordinate system"));
    addCustomItemTgl("RULER", ACTION_RULER, GROUP_RULER, false, "draw-line", _("Draw Line"));
    addCustomItemTgl("DRAW_SPLINE", ACTION_TOOL_DRAW_SPLINE, GROUP_RULER, false, "draw-spline", _("Draw Spline"));

    addCustomItemTgl("SELECT_REGION", ACTION_TOOL_SELECT_REGION, GROUP_TOOL, true, "select-lasso", _("Select Region"));
    addCustomItemTgl("SELECT_RECTANGLE", ACTION_TOOL_SELECT_RECT, GROUP_TOOL, true, "select-rect",
                     _("Select Rectangle"));
    addCustomItemTgl("SELECT_OBJECT", ACTION_TOOL_SELECT_OBJECT, GROUP_TOOL, true, "object-select", _("Select Object"));
    addCustomItemTgl("VERTICAL_SPACE", ACTION_TOOL_VERTICAL_SPACE, GROUP_TOOL, true, "spacer", _("Vertical Space"));
    addCustomItemTgl("PLAY_OBJECT", ACTION_TOOL_PLAY_OBJECT, GROUP_TOOL, true, "object-play", _("Play Object"));
    addCustomItemTgl("HAND", ACTION_TOOL_HAND, GROUP_TOOL, true, "hand", _("Hand"));

    fontButton = new FontButton(listener, gui, "SELECT_FONT", ACTION_FONT_BUTTON_CHANGED, _("Select Font"));
    addToolItem(fontButton);

    addCustomItemTgl("AUDIO_RECORDING", ACTION_AUDIO_RECORD, GROUP_AUDIO, false, "audio-record",
                     _("Record Audio / Stop Recording"));
    audioPausePlaybackButton = new ToolButton(listener, "AUDIO_PAUSE_PLAYBACK", ACTION_AUDIO_PAUSE_PLAYBACK,
                                              GROUP_AUDIO, false, iconName("audio-playback-pause"), _("Pause / Play"));
    addToolItem(audioPausePlaybackButton);
    audioStopPlaybackButton = new ToolButton(listener, "AUDIO_STOP_PLAYBACK", ACTION_AUDIO_STOP_PLAYBACK,
                                             iconName("audio-playback-stop"), _("Stop"));
    addToolItem(audioStopPlaybackButton);
    audioSeekForwardsButton = new ToolButton(listener, "AUDIO_SEEK_FORWARDS", ACTION_AUDIO_SEEK_FORWARDS,
                                             iconName("audio-seek-forwards"), _("Forward"));
    addToolItem(audioSeekForwardsButton);
    audioSeekBackwardsButton = new ToolButton(listener, "AUDIO_SEEK_BACKWARDS", ACTION_AUDIO_SEEK_BACKWARDS,
                                              iconName("audio-seek-backwards"), _("Back"));
    addToolItem(audioSeekBackwardsButton);

    // Menu Help
    // ************************************************************************
    // All tools are registered by the Glade Signals


    ///////////////////////////////////////////////////////////////////////////


    // Footer tools
    // ************************************************************************
    toolPageSpinner = new ToolPageSpinner(gui, listener, "PAGE_SPIN", ACTION_FOOTER_PAGESPIN, iconNameHelper);
    addToolItem(toolPageSpinner);

    auto* toolZoomSlider = new ToolZoomSlider(listener, "ZOOM_SLIDER", ACTION_FOOTER_ZOOM_SLIDER, zoom, iconNameHelper);
    addToolItem(toolZoomSlider);

    toolPageLayer = new ToolPageLayer(control->getLayerController(), gui, listener, "LAYER", ACTION_FOOTER_LAYER,
                                      iconNameHelper);
    addToolItem(toolPageLayer);

    addCustomItemTgl("TOOL_FILL", ACTION_TOOL_FILL, GROUP_FILL, false, "fill", _("Fill"));


    // Non-menu items
    // ************************************************************************

    // Color item - not in the menu
    addToolItem(new ColorToolItem(listener, toolHandler, this->parent, Color{0xff0000U}, true));

    addToolItem(new ToolSelectCombocontrol(this, listener, "SELECT"));
    addToolItem(new ToolDrawCombocontrol(this, listener, "DRAW"));

    // General tool configuration - working for every tool which support it
    addCustomItemTgl("VERY_FINE", ACTION_SIZE_VERY_FINE, GROUP_SIZE, true, "thickness-finer", _("Very Fine"));
    addCustomItemTgl("FINE", ACTION_SIZE_FINE, GROUP_SIZE, true, "thickness-fine", _("Fine"));
    addCustomItemTgl("MEDIUM", ACTION_SIZE_MEDIUM, GROUP_SIZE, true, "thickness-medium", _("Medium"));
    addCustomItemTgl("THICK", ACTION_SIZE_THICK, GROUP_SIZE, true, "thickness-thick", _("Thick"));
    addCustomItemTgl("VERY_THICK", ACTION_SIZE_VERY_THICK, GROUP_SIZE, true, "thickness-thicker", _("Very Thick"));


    // now connect all Glade Signals
    gtk_builder_connect_signals_full(gui->getBuilder(), reinterpret_cast<GtkBuilderConnectFunc>(signalConnectCallback),
                                     this);
}

void ToolMenuHandler::setFontButtonFont(XojFont& font) { this->fontButton->setFont(font); }

auto ToolMenuHandler::getFontButtonFont() -> XojFont { return this->fontButton->getFont(); }

void ToolMenuHandler::showFontSelectionDlg() { this->fontButton->showFontDialog(); }

void ToolMenuHandler::setUndoDescription(const string& description) {
    this->undoButton->updateDescription(description);
    gtk_menu_item_set_label(GTK_MENU_ITEM(gui->get("menuEditUndo")), description.c_str());
}

void ToolMenuHandler::setRedoDescription(const string& description) {
    this->redoButton->updateDescription(description);
    gtk_menu_item_set_label(GTK_MENU_ITEM(gui->get("menuEditRedo")), description.c_str());
}

auto ToolMenuHandler::getPageSpinner() -> SpinPageAdapter* { return this->toolPageSpinner->getPageSpinner(); }

void ToolMenuHandler::setPageInfo(const size_t pagecount, const size_t pdfpage) {
    this->toolPageSpinner->setPageInfo(pagecount, pdfpage);
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

    this->audioPausePlaybackButton->enable(false);
    this->audioStopPlaybackButton->enable(false);
    this->audioSeekBackwardsButton->enable(false);
    this->audioSeekForwardsButton->enable(false);

    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioPausePlayback")), false);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioStopPlayback")), false);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioSeekForwards")), false);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioSeekBackwards")), false);
}

void ToolMenuHandler::enableAudioPlaybackButtons() {
    this->audioPausePlaybackButton->enable(true);
    this->audioStopPlaybackButton->enable(true);
    this->audioSeekBackwardsButton->enable(true);
    this->audioSeekForwardsButton->enable(true);

    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioPausePlayback")), true);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioStopPlayback")), true);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioSeekForwards")), true);
    gtk_widget_set_sensitive(GTK_WIDGET(gui->get("menuAudioSeekBackwards")), true);
}

void ToolMenuHandler::setAudioPlaybackPaused(bool paused) {
    this->audioPausePlaybackButton->setActive(paused);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gui->get("menuAudioPausePlayback")), paused);
}

auto ToolMenuHandler::iconName(const char* icon) -> std::string { return iconNameHelper.iconName(icon); }
