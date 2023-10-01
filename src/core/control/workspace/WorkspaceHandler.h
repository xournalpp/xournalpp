/*
 * Xournal++
 *
 * Workspace Handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <filesystem.h>
#include <set>
#include <vector>

class WorkspaceHandlerListener;

class WorkspaceHandler {
public:
    
    WorkspaceHandler(fs::path filepath);
    ~WorkspaceHandler();

public:
    void addListener(WorkspaceHandlerListener* listener);
    void removeAllListeners();

    bool addFolder(fs::path folderPath);
    void closeAllFolders();

public:
    void load();
    void save();

    std::set<fs::path> getFoldersPaths();
    int getFoldersCount();

private:
    fs::path filepath;
    std::vector<WorkspaceHandlerListener*> listeners;
    std::set<fs::path> foldersPaths;

};
