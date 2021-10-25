/*
 * Xournal++
 *
 * The definitions for the possibel toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>

typedef struct {
    const char* guiName;   ///< The name in the glade.xml file
    const char* propName;  ///< The name in the .ini file
    bool horizontal;       ///< if horizontal (true) or vertical (false)
} ToolbarEntryDefintion;

constexpr std::array TOOLBAR_DEFINITIONS{
        ToolbarEntryDefintion{"tbTop1", "toolbarTop1", true},
        ToolbarEntryDefintion{"tbTop2", "toolbarTop2", true},
        ToolbarEntryDefintion{"tbLeft1", "toolbarLeft1", false},
        ToolbarEntryDefintion{"tbLeft2", "toolbarLeft2", false},
        ToolbarEntryDefintion{"tbRight1", "toolbarRight1", false},
        ToolbarEntryDefintion{"tbRight2", "toolbarRight2", false},
        ToolbarEntryDefintion{"tbBottom1", "toolbarBottom1", true},
        ToolbarEntryDefintion{"tbBottom2", "toolbarBottom2", true},
        ToolbarEntryDefintion{"tbFloat1", "toolbarFloat1", true},  // define this index below as TBFloatFirst
        ToolbarEntryDefintion{"tbFloat2", "toolbarFloat2", true},
        ToolbarEntryDefintion{"tbFloat3", "toolbarFloat3", true},
        ToolbarEntryDefintion{"tbFloat4", "toolbarFloat4", true}  // define this index below as TBFloatLast
};


constexpr auto TOOLBAR_DEFINITIONS_LEN = TOOLBAR_DEFINITIONS.size();

constexpr auto TBFloatFirst = 8;
constexpr auto TBFloatLast = 11;
