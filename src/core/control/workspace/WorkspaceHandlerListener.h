/*
 * Xournal++
 *
 * Layer Controller listener
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <filesystem.h>

class WorkspaceHandler;

class WorkspaceHandlerListener {
public:
    WorkspaceHandlerListener();
    virtual ~WorkspaceHandlerListener();

public:
    void registerListener(WorkspaceHandler* handler);
    
    virtual void addFolder(fs::path folderPath) = 0;
    virtual void closeAllFolders() = 0;

private:
    WorkspaceHandler* handler;
};
