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

constexpr ToolbarEntryDefinition TOOLBAR_DEFINITIONS[] = {
        {"tbTop1", "toolbarTop1"},       {"tbTop2", "toolbarTop2"},
        {"tbLeft1", "toolbarLeft1"},     {"tbLeft2", "toolbarLeft2"},
        {"tbRight1", "toolbarRight1"},   {"tbRight2", "toolbarRight2"},
        {"tbBottom1", "toolbarBottom1"}, {"tbBottom2", "toolbarBottom2"},
        {"tbFloat1", "toolbarFloat1"},  // define this index below as TBFloatFirst
        {"tbFloat2", "toolbarFloat2"},   {"tbFloat3", "toolbarFloat3"},
        {"tbFloat4", "toolbarFloat4"}  // define this index below as TBFloatLast
};


constexpr size_t TOOLBAR_DEFINITIONS_LEN = G_N_ELEMENTS(TOOLBAR_DEFINITIONS);

#define TBFloatFirst 8
#define TBFloatLast 11
