#include "ShortcutConfiguration.h"

#include <algorithm>

#include "control/Control.h"
#include "control/ScrollHandler.h"
#include "control/ToolEnums.h"
#include "control/actions/ActionDatabase.h"
#include "gui/MainWindow.h"
#include "gui/toolbarMenubar/ColorToolItem.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"

static auto defaultActionShortcuts() -> std::unordered_map<ActionKey, std::vector<Shortcut>>;
static auto defaultScrollShortcuts() -> KeyBindingsGroup<ScrollHandler>;
static auto defaultOtherShortcuts() -> KeyBindingsGroup<Control>;

static auto computeShunt(const std::unordered_map<ActionKey, std::vector<Shortcut>>& actionsShortcuts)
        -> KeyBindingsGroup<ActionDatabase>;

ShortcutConfiguration::ShortcutConfiguration():
        actionsShortcuts(defaultActionShortcuts()),
        scrollShortcuts(defaultScrollShortcuts()),
        otherShortcuts(defaultOtherShortcuts()),
        shuntGtkDefaultBindings(computeShunt(actionsShortcuts)) {}

static auto defaultActionShortcuts() -> std::unordered_map<ActionKey, std::vector<Shortcut>> {
    return std::unordered_map<ActionKey, std::vector<Shortcut>>({
            // clang-format off
        {Action::QUIT,                                       {{CTRL_OR_META, GDK_KEY_q}}},  // Name: Quit
        {Action::NEW_FILE,                                   {{CTRL_OR_META, GDK_KEY_n}}},  // Name: New
        {Action::OPEN,                                       {{CTRL_OR_META, GDK_KEY_o}}},  // Name: Open
        {Action::SAVE,                                       {{CTRL_OR_META, GDK_KEY_s}}},  // Name: Save
        {Action::SAVE_AS,                                    {{CTRL_OR_META & SHIFT, GDK_KEY_s}}},  // Name: Save As
        {Action::EXPORT_AS,                                  {{CTRL_OR_META, GDK_KEY_e}}},  // Name: Export as...
        {Action::PRINT,                                      {{CTRL_OR_META, GDK_KEY_p}}},  // Name Print
        {Action::CUT,                                        {{CTRL_OR_META, GDK_KEY_x},
                                                              {NONE, GDK_KEY_Cut}}},  // Name Cut
        {Action::COPY,                                       {{CTRL_OR_META, GDK_KEY_c},
                                                              {NONE, GDK_KEY_Copy}}},  // Name Copy
        {Action::PASTE,                                      {{CTRL_OR_META, GDK_KEY_v},
                                                              {NONE, GDK_KEY_Paste}}},  // Name Paste
        {Action::SELECT_ALL,                                 {{CTRL_OR_META, GDK_KEY_a}}},  // Name Select All
        {Action::SEARCH,                                     {{CTRL_OR_META, GDK_KEY_f}}},  // Name Find
        {Action::DELETE,                                     {{NONE, GDK_KEY_Delete}}},  // Name Delete
        {Action::PRESENTATION_MODE,                          {{NONE, GDK_KEY_F5}}},  // Name _Presentation Mode
        {Action::FULLSCREEN,                                 {{NONE, GDK_KEY_F11}}},  // Name Fullscreen
        {Action::SHOW_TOOLBAR,                               {{NONE, GDK_KEY_F9}}},  // Name Show Toolbars
        {Action::SHOW_MENUBAR,                               {{NONE, GDK_KEY_F10}}},  // Name Show Menubar
        {Action::SHOW_SIDEBAR,                               {{NONE, GDK_KEY_F12}}},  // Name Show Sidebar
        {Action::LAYER_GOTO_PREVIOUS,                        {{SHIFT, GDK_KEY_Page_Down}}},  // Name _Previous Layer
        {Action::LAYER_GOTO_NEXT,                            {{SHIFT, GDK_KEY_Page_Up}}},  // Name _Next Layer
        {Action::NEW_PAGE_AFTER,                             {{CTRL_OR_META, GDK_KEY_d}}},  // Name New Page _After
        {Action::DELETE_PAGE,                                {{CTRL_OR_META & SHIFT, GDK_KEY_Delete}}},  // Name _Delete Page
        {Action::LAYER_NEW,                                  {{CTRL_OR_META, GDK_KEY_l}}},  // Name New _Layer
        {Action::LAYER_DELETE,                               {{CTRL_OR_META & SHIFT, GDK_KEY_l}}},  // Name Delete Layer
        {Action::LAYER_MERGE_DOWN,                           {{CTRL_OR_META, GDK_KEY_m}}},  // Name Merge Layer Down
        {Action::LAYER_RENAME,                               {{CTRL_OR_META, GDK_KEY_r}}},  // Name Rename Layer
        {Action::SELECT_DEFAULT_TOOL,                        {{CTRL_OR_META & SHIFT, GDK_KEY_d}}},  // Name _Default Tool
        {Action::TOOL_DRAW_SHAPE_RECOGNIZER,                 {{CTRL_OR_META, GDK_KEY_1}}},  // Name _Shape Recognizer
        {Action::TOOL_DRAW_RECTANGLE,                        {{CTRL_OR_META, GDK_KEY_2}}},  // Name Draw Rectangle
        {Action::TOOL_DRAW_ELLIPSE,                          {{CTRL_OR_META, GDK_KEY_3}}},  // Name Draw Ellipse
        {Action::TOOL_DRAW_ARROW,                            {{CTRL_OR_META, GDK_KEY_4}}},  // Name Draw Arrow
        {Action::TOOL_DRAW_DOUBLE_ARROW,                     {{CTRL_OR_META, GDK_KEY_5}}},  // Name Draw Double Arrow
        {Action::TOOL_DRAW_COORDINATE_SYSTEM,                {{CTRL_OR_META, GDK_KEY_6}}},  // Name Draw Coordinate System
        {Action::TOOL_DRAW_LINE,                             {{CTRL_OR_META, GDK_KEY_7}}},  // Name Draw _Line
        {Action::TOOL_DRAW_SPLINE,                           {{CTRL_OR_META, GDK_KEY_8}}},  // Name Draw Spline
        {Action::SELECT_FONT,                                {{CTRL_OR_META & SHIFT, GDK_KEY_f}}},  // Name Text Font...
        {Action::TEX,                                        {{CTRL_OR_META & SHIFT, GDK_KEY_x}}},  // Name Add/Edit TeX
        {{Action::SELECT_TOOL, TOOL_PEN},                    {{CTRL_OR_META & SHIFT, GDK_KEY_p}}}, //  Name: _Pen
        {{Action::SELECT_TOOL, TOOL_ERASER},                 {{CTRL_OR_META & SHIFT, GDK_KEY_e}}}, //  Name: _Eraser
        {{Action::SELECT_TOOL, TOOL_HIGHLIGHTER},            {{CTRL_OR_META & SHIFT, GDK_KEY_h}}}, //  Name: _Highlighter
        {{Action::SELECT_TOOL, TOOL_TEXT},                   {{CTRL_OR_META & SHIFT, GDK_KEY_t}}}, //  Name: _Text
        {{Action::SELECT_TOOL, TOOL_IMAGE},                  {{CTRL_OR_META & SHIFT, GDK_KEY_i}}}, //  Name: _Image
        {{Action::SELECT_TOOL, TOOL_SELECT_RECT},            {{CTRL_OR_META & SHIFT, GDK_KEY_r}}}, //  Name: Select Rectangle
        {{Action::SELECT_TOOL, TOOL_SELECT_REGION},          {{CTRL_OR_META & SHIFT, GDK_KEY_g}}}, //  Name: Select Region
        {{Action::SELECT_TOOL, TOOL_SELECT_OBJECT},          {{CTRL_OR_META & SHIFT, GDK_KEY_o}}}, //  Name: Select Object
        {{Action::SELECT_TOOL, TOOL_VERTICAL_SPACE},         {{CTRL_OR_META & SHIFT, GDK_KEY_v}}}, //  Name: _Vertical Space
        {{Action::SELECT_TOOL, TOOL_HAND},                   {{CTRL_OR_META & SHIFT, GDK_KEY_a}}}, //  Name: H_and Tool
        {{Action::SELECT_TOOL, TOOL_SELECT_PDF_TEXT_LINEAR}, {{CTRL_OR_META & SHIFT, GDK_KEY_w}}}, //  Name: Select Linear Text
        {{Action::SELECT_TOOL, TOOL_SELECT_PDF_TEXT_RECT},   {{CTRL_OR_META & SHIFT, GDK_KEY_y}}}, //  Name: Select Text In Rectangle
        {Action::UNDO,                                       {{CTRL_OR_META, GDK_KEY_z}}}, //  Name: Undo
        {Action::REDO,                                       {{CTRL_OR_META & SHIFT, GDK_KEY_z},
                                                              {CTRL_OR_META, GDK_KEY_y}}}, //  Name: Redo
        {Action::ZOOM_OUT,                                   {{CTRL_OR_META, GDK_KEY_minus},
                                                              {CTRL_OR_META, GDK_KEY_KP_Subtract}}},  // Name: Zoom out
        {Action::ZOOM_IN,                                    {{CTRL_OR_META, GDK_KEY_plus},
                                                              {CTRL_OR_META, GDK_KEY_KP_Add},
                                                              {CTRL_OR_META, GDK_KEY_equal}}},  // Name: Zoom in

        {Action::GOTO_PAGE,                                  {{CTRL_OR_META, GDK_KEY_g}}},  // Name _Goto Page
        {Action::GOTO_NEXT_ANNOTATED_PAGE,                   {{CTRL_OR_META & SHIFT, GDK_KEY_Page_Down},
                                                              {CTRL_OR_META & SHIFT, GDK_KEY_KP_Page_Down}}},  // Name N_ext Annotated Page
        {Action::GOTO_PREVIOUS_ANNOTATED_PAGE,               {{CTRL_OR_META & SHIFT, GDK_KEY_Page_Up},
                                                              {CTRL_OR_META & SHIFT, GDK_KEY_KP_Page_Up}}},  // Name P_revious Annotated Page
        {Action::GOTO_NEXT,                                  {{CTRL_OR_META, GDK_KEY_Page_Down},
                                                              {CTRL_OR_META, GDK_KEY_KP_Page_Down}}},  // Name _Next Page
        {Action::GOTO_PREVIOUS,                              {{CTRL_OR_META, GDK_KEY_Page_Up},
                                                              {CTRL_OR_META, GDK_KEY_KP_Page_Up}}},  // Name _Previous Page
        {Action::GOTO_LAST,                                  {{NONE, GDK_KEY_End},
                                                              {NONE, GDK_KEY_KP_End}}},  // Name _Last Page
        {Action::GOTO_FIRST,                                 {{NONE, GDK_KEY_Home},
                                                              {NONE, GDK_KEY_KP_Home}}},  // Name _First Pages
            // clang-format on
    });
}

static auto defaultScrollShortcuts() -> KeyBindingsGroup<ScrollHandler> {
    /*
     * TODO
     * Before this PR: GDK_KEY_KP_Up did ScrollHandler::scrollByOnePage and SHIFT + GDK_KEY_KP_Up did nothing
     * and similarly for other directions
     * I changed that to: GDK_KEY_KP_Up does as GDK_KEY_Up and SHIFT + GDK_KEY_KP_Up does ScrollHandler::scrollByOnePage
     *
     * This is up for debate
     */

    using KeyBindingsUtil::wrap;

    // clang-format off
    return KeyBindingsGroup<ScrollHandler>({
        // Nb: other scrolling key bindings are defined above, with Action::GOTO_...
        {{NONE, GDK_KEY_Page_Down},    wrap<&ScrollHandler::scrollByVisibleArea, ScrollHandler::DOWN>},
        {{NONE, GDK_KEY_KP_Page_Down}, wrap<&ScrollHandler::scrollByVisibleArea, ScrollHandler::DOWN>},
        {{NONE, GDK_KEY_Page_Up},      wrap<&ScrollHandler::scrollByVisibleArea, ScrollHandler::UP>},
        {{NONE, GDK_KEY_KP_Page_Up},   wrap<&ScrollHandler::scrollByVisibleArea, ScrollHandler::UP>},

        {{SHIFT, GDK_KEY_KP_Up},       wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::UP>},
        {{SHIFT, GDK_KEY_Up},          wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::UP>},
        {{SHIFT, GDK_KEY_KP_Down},     wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::DOWN>},
        {{SHIFT, GDK_KEY_Down},        wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::DOWN>},
        {{SHIFT, GDK_KEY_KP_Left},     wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::LEFT>},
        {{SHIFT, GDK_KEY_Left},        wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::LEFT>},
        {{SHIFT, GDK_KEY_KP_Right},    wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::RIGHT>},
        {{SHIFT, GDK_KEY_Right},       wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::RIGHT>},

        {{NONE, GDK_KEY_KP_Up},        wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::UP>},
        {{NONE, GDK_KEY_Up},           wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::UP>},
        {{NONE, GDK_KEY_KP_Down},      wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::DOWN>},
        {{NONE, GDK_KEY_Down},         wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::DOWN>},
        {{NONE, GDK_KEY_KP_Left},      wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::LEFT>},
        {{NONE, GDK_KEY_Left},         wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::LEFT>},
        {{NONE, GDK_KEY_KP_Right},     wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::RIGHT>},
        {{NONE, GDK_KEY_Right},        wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::RIGHT>},

        // vim key bindings - kept for now but to be removed once shortcuts are configurable
        {{SHIFT, GDK_KEY_k},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::UP>},
        {{SHIFT, GDK_KEY_K},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::UP>},
        {{SHIFT, GDK_KEY_j},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::DOWN>},
        {{SHIFT, GDK_KEY_J},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::DOWN>},
        {{SHIFT, GDK_KEY_h},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::LEFT>},
        {{SHIFT, GDK_KEY_l},           wrap<&ScrollHandler::scrollByOnePage, ScrollHandler::RIGHT>},
        {{NONE, GDK_KEY_k},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::UP>},
        {{NONE, GDK_KEY_K},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::UP>},
        {{NONE, GDK_KEY_j},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::DOWN>},
        {{NONE, GDK_KEY_J},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::DOWN>},
        {{NONE, GDK_KEY_h},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::LEFT>},
        {{NONE, GDK_KEY_l},            wrap<&ScrollHandler::scrollByOneStep, ScrollHandler::RIGHT>},
    });
    // clang-format on
}


template <size_t n>
static void setColorByNumber(Control* ctrl) {
    if (auto* db = ctrl->getActionDatabase(); db->isActionEnabled(Action::TOOL_COLOR)) {
        const auto& colors = ctrl->getWindow()->getToolMenuHandler()->getColorToolItems();
        if (n < colors.size()) {
            db->fireChangeActionState(Action::TOOL_COLOR, colors[n]->getColor());
        }
    }
}
static auto defaultOtherShortcuts() -> KeyBindingsGroup<Control> {
    return KeyBindingsGroup<Control>({
            {{NONE, GDK_KEY_1}, setColorByNumber<0>},
            {{NONE, GDK_KEY_2}, setColorByNumber<1>},
            {{NONE, GDK_KEY_3}, setColorByNumber<2>},
            {{NONE, GDK_KEY_4}, setColorByNumber<3>},
            {{NONE, GDK_KEY_5}, setColorByNumber<4>},
            {{NONE, GDK_KEY_6}, setColorByNumber<5>},
            {{NONE, GDK_KEY_7}, setColorByNumber<6>},
            {{NONE, GDK_KEY_8}, setColorByNumber<7>},
            {{NONE, GDK_KEY_9}, setColorByNumber<8>},
            {{NONE, GDK_KEY_0}, setColorByNumber<9>},
    });
}

static auto computeShunt(const std::unordered_map<ActionKey, std::vector<Shortcut>>& actionsShortcuts)
        -> KeyBindingsGroup<ActionDatabase> {
    static constexpr PressedModifier CTRL(GDK_CONTROL_MASK);  // GTK does not use META on MacOS for those key bindings
    /// List of key bindings set by GTK in our main widget's ancestors
    static constexpr std::array<Shortcut, 40> gtkBindings = {{
            // Key bindings in GtkScrolledWindow
            {NONE, GDK_KEY_Home},
            {NONE, GDK_KEY_KP_Home},
            {NONE, GDK_KEY_End},
            {NONE, GDK_KEY_KP_End},
            {CTRL, GDK_KEY_Home},
            {CTRL, GDK_KEY_KP_Home},
            {CTRL, GDK_KEY_End},
            {CTRL, GDK_KEY_KP_End},
            {NONE, GDK_KEY_Page_Up},
            {NONE, GDK_KEY_KP_Page_Up},
            {NONE, GDK_KEY_Page_Down},
            {NONE, GDK_KEY_KP_Page_Down},
            {CTRL, GDK_KEY_Page_Up},
            {CTRL, GDK_KEY_KP_Page_Up},
            {CTRL, GDK_KEY_Page_Down},
            {CTRL, GDK_KEY_KP_Page_Down},
            {CTRL, GDK_KEY_Left},
            {CTRL, GDK_KEY_KP_Left},
            {CTRL, GDK_KEY_Right},
            {CTRL, GDK_KEY_KP_Right},
            {CTRL, GDK_KEY_Up},
            {CTRL, GDK_KEY_KP_Up},
            {CTRL, GDK_KEY_Down},
            {CTRL, GDK_KEY_KP_Down},
            {CTRL, GDK_KEY_Tab},
            {CTRL, GDK_KEY_KP_Tab},
            {CTRL & SHIFT, GDK_KEY_Tab},
            {CTRL & SHIFT, GDK_KEY_KP_Tab},

            // Key bindings in GtkPaned not already above
            {NONE, GDK_KEY_Tab},
            {NONE, GDK_KEY_KP_Tab},
            {SHIFT, GDK_KEY_Tab},
            {SHIFT, GDK_KEY_KP_Tab},

            // Key bindings in GtkWindow not already above
            {NONE, GDK_KEY_Left},
            {NONE, GDK_KEY_KP_Left},
            {NONE, GDK_KEY_Right},
            {NONE, GDK_KEY_KP_Right},
            {NONE, GDK_KEY_Up},
            {NONE, GDK_KEY_KP_Up},
            {NONE, GDK_KEY_Down},
            {NONE, GDK_KEY_KP_Down},
            // There are other more specific bindings on space/return or Ctrl+Shift+D and Ctrl+Shift+I in Gtkwindow
            // There are other more specific bindings on escape/space/return or (Shift+)F6/F8 in GtkPaned
            // Include them??
    }};
    KeyBindingsGroup<ActionDatabase>::table_type map;
    auto noop = +[](ActionDatabase*) {};
    for (auto&& s: gtkBindings) {
        for (auto&& [a, accels]: actionsShortcuts) {
            if (std::find(accels.begin(), accels.end(), s) != accels.end()) {
                if (a.parameter) {
                    map.emplace(s, [act = a.action, p = a.parameter.value()](ActionDatabase* db) {
                        db->fireActivateAction(act, p);
                    });
                } else {
                    map.emplace(s, [act = a.action](ActionDatabase* db) { db->fireActivateAction(act); });
                }
                break;
            }
        }
        map.try_emplace(s, noop);  // Catch the unused shortcuts anyway, to suppress GTK's default binding
    }
    return KeyBindingsGroup<ActionDatabase>(std::move(map));
}
