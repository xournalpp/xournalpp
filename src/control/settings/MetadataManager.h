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

#include <StringUtils.h>

#include <glib.h>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
namespace bp = boost::property_tree;

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
namespace basio = boost::asio;

#include <boost/thread.hpp>

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

	void copy(path source, path target);

	bool save();
	static bool save(MetadataManager* man, basio::deadline_timer* t = NULL);

private:
	bool checkPath(path p);
	
	void updateAccessTime(path p);
	void loadConfigFile();

	void cleanupMetadata();

	static path getFilePath();
	static bp::ptree::path_type getINIpathURI(path p);
	static bp::ptree::path_type getINIpath(string s);

private:
	XOJ_TYPE_ATTRIB;

	boost::thread* thread;
	basio::io_service io;
	basio::deadline_timer* timer;
	
	bp::ptree* config;
};
