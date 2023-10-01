#include <fstream>
#include <glib.h>
#include <set>

#include "WorkspaceHandler.h"
#include "WorkspaceHandlerListener.h"

WorkspaceHandler::WorkspaceHandler(fs::path filepath): filepath(filepath) {}

WorkspaceHandler::~WorkspaceHandler() {
	this->removeAllListeners();
}

void WorkspaceHandler::addListener(WorkspaceHandlerListener* listener) { this->listeners.emplace_back(listener); }

void WorkspaceHandler::removeAllListeners() { this->listeners.clear(); }

bool WorkspaceHandler::addFolder(fs::path folderPath) {
	
	if (!fs::is_directory(folderPath)) {
        g_warning("The added workspace folder path %s does not exist", folderPath.c_str());
        return false;
    }
	
	if (foldersPaths.find(folderPath) != foldersPaths.end()) {
		g_info("The added workspace folder path %s is already added", folderPath.c_str());
		return false;
	}

	foldersPaths.insert(folderPath);
	for (const auto& listener: this->listeners) listener->addFolder(folderPath);

    save();
	return true;
}

void WorkspaceHandler::closeAllFolders() {

	foldersPaths.clear();
	for (const auto &listener: this->listeners) listener->closeAllFolders();

	save();
}

void WorkspaceHandler::load() {
    if (!fs::exists(filepath)) {
        g_warning("Workspace file %s does not exist. Regenerating.", filepath.string().c_str());
        save();
		return;
    }

	std::ifstream file(filepath);
	if (!file.is_open()) {
        g_warning("Cannot open workspace file %s.", filepath.string().c_str());
		return;
    }

	bool dirtyFile = false;
    while (file) {
		std::string line;
		std::getline(file, line);

		if (line.empty())
			continue;
		
		fs::path filePath = fs::path(line);
		if (fs::exists(filePath) && fs::is_directory(filePath)) {
			foldersPaths.insert(filePath);
			addFolder(filePath);
		}
		else {
			g_warning("The workspace file contains an inexisting/invalid path %s", line.c_str());
			dirtyFile = true;
		}
	}

	file.close();
	if (dirtyFile)
		save();
}

void WorkspaceHandler::save() {
	std::ofstream file(filepath);

	for (const auto& path: foldersPaths)
		file << path.generic_string() << std::endl;
	
	file.close();
}

std::set<fs::path> WorkspaceHandler::getFoldersPaths() {
	return foldersPaths;
}
int WorkspaceHandler::getFoldersCount() {
	return foldersPaths.size();
}