/*
 * Xournal++
 *
 * Sidebar preview layout
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

class SidebarPreviewBase;

class SidebarLayout {
public:
    SidebarLayout() = delete;
    ~SidebarLayout() = delete;

public:
    /**
     * Layouts the sidebar
     */
    static void layout(SidebarPreviewBase* sidebar);

private:
};
