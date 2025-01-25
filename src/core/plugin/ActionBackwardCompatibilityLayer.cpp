#include "ActionBackwardCompatibilityLayer.h"

#ifdef ENABLE_PLUGINS

#include <map>
#include <string_view>

#include "control/Control.h"
#include "control/ToolEnums.h"
#include "control/actions/ActionDatabase.h"
#include "control/settings/Settings.h"
#include "control/tools/EditSelection.h"

using namespace std::literals::string_view_literals;

static const std::map<std::string_view, Action> STATELESS_ACTION_MAP = {
        {"ACTION_NEW"sv, Action::NEW_FILE},
        {"ACTION_OPEN"sv, Action::OPEN},
        {"ACTION_ANNOTATE_PDF"sv, Action::ANNOTATE_PDF},
        {"ACTION_SAVE"sv, Action::SAVE},
        {"ACTION_SAVE_AS"sv, Action::SAVE_AS},
        {"ACTION_EXPORT_AS_PDF"sv, Action::EXPORT_AS_PDF},
        {"ACTION_EXPORT_AS"sv, Action::EXPORT_AS},
        {"ACTION_PRINT"sv, Action::PRINT},
        {"ACTION_QUIT"sv, Action::QUIT},
        {"ACTION_UNDO"sv, Action::UNDO},
        {"ACTION_REDO"sv, Action::REDO},
        {"ACTION_CUT"sv, Action::CUT},
        {"ACTION_COPY"sv, Action::COPY},
        {"ACTION_PASTE"sv, Action::PASTE},
        {"ACTION_SEARCH"sv, Action::SEARCH},
        {"ACTION_SELECT_ALL"sv, Action::SELECT_ALL},
        {"ACTION_DELETE"sv, Action::DELETE},
        {"ACTION_SETTINGS"sv, Action::PREFERENCES},
        {"ACTION_GOTO_FIRST"sv, Action::GOTO_FIRST},
        {"ACTION_GOTO_BACK"sv, Action::GOTO_PREVIOUS},
        {"ACTION_GOTO_PAGE"sv, Action::GOTO_PAGE},
        {"ACTION_GOTO_NEXT"sv, Action::GOTO_NEXT},
        {"ACTION_GOTO_LAST"sv, Action::GOTO_LAST},
        {"ACTION_GOTO_NEXT_ANNOTATED_PAGE"sv, Action::GOTO_NEXT_ANNOTATED_PAGE},
        {"ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE"sv, Action::GOTO_PREVIOUS_ANNOTATED_PAGE},
        {"ACTION_NEW_PAGE_BEFORE"sv, Action::NEW_PAGE_BEFORE},
        {"ACTION_DUPLICATE_PAGE"sv, Action::DUPLICATE_PAGE},
        {"ACTION_NEW_PAGE_AFTER"sv, Action::NEW_PAGE_AFTER},
        {"ACTION_APPEND_NEW_PDF_PAGES"sv, Action::APPEND_NEW_PDF_PAGES},
        {"ACTION_NEW_PAGE_AT_END"sv, Action::NEW_PAGE_AT_END},
        {"ACTION_DELETE_PAGE"sv, Action::DELETE_PAGE},
        {"ACTION_PAPER_FORMAT"sv, Action::PAPER_FORMAT},
        {"ACTION_CONFIGURE_PAGE_TEMPLATE"sv, Action::CONFIGURE_PAGE_TEMPLATE},
        {"ACTION_PAPER_BACKGROUND_COLOR"sv, Action::PAPER_BACKGROUND_COLOR},
        {"ACTION_MOVE_SELECTION_LAYER_UP"sv, Action::MOVE_SELECTION_LAYER_UP},
        {"ACTION_MOVE_SELECTION_LAYER_DOWN"sv, Action::MOVE_SELECTION_LAYER_DOWN},
        {"ACTION_TOOL_PEN_FILL_OPACITY"sv, Action::TOOL_PEN_FILL_OPACITY},
        {"ACTION_TOOL_DEFAULT"sv, Action::SELECT_DEFAULT_TOOL},
        {"ACTION_TOOL_HIGHLIGHTER_FILL_OPACITY"sv, Action::TOOL_HIGHLIGHTER_FILL_OPACITY},
        {"ACTION_ZOOM_IN"sv, Action::ZOOM_IN},
        {"ACTION_ZOOM_OUT"sv, Action::ZOOM_OUT},
        {"ACTION_ZOOM_100"sv, Action::ZOOM_100},
        {"ACTION_TEX"sv, Action::TEX},
        {"ACTION_MANAGE_TOOLBAR"sv, Action::MANAGE_TOOLBAR},
        {"ACTION_CUSTOMIZE_TOOLBAR"sv, Action::CUSTOMIZE_TOOLBAR},
        {"ACTION_AUDIO_SEEK_FORWARDS"sv, Action::AUDIO_SEEK_FORWARDS},
        {"ACTION_AUDIO_SEEK_BACKWARDS"sv, Action::AUDIO_SEEK_BACKWARDS},
        {"ACTION_AUDIO_STOP_PLAYBACK"sv, Action::AUDIO_STOP_PLAYBACK},
        {"ACTION_PLUGIN_MANAGER"sv, Action::PLUGIN_MANAGER},
        {"ACTION_HELP"sv, Action::HELP},
        {"ACTION_ABOUT"sv, Action::ABOUT},
        {"ACTION_NEW_LAYER"sv, Action::LAYER_NEW},
        {"ACTION_DELETE_LAYER"sv, Action::LAYER_DELETE},
        {"ACTION_MERGE_LAYER_DOWN"sv, Action::LAYER_MERGE_DOWN},
        {"ACTION_GOTO_NEXT_LAYER"sv, Action::LAYER_GOTO_NEXT},
        {"ACTION_GOTO_PREVIOUS_LAYER"sv, Action::LAYER_GOTO_PREVIOUS},
        {"ACTION_GOTO_TOP_LAYER"sv, Action::LAYER_GOTO_TOP},
        {"ACTION_RENAME_LAYER"sv, Action::LAYER_RENAME},
        {"ACTION_SELECT_FONT"sv, Action::SELECT_FONT}};

static const std::map<std::string_view, ToolType> SELECT_TOOL_MAP = {
        {"ACTION_TOOL_PEN"sv, TOOL_PEN},
        {"ACTION_TOOL_ERASER"sv, TOOL_ERASER},
        {"ACTION_TOOL_HIGHLIGHTER"sv, TOOL_HIGHLIGHTER},
        {"ACTION_TOOL_TEXT"sv, TOOL_TEXT},
        {"ACTION_TOOL_IMAGE"sv, TOOL_IMAGE},
        {"ACTION_TOOL_SELECT_RECT"sv, TOOL_SELECT_RECT},
        {"ACTION_TOOL_SELECT_REGION"sv, TOOL_SELECT_REGION},
        {"ACTION_TOOL_SELECT_MULTILAYER_RECT"sv, TOOL_SELECT_MULTILAYER_RECT},
        {"ACTION_TOOL_SELECT_MULTILAYER_REGION"sv, TOOL_SELECT_MULTILAYER_REGION},
        {"ACTION_TOOL_SELECT_OBJECT"sv, TOOL_SELECT_OBJECT},
        {"ACTION_TOOL_PLAY_OBJECT"sv, TOOL_PLAY_OBJECT},
        {"ACTION_TOOL_SELECT_PDF_TEXT_LINEAR"sv, TOOL_SELECT_PDF_TEXT_LINEAR},
        {"ACTION_TOOL_SELECT_PDF_TEXT_RECT"sv, TOOL_SELECT_PDF_TEXT_RECT},
        {"ACTION_TOOL_VERTICAL_SPACE"sv, TOOL_VERTICAL_SPACE},
        {"ACTION_TOOL_HAND"sv, TOOL_HAND},
        {"ACTION_TOOL_FLOATING_TOOLBOX"sv, TOOL_FLOATING_TOOLBOX}};

static const std::map<std::string_view, std::pair<Action, ToolSize>> TOOL_SIZE_MAP = {
        {"ACTION_SIZE_VERY_FINE"sv, {Action::TOOL_SIZE, TOOL_SIZE_VERY_FINE}},
        {"ACTION_SIZE_FINE"sv, {Action::TOOL_SIZE, TOOL_SIZE_FINE}},
        {"ACTION_SIZE_MEDIUM"sv, {Action::TOOL_SIZE, TOOL_SIZE_MEDIUM}},
        {"ACTION_SIZE_THICK"sv, {Action::TOOL_SIZE, TOOL_SIZE_THICK}},
        {"ACTION_SIZE_VERY_THICK"sv, {Action::TOOL_SIZE, TOOL_SIZE_VERY_THICK}},
        {"ACTION_TOOL_ERASER_SIZE_VERY_FINE"sv, {Action::TOOL_ERASER_SIZE, TOOL_SIZE_VERY_FINE}},
        {"ACTION_TOOL_ERASER_SIZE_FINE"sv, {Action::TOOL_ERASER_SIZE, TOOL_SIZE_FINE}},
        {"ACTION_TOOL_ERASER_SIZE_MEDIUM"sv, {Action::TOOL_ERASER_SIZE, TOOL_SIZE_MEDIUM}},
        {"ACTION_TOOL_ERASER_SIZE_THICK"sv, {Action::TOOL_ERASER_SIZE, TOOL_SIZE_THICK}},
        {"ACTION_TOOL_ERASER_SIZE_VERY_THICK"sv, {Action::TOOL_ERASER_SIZE, TOOL_SIZE_VERY_THICK}},
        {"ACTION_TOOL_PEN_SIZE_VERY_FINE"sv, {Action::TOOL_PEN_SIZE, TOOL_SIZE_VERY_FINE}},
        {"ACTION_TOOL_PEN_SIZE_FINE"sv, {Action::TOOL_PEN_SIZE, TOOL_SIZE_FINE}},
        {"ACTION_TOOL_PEN_SIZE_MEDIUM"sv, {Action::TOOL_PEN_SIZE, TOOL_SIZE_MEDIUM}},
        {"ACTION_TOOL_PEN_SIZE_THICK"sv, {Action::TOOL_PEN_SIZE, TOOL_SIZE_THICK}},
        {"ACTION_TOOL_PEN_SIZE_VERY_THICK"sv, {Action::TOOL_PEN_SIZE, TOOL_SIZE_VERY_THICK}},
        {"ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_FINE"sv, {Action::TOOL_HIGHLIGHTER_SIZE, TOOL_SIZE_VERY_FINE}},
        {"ACTION_TOOL_HIGHLIGHTER_SIZE_FINE"sv, {Action::TOOL_HIGHLIGHTER_SIZE, TOOL_SIZE_FINE}},
        {"ACTION_TOOL_HIGHLIGHTER_SIZE_MEDIUM"sv, {Action::TOOL_HIGHLIGHTER_SIZE, TOOL_SIZE_MEDIUM}},
        {"ACTION_TOOL_HIGHLIGHTER_SIZE_THICK"sv, {Action::TOOL_HIGHLIGHTER_SIZE, TOOL_SIZE_THICK}},
        {"ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_THICK"sv, {Action::TOOL_HIGHLIGHTER_SIZE, TOOL_SIZE_VERY_THICK}}};

static const std::map<std::string_view, Action> BOOLEAN_ACTION_MAP = {
        {"ACTION_SETSQUARE"sv, Action::SETSQUARE},
        {"ACTION_COMPASS"sv, Action::COMPASS},
        {"ACTION_TOOL_DRAW_RECT"sv, Action::TOOL_DRAW_RECTANGLE},
        {"ACTION_TOOL_DRAW_ELLIPSE"sv, Action::TOOL_DRAW_ELLIPSE},
        {"ACTION_TOOL_DRAW_ARROW"sv, Action::TOOL_DRAW_ARROW},
        {"ACTION_TOOL_DRAW_DOUBLE_ARROW"sv, Action::TOOL_DRAW_DOUBLE_ARROW},
        {"ACTION_TOOL_DRAW_COORDINATE_SYSTEM"sv, Action::TOOL_DRAW_COORDINATE_SYSTEM},
        {"ACTION_RULER"sv, Action::TOOL_DRAW_LINE},
        {"ACTION_TOOL_DRAW_SPLINE"sv, Action::TOOL_DRAW_SPLINE},
        {"ACTION_SHAPE_RECOGNIZER"sv, Action::TOOL_DRAW_SHAPE_RECOGNIZER},
        {"ACTION_TOOL_FILL"sv, Action::TOOL_FILL},
        {"ACTION_TOOL_PEN_FILL"sv, Action::TOOL_PEN_FILL},
        {"ACTION_TOOL_HIGHLIGHTER_FILL"sv, Action::TOOL_HIGHLIGHTER_FILL},
        {"ACTION_ZOOM_FIT"sv, Action::ZOOM_FIT},
        {"ACTION_VIEW_PAIRED_PAGES"sv, Action::PAIRED_PAGES_MODE},
        {"ACTION_VIEW_PRESENTATION_MODE"sv, Action::PRESENTATION_MODE},
        {"ACTION_FULLSCREEN"sv, Action::FULLSCREEN},
        {"ACTION_SHOW_SIDEBAR"sv, Action::SHOW_SIDEBAR},
        {"ACTION_AUDIO_PAUSE_PLAYBACK"sv, Action::AUDIO_PAUSE_PLAYBACK},
        {"ACTION_AUDIO_RECORD"sv, Action::AUDIO_RECORD},
        {"ACTION_GRID_SNAPPING"sv, Action::GRID_SNAPPING},
        {"ACTION_ROTATION_SNAPPING"sv, Action::ROTATION_SNAPPING},
        {"ACTION_HIGHLIGHT_POSITION"sv, Action::POSITION_HIGHLIGHTING}};

static const std::map<std::string_view, const char*> LINE_STYLE_MAP = {{"ACTION_TOOL_LINE_STYLE_PLAIN"sv, "plain"},
                                                                       {"ACTION_TOOL_LINE_STYLE_DASH"sv, "dash"},
                                                                       {"ACTION_TOOL_LINE_STYLE_DASH_DOT"sv, "dashdot"},
                                                                       {"ACTION_TOOL_LINE_STYLE_DOT"sv, "dot"}};

static const std::map<std::string_view, EraserType> ERASER_TYPE_MAP = {
        {"ACTION_TOOL_ERASER_STANDARD"sv, ERASER_TYPE_DEFAULT},
        {"ACTION_TOOL_ERASER_DELETE_STROKE"sv, ERASER_TYPE_DELETE_STROKE},
        {"ACTION_TOOL_ERASER_WHITEOUT"sv, ERASER_TYPE_WHITEOUT}};

static const std::map<std::string_view, int> COL_ROWS_MAP = {
        {"ACTION_SET_COLUMNS_1"sv, 1}, {"ACTION_SET_COLUMNS_2"sv, 2}, {"ACTION_SET_COLUMNS_3"sv, 3},
        {"ACTION_SET_COLUMNS_4"sv, 4}, {"ACTION_SET_COLUMNS_5"sv, 5}, {"ACTION_SET_COLUMNS_6"sv, 6},
        {"ACTION_SET_COLUMNS_7"sv, 7}, {"ACTION_SET_COLUMNS_8"sv, 8}, {"ACTION_SET_ROWS_1"sv, -1},
        {"ACTION_SET_ROWS_2"sv, -2},   {"ACTION_SET_ROWS_3"sv, -3},   {"ACTION_SET_ROWS_4"sv, -4},
        {"ACTION_SET_ROWS_5"sv, -5},   {"ACTION_SET_ROWS_6"sv, -6},   {"ACTION_SET_ROWS_7"sv, -7},
        {"ACTION_SET_ROWS_8"sv, -8}};

static const std::map<std::string_view, std::pair<Action, bool>> LAYOUT_MAP = {
        {"ACTION_SET_LAYOUT_HORIZONTAL"sv, {Action::SET_LAYOUT_VERTICAL, false}},
        {"ACTION_SET_LAYOUT_VERTICAL"sv, {Action::SET_LAYOUT_VERTICAL, true}},
        {"ACTION_SET_LAYOUT_L2R"sv, {Action::SET_LAYOUT_RIGHT_TO_LEFT, false}},
        {"ACTION_SET_LAYOUT_R2L"sv, {Action::SET_LAYOUT_RIGHT_TO_LEFT, true}},
        {"ACTION_SET_LAYOUT_T2B"sv, {Action::SET_LAYOUT_BOTTOM_TO_TOP, false}},
        {"ACTION_SET_LAYOUT_B2T"sv, {Action::SET_LAYOUT_BOTTOM_TO_TOP, true}}};

static const std::map<std::string_view, EditSelection::OrderChange> SELECTION_ORDER_ACTION_MAP = {
        {"ACTION_ARRANGE_BRING_TO_FRONT"sv, EditSelection::OrderChange::BringToFront},
        {"ACTION_ARRANGE_BRING_FORWARD"sv, EditSelection::OrderChange::BringForward},
        {"ACTION_ARRANGE_SEND_BACKWARD"sv, EditSelection::OrderChange::SendBackward},
        {"ACTION_ARRANGE_SEND_TO_BACK"sv, EditSelection::OrderChange::SendToBack}};

void ActionBackwardCompatibilityLayer::actionPerformed(Control* ctrl, std::string_view type, bool enabled) {
    auto* actionDB = ctrl->getActionDatabase();
    if (auto it = STATELESS_ACTION_MAP.find(type); it != STATELESS_ACTION_MAP.end()) {
        actionDB->fireActivateAction(it->second);
    } else if (auto it = SELECT_TOOL_MAP.find(type); it != SELECT_TOOL_MAP.end()) {
        actionDB->fireChangeActionState(Action::SELECT_TOOL, it->second);
    } else if (auto it = TOOL_SIZE_MAP.find(type); it != TOOL_SIZE_MAP.end()) {
        actionDB->fireChangeActionState(it->second.first, it->second.second);
    } else if (auto it = LINE_STYLE_MAP.find(type); it != LINE_STYLE_MAP.end()) {
        actionDB->fireChangeActionState(Action::TOOL_PEN_LINE_STYLE, it->second);
    } else if (auto it = ERASER_TYPE_MAP.find(type); it != ERASER_TYPE_MAP.end()) {
        actionDB->fireChangeActionState(Action::TOOL_ERASER_TYPE, it->second);
    } else if (auto it = BOOLEAN_ACTION_MAP.find(type); it != BOOLEAN_ACTION_MAP.end()) {
        actionDB->fireChangeActionState(it->second, enabled);
    } else if (auto it = COL_ROWS_MAP.find(type); it != COL_ROWS_MAP.end()) {
        actionDB->fireChangeActionState(Action::SET_COLUMNS_OR_ROWS, it->second);
    } else if (auto it = LAYOUT_MAP.find(type); it != LAYOUT_MAP.end()) {
        actionDB->fireChangeActionState(it->second.first, it->second.second);
    } else if (auto it = SELECTION_ORDER_ACTION_MAP.find(type); it != SELECTION_ORDER_ACTION_MAP.end()) {
        actionDB->fireActivateAction(Action::ARRANGE_SELECTION_ORDER, it->second);
    } else {
        // The most specific translations not handled by the above cases
        if (type == "ACTION_TOGGLE_PAIRS_PARITY") {
            int offset = ctrl->getSettings()->getPairsOffset();
            actionDB->fireChangeActionState(Action::PAIRED_PAGES_OFFSET, offset + (offset % 2 == 0 ? 1 : -1));
        } else {
            g_warning("Plugin: Unhandled action event: %s", type.data());
        }
    }
}

#endif
