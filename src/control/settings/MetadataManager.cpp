#include "MetadataManager.h"
#include "../../cfg.h"

#include <gtk/gtk.h>

#define FILENAME() g_build_filename(g_get_home_dir(), CONFIG_DIR, METADATA_FILE, NULL)

MetadataManager::MetadataManager()
{
	XOJ_INIT_TYPE(MetadataManager);

	this->config = NULL;
	this->timeoutId = 0;
}

MetadataManager::~MetadataManager()
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (this->timeoutId)
	{
		g_source_remove(this->timeoutId);
		this->timeoutId = 0;
		save(this);
	}

	if (this->config)
	{
		g_key_file_free(this->config);
		this->config = NULL;
	}

	XOJ_RELEASE_TYPE(MetadataManager);
}

void MetadataManager::setInt(string uri, const char* name, int value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (uri.empty())
	{
		return;
	}
	loadConfigFile();

	g_key_file_set_integer(this->config, uri.c_str(), name, value);

	updateAccessTime(uri);
}

void MetadataManager::setDouble(string uri, const char* name, double value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (uri.empty())
	{
		return;
	}
	loadConfigFile();

	g_key_file_set_double(this->config, uri.c_str(), name, value);

	updateAccessTime(uri);
}

void MetadataManager::setString(string uri, const char* name,
                                const char* value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (uri.empty())
	{
		return;
	}
	loadConfigFile();

	g_key_file_set_value(this->config, uri.c_str(), name, value);

	updateAccessTime(uri);
}

void MetadataManager::updateAccessTime(string uri)
{
	XOJ_CHECK_TYPE(MetadataManager);

	// TODO LOW PRIO: newer GTK Version use _int64 instead of integer
	g_key_file_set_integer(this->config, uri.c_str(), "atime", time(NULL));

	if (this->timeoutId)
	{
		return;
	}

	this->timeoutId = g_timeout_add_seconds_full(G_PRIORITY_DEFAULT_IDLE, 2,
	                                             (GSourceFunc) save, this, NULL);
}

struct GroupTimeEntry
{
	char* group;
	int time;
};

int timeCompareFunc(GroupTimeEntry* a, GroupTimeEntry* b)
{
	return a->time - b->time;
}

void MetadataManager::cleanupMetadata()
{
	XOJ_CHECK_TYPE(MetadataManager);

	GList* data = NULL;

	gsize lenght = 0;
	gchar** groups = g_key_file_get_groups(this->config, &lenght);

	for (gsize i = 0; i < lenght; i++)
	{
		char* group = groups[i];

		GFile* file = g_file_new_for_uri(group);
		bool exists = g_file_query_exists(file, NULL);
		g_object_unref(file);

		if (!exists)
		{
			g_key_file_remove_group(this->config, group, NULL);
			continue;
		}

		GError* error = NULL;
		// TODO LOW PRIO: newer GTK Version use _int64 instead of integer
		int time = g_key_file_get_integer(this->config, group, "atime", &error);
		if (error)
		{
			g_error_free(error);
			continue;
		}

		GroupTimeEntry* e = g_new(GroupTimeEntry, 1);
		e->group = group;
		e->time = time;

		data = g_list_insert_sorted(data, e, (GCompareFunc) timeCompareFunc);
	}

	int count = g_list_length(data);
	GList* d = data;
	if (count > METADATA_MAX_ITEMS)
	{
		for (int i = count - METADATA_MAX_ITEMS; i > 0 && d; i--)
		{
			GroupTimeEntry* e = (GroupTimeEntry*) d->data;
			g_key_file_remove_group(this->config, e->group, NULL);
			d = d->next;
		}
	}

	g_list_foreach(data, (GFunc) g_free, NULL);
	g_list_free(data);

	g_strfreev(groups);
}

void MetadataManager::move(string source, string target)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (source.empty() || target.empty())
	{
		return;
	}

	gsize length = 0;
	gchar** keys = g_key_file_get_keys(this->config, source.c_str(), &length, NULL);

	for (gsize i = 0; i < length; i++)
	{
		char* str = g_key_file_get_string(this->config, source.c_str(), keys[i], NULL);
		g_key_file_set_string(this->config, target.c_str(), keys[i], str);

		g_free(str);
	}

	g_strfreev(keys);
}

bool MetadataManager::save(MetadataManager* manager)
{
	XOJ_CHECK_TYPE_OBJ(manager, MetadataManager);

	manager->timeoutId = 0;

	manager->cleanupMetadata();

	gsize length = 0;
	char* data = g_key_file_to_data(manager->config, &length, NULL);
	char* fileName = FILENAME();
	GFile* file = g_file_new_for_path(fileName);

	if (!g_file_replace_contents(file, data, length, NULL, false,
	                             G_FILE_CREATE_PRIVATE, NULL, NULL, NULL))
	{
		g_warning("could not write metadata file: %s", fileName);
	}

	g_free(data);
	g_free(fileName);
	g_object_unref(file);

	return false;
}

void MetadataManager::loadConfigFile()
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (this->config)
	{
		return;
	}

	this->config = g_key_file_new();
	g_key_file_set_list_separator(this->config, ',');

	char* file = FILENAME();

	if (g_file_test(file, G_FILE_TEST_EXISTS))
	{
		GError* error = NULL;
		if (!g_key_file_load_from_file(config, file, G_KEY_FILE_NONE, &error))
		{
			g_warning("Metadata file \"%s\" is invalid: %s", file, error->message);
			g_error_free(error);
		}
	}

	g_free(file);
}

bool MetadataManager::getInt(string uri, const char* name, int& value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (uri.empty())
	{
		return false;
	}
	loadConfigFile();

	GError* error = NULL;
	int v = g_key_file_get_integer(this->config, uri.c_str(), name, &error);
	if (error)
	{
		g_error_free(error);
		return false;
	}

	value = v;
	return true;
}

bool MetadataManager::getDouble(string uri, const char* name, double& value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (uri.empty())
	{
		return false;
	}
	loadConfigFile();

	GError* error = NULL;
	double v = g_key_file_get_double(this->config, uri.c_str(), name, &error);
	if (error)
	{
		g_error_free(error);
		return false;
	}

	value = v;
	return true;
}

bool MetadataManager::getString(string uri, const char* name, char*& value)
{
	XOJ_CHECK_TYPE(MetadataManager);

	if (uri.empty())
	{
		return false;
	}
	loadConfigFile();

	GError* error = NULL;

	char* v = g_key_file_get_string(this->config, uri.c_str(), name, &error);
	if (error)
	{
		g_error_free(error);
		return false;
	}

	value = v;
	return true;
}
