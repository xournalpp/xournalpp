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

typedef struct {
    /**
     * The name in the main.ui file
     */
    const char* guiName;

    /**
     * The name in the .ini file
     */
    const char* propName;
} ToolbarEntryDefinition;

constexpr std::array<ToolbarEntryDefinition, 12> TOOLBAR_DEFINITIONS(
        {ToolbarEntryDefinition{"tbTop1", "toolbarTop1"}, ToolbarEntryDefinition{"tbTop2", "toolbarTop2"},
         ToolbarEntryDefinition{"tbLeft1", "toolbarLeft1"}, ToolbarEntryDefinition{"tbLeft2", "toolbarLeft2"},
         ToolbarEntryDefinition{"tbRight1", "toolbarRight1"}, ToolbarEntryDefinition{"tbRight2", "toolbarRight2"},
         ToolbarEntryDefinition{"tbBottom1", "toolbarBottom1"}, ToolbarEntryDefinition{"tbBottom2", "toolbarBottom2"},
         ToolbarEntryDefinition{"tbFloat1", "toolbarFloat1"},  // define this index below as TBFloatFirst
         ToolbarEntryDefinition{"tbFloat2", "toolbarFloat2"}, ToolbarEntryDefinition{"tbFloat3", "toolbarFloat3"},
         ToolbarEntryDefinition{"tbFloat4", "toolbarFloat4"}});

#define TBFloatFirst 8
#define TBFloatLast 11
