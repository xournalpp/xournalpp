#include "ToolMenuHandler.h"

#include <algorithm>  // for max
#include <sstream>    // for istringstream

#include "control/Control.h"                         // for Control
#include "control/PageBackgroundChangeController.h"  // for PageBackgroundChangeController
#include "control/ScrollHandler.h"           // for ScrollHandler
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
#include "gui/widgets/ToolbarBox.h"
#include "plugin/Plugin.h"          // for ToolbarButtonEntr<
#include "util/GVariantTemplate.h"  // for gVariantType
#include "util/GtkUtil.h"
#include "util/NamedColor.h"  // for NamedColor
#include "util/PathUtil.h"
#include "util/StringUtils.h"  // for StringUtils
#include "util/XojMsgBox.h"
#include "util/i18n.h"  // for _

#include "AbstractToolItem.h"            // for AbstractToolItem
#include "ColorSelectorToolItem.h"       // for ColorSelectorToolItem
#include "ColorToolItem.h"               // for ColorToolItem
#include "DrawingTypeComboToolButton.h"  // for DrawingTypeComboToolButton
#include "FontButton.h"                  // for FontButton
#include "PluginToolButton.h"            // for PluginToolButton
#include "StylePopoverFactory.h"         // for ToolButtonWithStylePopover
#include "ToolButton.h"                  // for ToolButton
#include "ToolPageLayer.h"               // for ToolPageLayer
#include "ToolPageSpinner.h"             // for ToolPageSpinner
#include "ToolPdfCombocontrol.h"         // for ToolPdfCombocontrol
#include "ToolSelectCombocontrol.h"      // for ToolSelectComboc...
#include "ToolZoomSlider.h"              // for ToolZoomSlider
#include "TooltipToolButton.h"           // for TooltipToolButton
#include "config-dev.h"                  // for TOOLBAR_CONFIG
#include "config-features.h"             // for ENABLE_PLUGINS
#include "filesystem.h"                  // for exists


using std::string;

ToolMenuHandler::ToolMenuHandler(Control* control, GladeGui* gui):
        parent(GTK_WINDOW(gui->getWindow())),
        control(control),
        zoom(control->getZoomControl()),
        gui(gui),
        toolHandler(control->getToolHandler()),
        tbModel(std::make_unique<ToolbarModel>()),
        pageBackgroundChangeController(control->getPageBackgroundChangeController()),
        iconNameHelper(control->getSettings()),
        pageTypeSelectionPopup(std::make_unique<PageTypeSelectionPopover>(control, control->getSettings(),
                                                                          GTK_APPLICATION_WINDOW(parent))) {}

void ToolMenuHandler::populate(const GladeSearchpath* gladeSearchPath) {
    initToolItems();

    auto file = gladeSearchPath->findFile("", "toolbar.ini");
    if (!tbModel->parse(file, true, this->control->getPalette())) {
        std::string msg = FS(_F("Could not parse general toolbar.ini file: {1}\n"
                                "No Toolbars will be available") %
                             file.u8string());
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
    }

    file = Util::getConfigFile(TOOLBAR_CONFIG);
    if (fs::exists(file)) {
        if (!tbModel->parse(file, false, this->control->getPalette())) {
            string msg = FS(_F("Could not parse custom toolbar.ini file: {1}\n"
                               "Toolbars will not be available") %
                            file.u8string());
            XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
        }
    }
}

ToolMenuHandler::~ToolMenuHandler() = default;

void ToolMenuHandler::freeDynamicToolbarItems() { this->toolbarColorItems.clear(); }

void ToolMenuHandler::unloadToolbar(ToolbarBox* toolbar) {
    toolbar->clear();
    gtk_widget_hide(toolbar->getWidget());
}

void ToolMenuHandler::load(const ToolbarData* d, ToolbarBox* toolbar, const char* toolbarName) {
    int count = 0;
    const auto palette = this->control->getPalette();
    ToolbarSide side = toolbar->getSide();

    const auto& recolorParams = control->getSettings()->getRecolorParameters();
    auto recolor = recolorParams.recolorizeMainView ? std::make_optional(recolorParams.recolor) : std::nullopt;

    for (const ToolbarEntry& e: d->contents) {
        if (e.getName() == toolbarName) {
            for (const ToolbarItem& dataItem: e.getItems()) {
                std::string name = dataItem.getName();

                if (!this->control->getAudioController() &&
                    (name == "AUDIO_RECORDING" || name == "AUDIO_SEEK_BACKWARDS" || name == "AUDIO_PAUSE_PLAYBACK" ||
                     name == "AUDIO_STOP_PLAYBACK" || name == "AUDIO_SEEK_FORWARDS" || name == "PLAY_OBJECT")) {
                    continue;
                }

                if (name == "SEPARATOR") {
                    auto* it = gtk_separator_new(to_Orientation(side) == GTK_ORIENTATION_HORIZONTAL ?
                                                         GTK_ORIENTATION_VERTICAL :
                                                         GTK_ORIENTATION_HORIZONTAL);
                    auto* proxy = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
                    gtk_widget_add_css_class(proxy, "model");
                    toolbar->append(it, proxy);

                    ToolitemDragDrop::attachMetadata(it, dataItem.getId(), TOOL_ITEM_SEPARATOR);

                    continue;
                }

                if (name == "SPACER") {
                    GtkWidget* it;
                    if (to_Orientation(side) == GTK_ORIENTATION_HORIZONTAL) {
                        it = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
                        gtk_widget_set_hexpand(it, true);
                    } else {
                        it = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
                        gtk_widget_set_vexpand(it, true);
                    }
                    gtk_widget_add_css_class(it, "spacer");
                    toolbar->append(it, nullptr);

                    ToolitemDragDrop::attachMetadata(it, dataItem.getId(), TOOL_ITEM_SPACER);

                    continue;
                }
                if (StringUtils::startsWith(name, "COLOR(") && StringUtils::endsWith(name, ")")) {
                    std::string arg = name.substr(6, name.length() - 7);

                    size_t paletteIndex{};
                    std::istringstream colorStream(arg);
                    colorStream >> paletteIndex;
                    if (!colorStream.eof() || colorStream.fail()) {
                        g_warning("Toolbar:COLOR(N) has wrong format: %s", arg.c_str());
                        continue;
                    }

                    count++;
                    const NamedColor& namedColor = palette.getColorAt(paletteIndex);
                    auto& item =
                            this->toolbarColorItems.emplace_back(std::make_unique<ColorToolItem>(namedColor, recolor));

                    auto it = item->createItem(side);
                    toolbar->append(it.item.get(), it.proxy.get());

                    ToolitemDragDrop::attachMetadataColor(it.item.get(), dataItem.getId(), paletteIndex, item.get());
                    continue;
                }

                bool found = false;
                for (auto& item: this->toolItems) {
                    if (name == item->getId()) {
                        count++;
                        auto it = item->createItem(side);
                        toolbar->append(it.item.get(), it.proxy.get());

                        ToolitemDragDrop::attachMetadata(it.item.get(), dataItem.getId(), item.get());

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

    gtk_widget_set_visible(toolbar->getWidget(), count != 0);
}

void ToolMenuHandler::removeColorToolItem(AbstractToolItem* it) {
    g_return_if_fail(it != nullptr);
    this->toolbarColorItems.erase(std::find_if(this->toolbarColorItems.begin(), this->toolbarColorItems.end(),
                                               [it](const auto& p) { return p.get() == it; }));
}

void ToolMenuHandler::addColorToolItem(std::unique_ptr<ColorToolItem> it) {
    g_return_if_fail(it != nullptr);
    this->toolbarColorItems.emplace_back(std::move(it));
}

template <class tool_item, class... Args>
tool_item& ToolMenuHandler::emplaceItem(Args&&... args) {
    return static_cast<tool_item&>(*toolItems.emplace_back(std::make_unique<tool_item>(std::forward<Args>(args)...)));
}

#ifdef ENABLE_PLUGINS
void ToolMenuHandler::addPluginItem(ToolbarButtonEntry* t) { emplaceItem<PluginToolButton>(t); }
#endif /* ENABLE_PLUGINS */

void ToolMenuHandler::initToolItems() {
    using Cat = AbstractToolItem::Category;
    /**
     * @brief Simple button, with a GTK stock icon name
     *      The corresponding action in ActionDatabase[action] should have no state (it can have a parameter)
     **/
    auto emplaceStockItem = [this](const char* name, Cat c, Action action, const char* icon, std::string description) {
        emplaceItem<ToolButton>(name, c, action, icon, description, false);
    };
    /**
     * @brief Simple button, with a custom loaded icon
     *      The corresponding action in ActionDatabase[action] should have no state (it can have a parameter)
     **/
    auto emplaceCustomItem = [this](const char* name, Cat c, Action action, const char* icon, std::string description) {
        emplaceItem<ToolButton>(name, c, action, iconName(icon), description, false);
    };

    /**
     * @brief Toggle button, with a GTK stock icon name
     *      The corresponding action in ActionDatabase[action] should have a boolean state and no parameter
     **/
    auto emplaceStockItemTgl = [this](const char* name, Cat c, Action action, const char* icon,
                                      std::string description) {
        emplaceItem<ToolButton>(name, c, action, icon, description, true);
    };

    /**
     * @brief Toggle button, with a custom loaded icon
     *      The corresponding action in ActionDatabase[action] should have a boolean state and no parameter
     **/
    auto emplaceCustomItemTgl = [this](const char* name, Cat c, Action action, const char* icon,
                                       std::string description) {
        emplaceItem<ToolButton>(name, c, action, iconName(icon), description, true);
    };

    /**
     * @brief Toggle button linked to others sharing the same action (with a custom loaded icon)
     *      The corresponding action in ActionDatabase[action] should have a state and a parameter. The button is "on"
     *when the action state matches `target`.
     **/
    auto emplaceCustomItemWithTarget = [this](const char* name, Cat c, Action action, auto target, const char* icon,
                                              std::string description) {
        emplaceItem<ToolButton>(name, c, action, makeGVariant(target), iconName(icon), description);
    };

    /**
     * @brief Simple button (with a custom loaded icon), with a popover menu to change parameters of the tool
     *      The corresponding action in ActionDatabase[action] should have no state (it can have a parameter)
     **/
    auto emplaceCustomItemWithPopover = [this](const char* name, Cat c, Action action, const char* icon,
                                               std::string description, const PopoverFactory* popover) {
        auto&& tb = emplaceItem<ToolButton>(name, c, action, iconName(icon), description, false);
        tb.setPopoverFactory(popover);
    };

    auto emplaceCustomItemWithTargetAndMenu = [this](const char* name, Cat c, Action action, auto target,
                                                     const char* icon, std::string description,
                                                     const PopoverFactory* popover) {
        auto&& tb = emplaceItem<ToolButton>(name, c, action, makeGVariant(target), iconName(icon), description);
        tb.setPopoverFactory(popover);
    };


    /*
     * Items ordered by menu, if possible.
     * There are some entries which are not available in the menu, like the Zoom slider
     * All menu items without tool icon are not listed here - they are connected by ActionDatabase
     */

    /*
     * Menu File
     * ------------------------------------------------------------------------
     */

    emplaceCustomItem("NEW", Cat::FILES, Action::NEW_FILE, "document-new", _("New Xournal"));
    emplaceCustomItem("OPEN", Cat::FILES, Action::OPEN, "document-open", _("Open file"));
    emplaceCustomItem("SAVE", Cat::FILES, Action::SAVE, "document-save", _("Save"));
    emplaceCustomItem("SAVEPDF", Cat::FILES, Action::EXPORT_AS_PDF, "document-export-pdf", _("Export as PDF"));
    emplaceCustomItem("PRINT", Cat::FILES, Action::PRINT, "document-print", _("Print"));

    /*
     * Menu Edit
     * ------------------------------------------------------------------------
     */

    // Undo / Redo Texts are updated
    emplaceItem<TooltipToolButton>(
            "UNDO", Cat::MISC, Action::UNDO, iconName("edit-undo"), _("Undo"),
            [undoredo = control->getUndoRedoHandler()]() { return undoredo->undoDescription(); });
    emplaceItem<TooltipToolButton>(
            "REDO", Cat::MISC, Action::REDO, iconName("edit-redo"), _("Redo"),
            [undoredo = control->getUndoRedoHandler()]() { return undoredo->redoDescription(); });

    emplaceCustomItem("CUT", Cat::MISC, Action::CUT, "edit-cut", _("Cut"));
    emplaceCustomItem("COPY", Cat::MISC, Action::COPY, "edit-copy", _("Copy"));
    emplaceCustomItem("PASTE", Cat::MISC, Action::PASTE, "edit-paste", _("Paste"));

    emplaceStockItem("SEARCH", Cat::MISC, Action::SEARCH, "edit-find", _("Search"));

    emplaceStockItem("DELETE", Cat::MISC, Action::DELETE, "edit-delete", _("Delete"));

    emplaceCustomItemTgl("ROTATION_SNAPPING", Cat::MISC, Action::ROTATION_SNAPPING, "snapping-rotation",
                         _("Rotation Snapping"));
    emplaceCustomItemTgl("GRID_SNAPPING", Cat::MISC, Action::GRID_SNAPPING, "snapping-grid", _("Grid Snapping"));

    /*
     * Menu View
     * ------------------------------------------------------------------------
     */

    emplaceCustomItemTgl("PAIRED_PAGES", Cat::NAVIGATION, Action::PAIRED_PAGES_MODE, "show-paired-pages",
                         _("Paired pages"));
    emplaceCustomItemTgl("PRESENTATION_MODE", Cat::NAVIGATION, Action::PRESENTATION_MODE, "presentation-mode",
                         _("Presentation mode"));
    emplaceCustomItemTgl("FULLSCREEN", Cat::NAVIGATION, Action::FULLSCREEN, "fullscreen", _("Toggle fullscreen"));
    emplaceCustomItemTgl("SHOW_SIDEBAR", Cat::NAVIGATION, Action::SHOW_SIDEBAR, "sidebar-show", _("Toggle sidebar"));

    emplaceCustomItem("MANAGE_TOOLBAR", Cat::MISC, Action::MANAGE_TOOLBAR, "toolbars-manage", _("Manage Toolbars"));
    emplaceCustomItem("CUSTOMIZE_TOOLBAR", Cat::MISC, Action::CUSTOMIZE_TOOLBAR, "toolbars-customize",
                      _("Customize Toolbars"));

    emplaceStockItem("ZOOM_OUT", Cat::NAVIGATION, Action::ZOOM_OUT, "zoom-out", _("Zoom out"));
    emplaceStockItem("ZOOM_IN", Cat::NAVIGATION, Action::ZOOM_IN, "zoom-in", _("Zoom in"));
    emplaceStockItemTgl("ZOOM_FIT", Cat::NAVIGATION, Action::ZOOM_FIT, "zoom-fit-best", _("Zoom fit to screen"));
    emplaceStockItem("ZOOM_100", Cat::NAVIGATION, Action::ZOOM_100, "zoom-original", _("Zoom to 100%"));

    /*
     * Menu Navigation
     * ------------------------------------------------------------------------
     */

    emplaceStockItem("GOTO_FIRST", Cat::NAVIGATION, Action::GOTO_FIRST, "go-first", _("Go to first page"));
    emplaceStockItem("GOTO_BACK", Cat::NAVIGATION, Action::GOTO_PREVIOUS, "go-previous", _("Back"));
    emplaceCustomItem("GOTO_PAGE", Cat::NAVIGATION, Action::GOTO_PAGE, "go-to", _("Go to page"));
    emplaceStockItem("GOTO_NEXT", Cat::NAVIGATION, Action::GOTO_NEXT, "go-next", _("Next"));
    emplaceStockItem("GOTO_LAST", Cat::NAVIGATION, Action::GOTO_LAST, "go-last", _("Go to last page"));

    emplaceStockItem("GOTO_PREVIOUS_LAYER", Cat::NAVIGATION, Action::LAYER_GOTO_PREVIOUS, "go-previous",
                     _("Go to previous layer"));
    emplaceStockItem("GOTO_NEXT_LAYER", Cat::NAVIGATION, Action::LAYER_GOTO_NEXT, "go-next", _("Go to next layer"));
    emplaceStockItem("GOTO_TOP_LAYER", Cat::NAVIGATION, Action::LAYER_GOTO_TOP, "go-top", _("Go to top layer"));

    emplaceCustomItem("GOTO_NEXT_ANNOTATED_PAGE", Cat::NAVIGATION, Action::GOTO_NEXT_ANNOTATED_PAGE,
                      "page-annotated-next", _("Next annotated page"));

    /* Menu Journal
     * ------------------------------------------------------------------------
     */

    emplaceCustomItemWithPopover("INSERT_NEW_PAGE", Cat::TOOLS, Action::NEW_PAGE_AFTER, "page-add", _("Insert page"),
                                 this->pageTypeSelectionPopup.get());
    emplaceCustomItem("DELETE_CURRENT_PAGE", Cat::TOOLS, Action::DELETE_PAGE, "page-delete", _("Delete current page"));

    /*
     * Menu Tool
     * ------------------------------------------------------------------------
     */
    this->penLineStylePopover = std::make_unique<StylePopoverFactory>(
            Action::TOOL_PEN_LINE_STYLE,
            std::vector<StylePopoverFactory::Entry>{{_("standard"), iconName("line-style-plain"), "plain"},
                                                    {_("dashed"), iconName("line-style-dash"), "dash"},
                                                    {_("dash-/ dotted"), iconName("line-style-dash-dot"), "dashdot"},
                                                    {_("dotted"), iconName("line-style-dot"), "dot"}});
    emplaceCustomItemWithTargetAndMenu("PEN", Cat::TOOLS, Action::SELECT_TOOL, TOOL_PEN, "tool-pencil", _("Pen"),
                                       this->penLineStylePopover.get());

    this->eraserTypePopover = std::make_unique<StylePopoverFactory>(
            Action::TOOL_ERASER_TYPE,
            std::vector<StylePopoverFactory::Entry>{{_("standard"), ERASER_TYPE_DEFAULT},
                                                    {_("whiteout"), ERASER_TYPE_WHITEOUT},
                                                    {_("delete stroke"), ERASER_TYPE_DELETE_STROKE}});
    emplaceCustomItemWithTargetAndMenu("ERASER", Cat::TOOLS, Action::SELECT_TOOL, TOOL_ERASER, "tool-eraser",
                                       _("Eraser"), this->eraserTypePopover.get());

    // Add individual line styles as toolbar items
    emplaceCustomItemWithTarget("PLAIN", Cat::TOOLS, Action::TOOL_PEN_LINE_STYLE, "plain", "line-style-plain-with-pen",
                                _("standard"));
    emplaceCustomItemWithTarget("DASHED", Cat::TOOLS, Action::TOOL_PEN_LINE_STYLE, "dash", "line-style-dash-with-pen",
                                _("dashed"));
    emplaceCustomItemWithTarget("DASH-/ DOTTED", Cat::TOOLS, Action::TOOL_PEN_LINE_STYLE, "dashdot",
                                "line-style-dash-dot-with-pen", _("dash-/ dotted"));
    emplaceCustomItemWithTarget("DOTTED", Cat::TOOLS, Action::TOOL_PEN_LINE_STYLE, "dot", "line-style-dot-with-pen",
                                _("dotted"));


    emplaceCustomItemWithTarget("HIGHLIGHTER", Cat::TOOLS, Action::SELECT_TOOL, TOOL_HIGHLIGHTER, "tool-highlighter",
                                _("Highlighter"));

    emplaceCustomItemWithTarget("TEXT", Cat::TOOLS, Action::SELECT_TOOL, TOOL_TEXT, "tool-text", _("Text"));
    emplaceCustomItem("MATH_TEX", Cat::TOOLS, Action::TEX, "tool-math-tex", _("Add/Edit TeX"));
    emplaceCustomItemWithTarget("IMAGE", Cat::TOOLS, Action::SELECT_TOOL, TOOL_IMAGE, "tool-image", _("Image"));
    emplaceCustomItem("DEFAULT_TOOL", Cat::TOOLS, Action::SELECT_DEFAULT_TOOL, "default", _("Default Tool"));
    emplaceCustomItemWithTarget("SELECT_PDF_TEXT_LINEAR", Cat::SELECTION, Action::SELECT_TOOL,
                                TOOL_SELECT_PDF_TEXT_LINEAR, "select-pdf-text-ht", _("Select Linear PDF Text"));
    emplaceCustomItemWithTarget("SELECT_PDF_TEXT_RECT", Cat::SELECTION, Action::SELECT_TOOL, TOOL_SELECT_PDF_TEXT_RECT,
                                "select-pdf-text-area", _("Select PDF Text in Rectangle"));

    emplaceCustomItemTgl("SETSQUARE", Cat::MISC, Action::SETSQUARE, "setsquare", _("Setsquare"));
    emplaceCustomItemTgl("COMPASS", Cat::MISC, Action::COMPASS, "compass", _("Compass"));


    emplaceCustomItemTgl("SHAPE_RECOGNIZER", Cat::TOOLS, Action::TOOL_DRAW_SHAPE_RECOGNIZER, "shape-recognizer",
                         _("Shape Recognizer"));
    emplaceCustomItemTgl("DRAW_RECTANGLE", Cat::TOOLS, Action::TOOL_DRAW_RECTANGLE, "draw-rect", _("Draw Rectangle"));
    emplaceCustomItemTgl("DRAW_ELLIPSE", Cat::TOOLS, Action::TOOL_DRAW_ELLIPSE, "draw-ellipse", _("Draw Ellipse"));
    emplaceCustomItemTgl("DRAW_ARROW", Cat::TOOLS, Action::TOOL_DRAW_ARROW, "draw-arrow", _("Draw Arrow"));
    emplaceCustomItemTgl("DRAW_DOUBLE_ARROW", Cat::TOOLS, Action::TOOL_DRAW_DOUBLE_ARROW, "draw-double-arrow",
                         _("Draw Double Arrow"));
    emplaceCustomItemTgl("DRAW_COORDINATE_SYSTEM", Cat::TOOLS, Action::TOOL_DRAW_COORDINATE_SYSTEM,
                         "draw-coordinate-system", _("Draw Coordinate System"));
    emplaceCustomItemTgl("RULER", Cat::TOOLS, Action::TOOL_DRAW_LINE, "draw-line", _("Draw Line"));
    emplaceCustomItemTgl("DRAW_SPLINE", Cat::TOOLS, Action::TOOL_DRAW_SPLINE, "draw-spline", _("Draw Spline"));

    emplaceCustomItemWithTarget("SELECT_REGION", Cat::SELECTION, Action::SELECT_TOOL, TOOL_SELECT_REGION,
                                "select-lasso", _("Select Region"));
    emplaceCustomItemWithTarget("SELECT_RECTANGLE", Cat::SELECTION, Action::SELECT_TOOL, TOOL_SELECT_RECT,
                                "select-rect", _("Select Rectangle"));
    emplaceCustomItemWithTarget("SELECT_MULTILAYER_REGION", Cat::SELECTION, Action::SELECT_TOOL,
                                TOOL_SELECT_MULTILAYER_REGION, "select-multilayer-lasso",
                                _("Select Multi-Layer Region"));
    emplaceCustomItemWithTarget("SELECT_MULTILAYER_RECTANGLE", Cat::SELECTION, Action::SELECT_TOOL,
                                TOOL_SELECT_MULTILAYER_RECT, "select-multilayer-rect",
                                _("Select Multi-Layer Rectangle"));
    emplaceCustomItemWithTarget("SELECT_OBJECT", Cat::SELECTION, Action::SELECT_TOOL, TOOL_SELECT_OBJECT,
                                "object-select", _("Select Object"));
    emplaceCustomItemWithTarget("VERTICAL_SPACE", Cat::SELECTION, Action::SELECT_TOOL, TOOL_VERTICAL_SPACE, "spacer",
                                _("Vertical Space"));
    emplaceCustomItemWithTarget("PLAY_OBJECT", Cat::SELECTION, Action::SELECT_TOOL, TOOL_PLAY_OBJECT, "object-play",
                                _("Play Object"));
    emplaceCustomItemWithTarget("HAND", Cat::SELECTION, Action::SELECT_TOOL, TOOL_HAND, "hand", _("Hand"));

    emplaceItem<FontButton>("SELECT_FONT", *control->getActionDatabase());

    emplaceCustomItemTgl("AUDIO_RECORDING", Cat::AUDIO, Action::AUDIO_RECORD, "audio-record",
                         _("Record Audio / Stop Recording"));
    emplaceCustomItemTgl("AUDIO_PAUSE_PLAYBACK", Cat::AUDIO, Action::AUDIO_PAUSE_PLAYBACK, "audio-playback-pause",
                         _("Pause / Play"));
    emplaceCustomItem("AUDIO_STOP_PLAYBACK", Cat::AUDIO, Action::AUDIO_STOP_PLAYBACK, "audio-playback-stop", _("Stop"));
    emplaceCustomItem("AUDIO_SEEK_FORWARDS", Cat::AUDIO, Action::AUDIO_SEEK_FORWARDS, "audio-seek-forwards",
                      _("Forward"));
    emplaceCustomItem("AUDIO_SEEK_BACKWARDS", Cat::AUDIO, Action::AUDIO_SEEK_BACKWARDS, "audio-seek-backwards",
                      _("Back"));


    ///////////////////////////////////////////////////////////////////////////


    /*
     * Footer tools
     * ------------------------------------------------------------------------
     */
    toolPageSpinner = &emplaceItem<ToolPageSpinner>("PAGE_SPIN", iconNameHelper, control->getScrollHandler());

    emplaceItem<ToolZoomSlider>("ZOOM_SLIDER", zoom, iconNameHelper, *control->getActionDatabase());

    emplaceItem<ToolPageLayer>("LAYER", control->getLayerController(), iconNameHelper);

    /*
     * Non-menu items
     * ------------------------------------------------------------------------
     */

    /*
     * Color item - not in the menu
     * aka. COLOR_SELECT
     */
    emplaceItem<ColorSelectorToolItem>(*control->getActionDatabase());

    bool hideAudio = !this->control->getAudioController();
    emplaceItem<ToolSelectCombocontrol>("SELECT", this->iconNameHelper, *this->control->getActionDatabase(), hideAudio);
    emplaceItem<DrawingTypeComboToolButton>("DRAW", this->iconNameHelper, *this->control->getActionDatabase());
    emplaceItem<ToolPdfCombocontrol>("PDF_TOOL", this->iconNameHelper, *this->control->getActionDatabase());

    // General tool configuration - working for every tool which support it
    emplaceCustomItemTgl("TOOL_FILL", Cat::TOOLS, Action::TOOL_FILL, "fill", _("Fill"));
    emplaceCustomItem("FILL_OPACITY", Cat::TOOLS, Action::TOOL_FILL_OPACITY, "fill-opacity", _("Fill Opacity"));

    emplaceCustomItemWithTarget("VERY_FINE", Cat::TOOLS, Action::TOOL_SIZE, TOOL_SIZE_VERY_FINE, "thickness-finer",
                                _("Very Fine"));
    emplaceCustomItemWithTarget("FINE", Cat::TOOLS, Action::TOOL_SIZE, TOOL_SIZE_FINE, "thickness-fine", _("Fine"));
    emplaceCustomItemWithTarget("MEDIUM", Cat::TOOLS, Action::TOOL_SIZE, TOOL_SIZE_MEDIUM, "thickness-medium",
                                _("Medium"));
    emplaceCustomItemWithTarget("THICK", Cat::TOOLS, Action::TOOL_SIZE, TOOL_SIZE_THICK, "thickness-thick", _("Thick"));
    emplaceCustomItemWithTarget("VERY_THICK", Cat::TOOLS, Action::TOOL_SIZE, TOOL_SIZE_VERY_THICK, "thickness-thicker",
                                _("Very Thick"));
}

void ToolMenuHandler::setPageInfo(size_t currentPage, size_t pageCount, size_t pdfpage) {
    if (this->toolPageSpinner) {
        this->toolPageSpinner->setPageInfo(currentPage, pageCount, pdfpage);
    }
}

auto ToolMenuHandler::getModel() -> ToolbarModel* { return this->tbModel.get(); }

auto ToolMenuHandler::getControl() -> Control* { return this->control; }

auto ToolMenuHandler::getToolItems() const -> const std::vector<std::unique_ptr<AbstractToolItem>>& {
    return this->toolItems;
}

auto ToolMenuHandler::getColorToolItems() const -> const std::vector<std::unique_ptr<ColorToolItem>>& {
    return this->toolbarColorItems;
}

auto ToolMenuHandler::iconName(const char* icon) -> std::string { return iconNameHelper.iconName(icon); }

void ToolMenuHandler::updateColorToolItems(const Palette& palette) {
    for (const auto& it: this->toolbarColorItems) {
        it->updateColor(palette);
    }
}

void ToolMenuHandler::updateColorToolItemsRecoloring(const std::optional<Recolor>& recolor) {
    for (const auto& it: this->toolbarColorItems) {
        it->updateSecondaryColor(recolor);
    }
}

void ToolMenuHandler::setDefaultNewPageType(const std::optional<PageType>& pt) {
    this->pageTypeSelectionPopup->setSelectedPT(pt);
    this->control->getPageBackgroundChangeController()->setPageTypeForNewPages(pt);
}
void ToolMenuHandler::setDefaultNewPaperSize(const std::optional<PaperSize>& paperSize) {
    this->pageTypeSelectionPopup->setSelectedPaperSize(paperSize);
}