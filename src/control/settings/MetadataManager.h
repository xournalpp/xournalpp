/*
 * Xournal++
 *
 * Last opened files with all settings
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __METADATAMANAGER_H__
#define __METADATAMANAGER_H__

#include <glib.h>
#include <StringUtils.h>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

class MetadataManager
{
public:
	MetadataManager();
	virtual ~MetadataManager();

public:
	/**
	 * Setter / Getter: if uri is NULL the request will be ignored
	 */

	void setInt(path p, const char* name, int value);
	void setDouble(path p, const char* name, double value);
	void setString(path p, const char* name, const char* value);

	bool getInt(path p, const char* name, int& value);
	bool getDouble(path p, const char* name, double& value);

	/**
	 * The returned String should be freed with g_free
	 */
	bool getString(path p, const char* name, char*& value);

	void move(string source, string target);

private:
	void updateAccessTime(path p);
	void loadConfigFile();

	void cleanupMetadata();

	static bool save(MetadataManager* manager);


private:
	XOJ_TYPE_ATTRIB;

	int timeoutId;
	GKeyFile* config;
};

#endif /* __METADATAMANAGER_H__ */
