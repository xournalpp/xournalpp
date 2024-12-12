#include "ShortcutConfiguration.h"

#include <control/ScrollHandler.h>

#include "control/ToolEnums.h"
#include "control/actions/ActionDatabase.h"

// clang-format off
ShortcutConfiguration::ShortcutConfiguration() : actionsShortcuts({
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
        {Action::LAYER_NEW_ABOVE_CURRENT,                    {{CTRL_OR_META, GDK_KEY_l}}},  // Name Add a _Layer above the active layer
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

        {Action::NAV_GOTO_PAGE,                              {{CTRL_OR_META, GDK_KEY_g}}},  // Name _Goto Page
        {Action::NAV_GOTO_NEXT_ANNOTATED_PAGE,               {{CTRL_OR_META & SHIFT, GDK_KEY_Page_Down}}},  // Name N_ext Annotated Page
        {Action::NAV_GOTO_PREVIOUS_ANNOTATED_PAGE,           {{CTRL_OR_META & SHIFT, GDK_KEY_Page_Up}}},  // Name P_revious Annotated Page
        {Action::NAV_GOTO_NEXT,                              {{CTRL_OR_META, GDK_KEY_Page_Down},
                                                              {CTRL_OR_META, GDK_KEY_KP_Page_Down}}},  // Name _Next Page
        {Action::NAV_GOTO_PREVIOUS,                          {{CTRL_OR_META, GDK_KEY_Page_Up},
                                                              {CTRL_OR_META, GDK_KEY_KP_Page_Up}}},  // Name _Previous Page
        {Action::NAV_GOTO_LAST,                              {{NONE, GDK_KEY_End},
                                                              {NONE, GDK_KEY_KP_End}}},  // Name _Last Page
        {Action::NAV_GOTO_FIRST,                             {{NONE, GDK_KEY_Home},
                                                              {NONE, GDK_KEY_KP_Home}}},  // Name _First Pages

        // Put those in as well??
        // {{Action::NAV_MOVE_BY_VISIBLE_AREA, ScrollHandler::DOWN}, {{NONE, GDK_KEY_Page_Down}, {NONE, GDK_KEY_KP_Page_Down}}},
        // {{Action::NAV_MOVE_BY_VISIBLE_AREA, ScrollHandler::UP}, {{NONE, GDK_KEY_Page_Up}, {NONE, GDK_KEY_KP_Page_Up}}},
        // {{Action::NAV_MOVE_ONE_PAGE, ScrollHandler::LEFT}, {{SHIFT, GDK_KEY_Left}, {SHIFT, GDK_KEY_KP_Left}}},
        // {{Action::NAV_MOVE_ONE_PAGE, ScrollHandler::RIGHT}, {{SHIFT, GDK_KEY_Right}, {SHIFT, GDK_KEY_KP_Right}}},
        // {{Action::NAV_MOVE_ONE_PAGE, ScrollHandler::DOWN}, {{SHIFT, GDK_KEY_Down}, {SHIFT, GDK_KEY_KP_Down}}},
        // {{Action::NAV_MOVE_ONE_PAGE, ScrollHandler::UP}, {{SHIFT, GDK_KEY_Up}, {SHIFT, GDK_KEY_KP_Up}}},
        // {{Action::NAV_MOVE_ONE_STEP, ScrollHandler::LEFT}, {{NONE, GDK_KEY_Left}, {NONE, GDK_KEY_KP_Left}}},
        // {{Action::NAV_MOVE_ONE_STEP, ScrollHandler::RIGHT}, {{NONE, GDK_KEY_Right}, {NONE, GDK_KEY_KP_Right}}},
        // {{Action::NAV_MOVE_ONE_STEP, ScrollHandler::DOWN}, {{NONE, GDK_KEY_Down}, {NONE, GDK_KEY_KP_Down}}},
        // {{Action::NAV_MOVE_ONE_STEP, ScrollHandler::UP}, {{NONE, GDK_KEY_Up}, {NONE, GDK_KEY_KP_Up}}},
}) {}
// clang-format on
