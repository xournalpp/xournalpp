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

class MetadataManager
{
public:
	MetadataManager();
	virtual ~MetadataManager();

public:
	/**
	 * Setter / Getter: if uri is NULL the request will be ignored
	 */

	void setInt(string uri, const char* name, int value);
	void setDouble(string uri, const char* name, double value);
	void setString(string uri, const char* name, const char* value);

	bool getInt(string uri, const char* name, int& value);
	bool getDouble(string uri, const char* name, double& value);

	/**
	 * The returned String should be freed with g_free
	 */
	bool getString(string uri, const char* name, char*& value);

	void move(string source, string target);

private:
	void updateAccessTime(string uri);
	void loadConfigFile();

	void cleanupMetadata();

	static bool save(MetadataManager* manager);


private:
	XOJ_TYPE_ATTRIB;

	int timeoutId;
	GKeyFile* config;
};

#endif /* __METADATAMANAGER_H__ */
