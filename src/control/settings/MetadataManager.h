/*
 * Xournal++
 *
 * Last opened files with all settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <glib.h>
#include <StringUtils.h>

#include <fstream>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
namespace bp = boost::property_tree;

class MetadataManager
{
public:
	MetadataManager();
	virtual ~MetadataManager();

public:
	/**
	 * Setter / Getter: if path is empty the request will be ignored
	 */

	void setInt(path p, string name, int value);
	void setDouble(path p, string name, double value);
	void setString(path p, string name, string value);

	bool getInt(path p, string name, int& value);
	bool getDouble(path p, string name, double& value);

	bool getString(path p, string name, string& value);

	void move(path source, path target);

private:
	void openFile();
	
	void updateAccessTime(path p);
	void loadConfigFile();

	void cleanupMetadata();

	static bool save(MetadataManager* manager);

	static path getFilePath();
	static string getURI(path &p);

private:
	XOJ_TYPE_ATTRIB;

	int timeoutId;
	
	std::ofstream file;
	bp::ptree* config;
};
