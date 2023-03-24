#include "ToolMenuHandler.h"

#include <algorithm>  // for max
#include <sstream>    // for istringstream

#include "control/Actions.h"                 // for ActionH...
#include "control/Control.h"                 // for Control
#include "control/actions/ActionDatabase.h"  // for ActionDatabase
#include "control/settings/Settings.h"       // for Settings
#include "gui/GladeGui.h"                    // for GladeGui
#include "gui/GladeSearchpath.h"
#include "gui/ToolitemDragDrop.h"  // for ToolitemDragDrop
#include "gui/menus/popoverMenus/PageTypeSelectionPopover.h"
#include "gui/toolbarMenubar/model/ColorPalette.h"  // for Palette
#include "gui/toolbarMenubar/model/ToolbarData.h"   // for ToolbarData
#include "gui/toolbarMenubar/model/ToolbarEntry.h"  // for ToolbarEntry
#include "gui/toolbarMenubar/model/ToolbarItem.h"   // for ToolbarItem
#include "gui/toolbarMenubar/model/ToolbarModel.h"  // for ToolbarModel
#include "model/Font.h"                             // for XojFont
#include "plugin/Plugin.h"                          // for ToolbarButtonEntr<
#include "util/GVariantTemplate.h"                  // for gVariantType
#include "util/GtkUtil.h"
#include "util/NamedColor.h"  // for NamedColor
#include "util/PathUtil.h"
#include "util/StringUtils.h"  // for StringUtils
#include "util/XojMsgBox.h"
#include "util/gtk4_helper.h"
#include "util/i18n.h"  // for _

#include "AbstractToolItem.h"        // for AbstractToolItem
#include "ColorToolItem.h"           // for ColorToolItem
#include "FontButton.h"              // for FontButton
#include "MenuItem.h"                // for MenuItem
#include "PluginToolButton.h"        // for ToolButton
#include "ToolButton.h"              // for ToolButton
#include "ToolDrawCombocontrol.h"    // for ToolDrawCombocon...
#include "ToolPageLayer.h"           // for ToolPageLayer
#include "ToolPageSpinner.h"         // for ToolPageSpinner
#include "ToolPdfCombocontrol.h"     // for ToolPdfCombocontrol
#include "ToolSelectCombocontrol.h"  // for ToolSelectComboc...
#include "ToolZoomSlider.h"          // for ToolZoomSlider
#include "config-dev.h"              // for TOOLBAR_CONFIG
#include "config-features.h"         // for ENABLE_PLUGINS
#include "filesystem.h"              // for exists


using std::string;

ToolMenuHandler::ToolMenuHandler(Control* control, GladeGui* gui):
        parent(GTK_WINDOW(gui->getWindow())),
        control(control),
        listener(control),
        zoom(control->getZoomControl()),
        gui(gui),
        toolHandler(control->getToolHandler()),
        tbModel(std::make_unique<ToolbarModel>()),
        pageBackgroundChangeController(control->getPageBackgroundChangeController()),
        iconNameHelper(control->getSettings()),
        pageTypeSelectionPopup(std::make_unique<PageTypeSelectionPopover>(
                control->getPageTypes(), control->getPageBackgroundChangeController(), control->getSettings(),
                GTK_APPLICATION_WINDOW(parent))) {}

void ToolMenuHandler::populate(const GladeSearchpath* gladeSearchPath) {
    initToolItems();

    auto file = gladeSearchPath->findFile("", "toolbar.ini");
    if (!tbModel->parse(file, true)) {

        string msg = FS(_F("Could not parse general toolbar.ini file: {1}\n"
                           "No Toolbars will be available") %
                        file.u8string());
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
    }

    file = Util::getConfigFile(TOOLBAR_CONFIG);
    if (fs::exists(file)) {
        if (!tbModel->parse(file, false)) {
            string msg = FS(_F("Could not parse custom toolbar.ini file: {1}\n"
                               "Toolbars will not be available") %
                            file.u8string());
            XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
        }
    }
}

ToolMenuHandler::~ToolMenuHandler() {
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

void ToolMenuHandler::load(ToolbarData* d, GtkWidget* toolbar, const char* toolbarName, bool horizontal) {
    int count = 0;

    const Palette& palette = this->control->getSettings()->getColorPalette();
    size_t colorIndex{};

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

                // recognize previous name, V1.1.0+dev (Nov 2022) and earlier
                if (name == "PEN_FILL_OPACITY") {
                    name = "FILL_OPACITY";
                }

                if (!this->control->getAudioController() &&
                    (name == "AUDIO_RECORDING" || name == "AUDIO_SEEK_BACKWARDS" || name == "AUDIO_PAUSE_PLAYBACK" ||
                     name == "AUDIO_STOP_PLAYBACK" || name == "AUDIO_SEEK_FORWARDS" || name == "PLAY_OBJECT")) {
                    continue;
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
                if (StringUtils::startsWith(name, "COLOR(") && StringUtils::endsWith(name, ")")) {
                    string arg = name.substr(6, name.length() - 7);
                    /*
                     * namedColor needs to be a pointer to enable attachMetadataColor to reference to the
                     * non local scoped namedColor instance
                     */

                    size_t paletteIndex{};

                    // check for old color format of toolbar.ini
                    if (StringUtils::startsWith(arg, "0x")) {
                        // use tmpColor only to get Index
                        NamedColor tmpColor = palette.getColorAt(colorIndex);
                        paletteIndex = tmpColor.getIndex();

                        // set new COLOR Toolbar entry
                        std::ostringstream newColor("");
                        newColor << "COLOR(" << paletteIndex << ")";
                        dataItem->setName(newColor.str());
                        colorIndex++;
                    } else {
                        std::istringstream colorStream(arg);
                        colorStream >> paletteIndex;
                        if (!colorStream.eof() || colorStream.fail()) {
                            g_warning("Toolbar:COLOR(...) has to start with 0x, get color: %s", arg.c_str());
                            continue;
                        }
                    }

                    count++;
                    const NamedColor& namedColor = palette.getColorAt(paletteIndex);
                    auto* item = new ColorToolItem(listener, toolHandler, this->parent, namedColor);
                    this->toolbarColorItems.push_back(item);

                    GtkToolItem* it = item->createItem(horizontal);
                    gtk_widget_show_all(GTK_WIDGET(it));
                    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), it, -1);

                    ToolitemDragDrop::attachMetadataColor(GTK_WIDGET(it), dataItem->getId(), &namedColor, item);

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
    for (AbstractToolItem* it: this->toolItems) {
        it->setTmpDisabled(disabled);
    }

    for (MenuItem* it: this->menuItems) {
        it->setTmpDisabled(disabled);
    }

    for (ColorToolItem* it: this->toolbarColorItems) {
        it->setTmpDisabled(disabled);
    }

    control->getActionDatabase()->enableAction(Action::SHOW_SIDEBAR, !disabled);
}

void ToolMenuHandler::addToolItem(AbstractToolItem* it) { this->toolItems.push_back(it); }

void ToolMenuHandler::registerMenupoint(GtkWidget* widget, ActionType type, ActionGroup group) {
    this->menuItems.push_back(new MenuItem(listener, widget, type, group));
}

/**
 * @return floating ref
 */
template <typename state_type>
static GtkWidget* makeRadioButton(const char* label, const char* actionName, state_type target, const char* icon) {
    GtkWidget* btn = gtk_radio_button_new_from_widget(nullptr);
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), xoj::util::makeGVariantSPtr(target).get());
    xoj::util::gtk::setRadioButtonActionName(GTK_RADIO_BUTTON(btn), "win", actionName);

    if (icon) {
        GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_box_append(GTK_BOX(hbox), gtk_image_new_from_icon_name(icon, GTK_ICON_SIZE_LARGE_TOOLBAR));
        gtk_box_append(GTK_BOX(hbox), gtk_label_new(label));
        gtk_button_set_child(GTK_BUTTON(btn), hbox);
    } else {
        gtk_button_set_label(GTK_BUTTON(btn), label);
    }
    return btn;
}

static auto createPenLineStylePopover(IconNameHelper& icons) {
    xoj::util::WidgetSPtr popover(gtk_popover_new(), xoj::util::adopt);
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_popover_set_child(GTK_POPOVER(popover.get()), box);

#if GTK_MAJOR_VERSION == 3
    GtkRadioButton* group = nullptr;
    auto appendLineStyleItem = [&](const char* label, const char* target, const char* icon) {
        GtkWidget* btn = gtk_radio_button_new_from_widget(group);
        group = GTK_RADIO_BUTTON(btn);
        xoj::util::gtk::setRadioButtonActionName(GTK_RADIO_BUTTON(btn), "win",
                                                 Action_toString(Action::TOOL_PEN_LINE_STYLE));
#else
    GtkCheckButton* group = nullptr;
    std::string actionName = std::string("win.") + Action_toString(Action::TOOL_PEN_LINE_STYLE);
    auto appendLineStyleItem = [&](const char* label, const char* target, const char* icon) {
        GtkWidget* btn = gtk_check_button_new_with_label(layerName.c_str());
        // Is grouping necessary here? The GTK4 doc is unclear
        gtk_check_button_set_group(GTK_CHECK_BUTTON(btn), std::exchange(group, GTK_CHECK_BUTTON(btn));
        gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.c_str());
#endif
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), xoj::util::makeGVariantSPtr(target).get());
        GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_box_append(GTK_BOX(hbox),
                       gtk_image_new_from_icon_name(icons.iconName(icon).c_str(), GTK_ICON_SIZE_LARGE_TOOLBAR));
        gtk_box_append(GTK_BOX(hbox), gtk_label_new(label));
        gtk_button_set_child(GTK_BUTTON(btn), hbox);
        gtk_box_append(GTK_BOX(box), btn);
    };

    appendLineStyleItem(_("standard"), "plain", "line-style-plain");
    appendLineStyleItem(_("dashed"), "dash", "line-style-dash");
    appendLineStyleItem(_("dash-/ dotted"), "dashdot", "line-style-dash-dot");
    appendLineStyleItem(_("dotted"), "dot", "line-style-dot");

    gtk_widget_show_all(box);
    return popover;
}

static auto createEraserTypePopover() {
    xoj::util::WidgetSPtr popover(gtk_popover_new(), xoj::util::adopt);
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_popover_set_child(GTK_POPOVER(popover.get()), box);

#if GTK_MAJOR_VERSION == 3
    GtkRadioButton* group = nullptr;
    auto appendEraserTypeItem = [&](const char* label, EraserType target) {
        GtkWidget* btn = gtk_radio_button_new_from_widget(group);
        group = GTK_RADIO_BUTTON(btn);
        xoj::util::gtk::setRadioButtonActionName(GTK_RADIO_BUTTON(btn), "win",
                                                 Action_toString(Action::TOOL_ERASER_TYPE));
#else
    GtkCheckButton* group = nullptr;
    std::string actionName = std::string("win.") + Action_toString(Action::TOOL_ERASER_TYPE);
    auto appendEraserTypeItem = [&](const char* label, EraserType target) {
        GtkWidget* btn = gtk_check_button_new_with_label(layerName.c_str());
        // Is grouping necessary here? The GTK4 doc is unclear
        gtk_check_button_set_group(GTK_CHECK_BUTTON(btn), std::exchange(group, GTK_CHECK_BUTTON(btn));
        gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.c_str());
#endif
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), xoj::util::makeGVariantSPtr(target).get());
        gtk_button_set_label(GTK_BUTTON(btn), label);
        gtk_box_append(GTK_BOX(box), btn);
    };

    appendEraserTypeItem(_("standard"), ERASER_TYPE_DEFAULT);
    appendEraserTypeItem(_("whiteout"), ERASER_TYPE_WHITEOUT);
    appendEraserTypeItem(_("delete stroke"), ERASER_TYPE_DELETE_STROKE);

    gtk_widget_show_all(box);
    return popover;
}

#ifdef ENABLE_PLUGINS
void ToolMenuHandler::addPluginItem(ToolbarButtonEntry* t) { addToolItem(new PluginToolButton(t)); }
#endif /* ENABLE_PLUGINS */

void ToolMenuHandler::initToolItems() {
    /**
     * @brief Simple button, with a GTK stock icon name
     *      The corresponding action in ActionDatabase[action] should have no state (it can have a parameter)
     **/
    auto emplaceStockItem = [this](const char* name, Action action, const char* icon, std::string description) {
        addToolItem(new ToolButton(name, action, icon, description, false));
    };
    /**
     * @brief Simple button, with a custom loaded icon
     *      The corresponding action in ActionDatabase[action] should have no state (it can have a parameter)
     **/
    auto emplaceCustomItem = [this](const char* name, Action action, const char* icon, std::string description) {
        addToolItem(new ToolButton(name, action, iconName(icon), description, false));
    };

    /**
     * @brief Toggle button, with a GTK stock icon name
     *      The corresponding action in ActionDatabase[action] should have a boolean state and no parameter
     **/
    auto emplaceStockItemTgl = [this](const char* name, Action action, const char* icon, std::string description) {
        addToolItem(new ToolButton(name, action, icon, description, true));
    };

    /**
     * @brief Toggle button, with a custom loaded icon
     *      The corresponding action in ActionDatabase[action] should have a boolean state and no parameter
     **/
    auto emplaceCustomItemTgl = [this](const char* name, Action action, const char* icon, std::string description) {
        addToolItem(new ToolButton(name, action, iconName(icon), description, true));
    };

    /**
     * @brief Toggle button linked to others sharing the same action (with a custom loaded icon)
     *      The corresponding action in ActionDatabase[action] should have a state and a parameter. The button is "on"
     *when the action state matches `target`.
     **/
    auto emplaceCustomItemWithTarget = [this](const char* name, Action action, auto target, const char* icon,
                                              std::string description) {
        addToolItem(new ToolButton(name, action, makeGVariant(target), iconName(icon), description));
    };

    /**
     * @brief Simple button (with a custom loaded icon), with a popover menu to change parameters of the tool
     *      The corresponding action in ActionDatabase[action] should have no state (it can have a parameter)
     **/
    auto emplaceCustomItemWithPopover = [this](const char* name, Action action, const char* icon,
                                               std::string description, GtkWidget* popover) {
        auto* tb = new ToolButton(name, action, iconName(icon), description, false);
        tb->setPopover(popover);
        addToolItem(tb);
    };

    auto emplaceCustomItemWithTargetAndMenu = [this](const char* name, Action action, auto target, const char* icon,
                                                     std::string description, GtkWidget* popover) {
        auto* tb = new ToolButton(name, action, makeGVariant(target), iconName(icon), description);
        tb->setPopover(popover);
        addToolItem(tb);
    };


    /*
     * Items ordered by menu, if possible.
     * There are some entries which are not available in the menu, like the Zoom slider
     * All menu items without tool icon are not listed here - they are connected by Glade Signals
     */

    /*
     * Menu File
     * ------------------------------------------------------------------------
     */

    emplaceCustomItem("NEW", Action::NEW_FILE, "document-new", _("New Xournal"));
    emplaceCustomItem("OPEN", Action::OPEN, "document-open", _("Open file"));
    emplaceCustomItem("SAVE", Action::SAVE, "document-save", _("Save"));
    emplaceCustomItem("SAVEPDF", Action::EXPORT_AS_PDF, "document-export-pdf", _("Export as PDF"));
    emplaceCustomItem("PRINT", Action::PRINT, "document-print", _("Print"));

    /*
     * Menu Edit
     * ------------------------------------------------------------------------
     */

    // Undo / Redo Texts are updated from code, therefore a reference is held for this items
    undoButton = new ToolButton("UNDO", Action::UNDO, iconName("edit-undo"), _("Undo"), false);
    redoButton = new ToolButton("REDO", Action::REDO, iconName("edit-redo"), _("Redo"), false);
    addToolItem(undoButton);
    addToolItem(redoButton);

    emplaceCustomItem("CUT", Action::CUT, "edit-cut", _("Cut"));
    emplaceCustomItem("COPY", Action::COPY, "edit-copy", _("Copy"));
    emplaceCustomItem("PASTE", Action::PASTE, "edit-paste", _("Paste"));

    emplaceStockItem("SEARCH", Action::SEARCH, "edit-find", _("Search"));

    emplaceStockItem("DELETE", Action::DELETE, "edit-delete", _("Delete"));

    emplaceCustomItemTgl("ROTATION_SNAPPING", Action::ROTATION_SNAPPING, "snapping-rotation", _("Rotation Snapping"));
    emplaceCustomItemTgl("GRID_SNAPPING", Action::GRID_SNAPPING, "snapping-grid", _("Grid Snapping"));

    /*
     * Menu View
     * ------------------------------------------------------------------------
     */

    emplaceCustomItemTgl("PAIRED_PAGES", Action::PAIRED_PAGES_MODE, "show-paired-pages", _("Paired pages"));
    emplaceCustomItemTgl("PRESENTATION_MODE", Action::PRESENTATION_MODE, "presentation-mode", _("Presentation mode"));
    emplaceCustomItemTgl("FULLSCREEN", Action::FULLSCREEN, "fullscreen", _("Toggle fullscreen"));

    emplaceCustomItem("MANAGE_TOOLBAR", Action::MANAGE_TOOLBAR, "toolbars-manage", _("Manage Toolbars"));
    emplaceCustomItem("CUSTOMIZE_TOOLBAR", Action::CUSTOMIZE_TOOLBAR, "toolbars-customize", _("Customize Toolbars"));

    emplaceStockItem("ZOOM_OUT", Action::ZOOM_OUT, "zoom-out", _("Zoom out"));
    emplaceStockItem("ZOOM_IN", Action::ZOOM_IN, "zoom-in", _("Zoom in"));
    emplaceStockItemTgl("ZOOM_FIT", Action::ZOOM_FIT, "zoom-fit-best", _("Zoom fit to screen"));
    emplaceStockItem("ZOOM_100", Action::ZOOM_100, "zoom-original", _("Zoom to 100%"));

    /*
     * Menu Navigation
     * ------------------------------------------------------------------------
     */

    emplaceStockItem("GOTO_FIRST", Action::GOTO_FIRST, "go-first", _("Go to first page"));
    emplaceStockItem("GOTO_BACK", Action::GOTO_PREVIOUS, "go-previous", _("Back"));
    emplaceCustomItem("GOTO_PAGE", Action::GOTO_PAGE, "go-to", _("Go to page"));
    emplaceStockItem("GOTO_NEXT", Action::GOTO_NEXT, "go-next", _("Next"));
    emplaceStockItem("GOTO_LAST", Action::GOTO_LAST, "go-last", _("Go to last page"));

    emplaceStockItem("GOTO_PREVIOUS_LAYER", Action::LAYER_GOTO_PREVIOUS, "go-previous", _("Go to previous layer"));
    emplaceStockItem("GOTO_NEXT_LAYER", Action::LAYER_GOTO_NEXT, "go-next", _("Go to next layer"));
    emplaceStockItem("GOTO_TOP_LAYER", Action::LAYER_GOTO_TOP, "go-top", _("Go to top layer"));

    emplaceCustomItem("GOTO_NEXT_ANNOTATED_PAGE", Action::GOTO_NEXT_ANNOTATED_PAGE, "page-annotated-next",
                      _("Next annotated page"));

    /* Menu Journal
     * ------------------------------------------------------------------------
     */

    emplaceCustomItemWithPopover("INSERT_NEW_PAGE", Action::NEW_PAGE_AFTER, "page-add", _("Insert page"),
                                 this->pageTypeSelectionPopup->getPopover());
    emplaceCustomItem("DELETE_CURRENT_PAGE", Action::DELETE_PAGE, "page-delete", _("Delete current page"));

    /*
     * Menu Tool
     * ------------------------------------------------------------------------
     */

    emplaceCustomItemWithTargetAndMenu("PEN", Action::SELECT_TOOL, TOOL_PEN, "tool-pencil", _("Pen"),
                                       createPenLineStylePopover(this->iconNameHelper).get());
    // Add individual line styles as toolbar items
    emplaceCustomItemWithTarget("PLAIN", Action::TOOL_PEN_LINE_STYLE, "plain", "line-style-plain", _("standard"));
    emplaceCustomItemWithTarget("DASHED", Action::TOOL_PEN_LINE_STYLE, "dash", "line-style-dash", _("dashed"));
    emplaceCustomItemWithTarget("DASH-/ DOTTED", Action::TOOL_PEN_LINE_STYLE, "dashdot", "line-style-dash-dot",
                                _("dash-/ dotted"));
    emplaceCustomItemWithTarget("DOTTED", Action::TOOL_PEN_LINE_STYLE, "dot", "line-style-dot", _("dotted"));


    emplaceCustomItemWithTargetAndMenu("ERASER", Action::SELECT_TOOL, TOOL_ERASER, "tool-eraser", _("Eraser"),
                                       createEraserTypePopover().get());
    // no icons for individual eraser modes available, therefore can't add them as toolbar items


    emplaceCustomItemWithTarget("HIGHLIGHTER", Action::SELECT_TOOL, TOOL_HIGHLIGHTER, "tool-highlighter",
                                _("Highlighter"));

    emplaceCustomItemWithTarget("TEXT", Action::SELECT_TOOL, TOOL_TEXT, "tool-text", _("Text"));
    emplaceCustomItemWithTarget("LINK", Action::SELECT_TOOL, GROUP_TOOL, "tool-text", _("Add/Edit Link"));
    emplaceCustomItem("MATH_TEX", Action::TEX, "tool-math-tex", _("Add/Edit TeX"));
    emplaceCustomItemWithTarget("IMAGE", Action::SELECT_TOOL, TOOL_IMAGE, "tool-image", _("Image"));
    emplaceCustomItem("DEFAULT_TOOL", Action::SELECT_DEFAULT_TOOL, "default", _("Default Tool"));
    emplaceCustomItemWithTarget("SELECT_PDF_TEXT_LINEAR", Action::SELECT_TOOL, TOOL_SELECT_PDF_TEXT_LINEAR,
                                "select-pdf-text-ht", _("Select Linear PDF Text"));
    emplaceCustomItemWithTarget("SELECT_PDF_TEXT_RECT", Action::SELECT_TOOL, TOOL_SELECT_PDF_TEXT_RECT,
                                "select-pdf-text-area", _("Select PDF Text in Rectangle"));

    emplaceCustomItemTgl("SETSQUARE", Action::SETSQUARE, "setsquare", _("Setsquare"));
    emplaceCustomItemTgl("COMPASS", Action::COMPASS, "compass", _("Compass"));


    emplaceCustomItemTgl("SHAPE_RECOGNIZER", Action::TOOL_DRAW_SHAPE_RECOGNIZER, "shape-recognizer",
                         _("Shape Recognizer"));
    emplaceCustomItemTgl("DRAW_RECTANGLE", Action::TOOL_DRAW_RECTANGLE, "draw-rect", _("Draw Rectangle"));
    emplaceCustomItemTgl("DRAW_ELLIPSE", Action::TOOL_DRAW_ELLIPSE, "draw-ellipse", _("Draw Ellipse"));
    emplaceCustomItemTgl("DRAW_ARROW", Action::TOOL_DRAW_ARROW, "draw-arrow", _("Draw Arrow"));
    emplaceCustomItemTgl("DRAW_DOUBLE_ARROW", Action::TOOL_DRAW_DOUBLE_ARROW, "draw-double-arrow",
                         _("Draw Double Arrow"));
    emplaceCustomItemTgl("DRAW_COORDINATE_SYSTEM", Action::TOOL_DRAW_COORDINATE_SYSTEM, "draw-coordinate-system",
                         _("Draw coordinate system"));
    emplaceCustomItemTgl("RULER", Action::TOOL_DRAW_LINE, "draw-line", _("Draw Line"));
    emplaceCustomItemTgl("DRAW_SPLINE", Action::TOOL_DRAW_SPLINE, "draw-spline", _("Draw Spline"));

    emplaceCustomItemWithTarget("SELECT_REGION", Action::SELECT_TOOL, TOOL_SELECT_REGION, "select-lasso",
                                _("Select Region"));
    emplaceCustomItemWithTarget("SELECT_RECTANGLE", Action::SELECT_TOOL, TOOL_SELECT_RECT, "select-rect",
                                _("Select Rectangle"));
    emplaceCustomItemWithTarget("SELECT_MULTILAYER_REGION", Action::SELECT_TOOL, TOOL_SELECT_MULTILAYER_REGION,
                                "select-multilayer-lasso", _("Select Multi-Layer Region"));
    emplaceCustomItemWithTarget("SELECT_MULTILAYER_RECTANGLE", Action::SELECT_TOOL, TOOL_SELECT_MULTILAYER_RECT,
                                "select-multilayer-rect", _("Select Multi-Layer Rectangle"));
    emplaceCustomItemWithTarget("SELECT_OBJECT", Action::SELECT_TOOL, TOOL_SELECT_OBJECT, "object-select",
                                _("Select Object"));
    emplaceCustomItemWithTarget("VERTICAL_SPACE", Action::SELECT_TOOL, TOOL_VERTICAL_SPACE, "spacer",
                                _("Vertical Space"));
    emplaceCustomItemWithTarget("PLAY_OBJECT", Action::SELECT_TOOL, TOOL_PLAY_OBJECT, "object-play", _("Play Object"));
    emplaceCustomItemWithTarget("HAND", Action::SELECT_TOOL, TOOL_HAND, "hand", _("Hand"));

    fontButton = new FontButton(listener, "SELECT_FONT", ACTION_FONT_BUTTON_CHANGED, _("Select Font"));
    addToolItem(fontButton);

    emplaceCustomItemTgl("AUDIO_RECORDING", Action::AUDIO_RECORD, "audio-record", _("Record Audio / Stop Recording"));
    emplaceCustomItemTgl("AUDIO_PAUSE_PLAYBACK", Action::AUDIO_PAUSE_PLAYBACK, "audio-playback-pause",
                         _("Pause / Play"));
    emplaceCustomItem("AUDIO_STOP_PLAYBACK", Action::AUDIO_STOP_PLAYBACK, "audio-playback-stop", _("Stop"));
    emplaceCustomItem("AUDIO_SEEK_FORWARDS", Action::AUDIO_SEEK_FORWARDS, "audio-seek-forwards", _("Forward"));
    emplaceCustomItem("AUDIO_SEEK_BACKWARDS", Action::AUDIO_SEEK_BACKWARDS, "audio-seek-backwards", _("Back"));


    ///////////////////////////////////////////////////////////////////////////


    /*
     * Footer tools
     * ------------------------------------------------------------------------
     */
    toolPageSpinner = new ToolPageSpinner(listener, "PAGE_SPIN", ACTION_FOOTER_PAGESPIN, iconNameHelper);
    addToolItem(toolPageSpinner);

    auto* toolZoomSlider = new ToolZoomSlider("ZOOM_SLIDER", listener, ACTION_FOOTER_ZOOM_SLIDER, zoom, iconNameHelper);
    addToolItem(toolZoomSlider);

    toolPageLayer =
            new ToolPageLayer(control->getLayerController(), listener, "LAYER", ACTION_FOOTER_LAYER, iconNameHelper);
    addToolItem(toolPageLayer);

    /*
     * Non-menu items
     * ------------------------------------------------------------------------
     */

    /*
     * Color item - not in the menu
     * aka. COLOR_SELECT
     */
    addToolItem(new ColorToolItem(listener, toolHandler, this->parent, NamedColor{}, true));
    bool hideAudio = !this->control->getAudioController();
    addToolItem(new ToolSelectCombocontrol(this, listener, "SELECT", hideAudio));
    addToolItem(new ToolDrawCombocontrol(this, listener, "DRAW"));
    addToolItem(new ToolPdfCombocontrol(this, listener, "PDF_TOOL"));

    // General tool configuration - working for every tool which support it
    emplaceCustomItemTgl("TOOL_FILL", Action::TOOL_FILL, "fill", _("Fill"));
    emplaceCustomItem("FILL_OPACITY", Action::TOOL_FILL_OPACITY, "fill-opacity", _("Fill Opacity"));

    emplaceCustomItemWithTarget("VERY_FINE", Action::TOOL_SIZE, TOOL_SIZE_VERY_FINE, "thickness-finer", _("Very Fine"));
    emplaceCustomItemWithTarget("FINE", Action::TOOL_SIZE, TOOL_SIZE_FINE, "thickness-fine", _("Fine"));
    emplaceCustomItemWithTarget("MEDIUM", Action::TOOL_SIZE, TOOL_SIZE_MEDIUM, "thickness-medium", _("Medium"));
    emplaceCustomItemWithTarget("THICK", Action::TOOL_SIZE, TOOL_SIZE_THICK, "thickness-thick", _("Thick"));
    emplaceCustomItemWithTarget("VERY_THICK", Action::TOOL_SIZE, TOOL_SIZE_VERY_THICK, "thickness-thicker",
                                _("Very Thick"));
}

void ToolMenuHandler::setFontButtonFont(const XojFont& font) { this->fontButton->setFont(font); }

auto ToolMenuHandler::getFontButtonFont() -> XojFont { return this->fontButton->getFont(); }

void ToolMenuHandler::showFontSelectionDlg() { this->fontButton->showFontDialog(); }

void ToolMenuHandler::setUndoDescription(const string& description) {
    this->undoButton->updateDescription(description);
}

void ToolMenuHandler::setRedoDescription(const string& description) {
    this->redoButton->updateDescription(description);
}

auto ToolMenuHandler::getPageSpinner() -> SpinPageAdapter* {
    return this->toolPageSpinner ? this->toolPageSpinner->getPageSpinner() : nullptr;
}

void ToolMenuHandler::setPageInfo(const size_t pagecount, const size_t pdfpage) {
    this->toolPageSpinner->setPageInfo(pagecount, pdfpage);
}

auto ToolMenuHandler::getModel() -> ToolbarModel* { return this->tbModel.get(); }

auto ToolMenuHandler::getControl() -> Control* { return this->control; }

auto ToolMenuHandler::isColorInUse(Color color) -> bool {
    for (ColorToolItem* it: this->toolbarColorItems) {
        if (it->getColor() == color) {
            return true;
        }
    }

    return false;
}

auto ToolMenuHandler::getToolItems() -> std::vector<AbstractToolItem*>* { return &this->toolItems; }

auto ToolMenuHandler::getColorToolItems() const -> const std::vector<ColorToolItem*>& {
    return this->toolbarColorItems;
}

void ToolMenuHandler::disableAudioPlaybackButtons() {
    setAudioPlaybackPaused(false);

    auto* actionDB = this->control->getActionDatabase();
    actionDB->enableAction(Action::AUDIO_PAUSE_PLAYBACK, false);
    actionDB->enableAction(Action::AUDIO_STOP_PLAYBACK, false);
    actionDB->enableAction(Action::AUDIO_SEEK_FORWARDS, false);
    actionDB->enableAction(Action::AUDIO_SEEK_BACKWARDS, false);
}

void ToolMenuHandler::enableAudioPlaybackButtons() {
    auto* actionDB = this->control->getActionDatabase();
    actionDB->enableAction(Action::AUDIO_PAUSE_PLAYBACK, true);
    actionDB->enableAction(Action::AUDIO_STOP_PLAYBACK, true);
    actionDB->enableAction(Action::AUDIO_SEEK_FORWARDS, true);
    actionDB->enableAction(Action::AUDIO_SEEK_BACKWARDS, true);
}

void ToolMenuHandler::setAudioPlaybackPaused(bool paused) {
    this->control->getActionDatabase()->setActionState(Action::AUDIO_PAUSE_PLAYBACK, paused);
}

auto ToolMenuHandler::iconName(const char* icon) -> std::string { return iconNameHelper.iconName(icon); }

void ToolMenuHandler::setDefaultNewPageType(const std::optional<PageType>& pt) {
    this->pageTypeSelectionPopup->setSelected(pt);
}
