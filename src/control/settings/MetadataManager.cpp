#include "MetadataManager.h"
#include "../../cfg.h"

#include <gtk/gtk.h>

#define FILENAME() g_build_filename(g_get_home_dir(), CONFIG_DIR, METADATA_FILE, NULL)

MetadataManager::MetadataManager() {
	this->config = NULL;
	this->timeoutId = 0;
}

MetadataManager::~MetadataManager() {
	if(this->timeoutId) {
		g_source_remove(this->timeoutId);
		this->timeoutId = 0;

	}

	save(this);

	g_key_file_free(this->config);
}

void MetadataManager::setInt(String uri, const char * name, int value) {
	if (uri.isEmpty()) {
		return;
	}
	loadConfigFile();

	g_key_file_set_integer(this->config, uri.c_str(), name, value);

	updateAccessTime(uri);
}

void MetadataManager::setDouble(String uri, const char * name, double value) {
	if (uri.isEmpty()) {
		return;
	}
	loadConfigFile();

	g_key_file_set_double(this->config, uri.c_str(), name, value);

	updateAccessTime(uri);
}

void MetadataManager::setString(String uri, const char * name, const char * value) {
	if (uri.isEmpty()) {
		return;
	}
	loadConfigFile();

	g_key_file_set_value(this->config, uri.c_str(), name, value);

	updateAccessTime(uri);
}

void MetadataManager::updateAccessTime(String uri) {
	g_key_file_set_integer(this->config, uri.c_str(), "atime", time(NULL));

	if (this->timeoutId) {
		return;
	}

	this->timeoutId = g_timeout_add_seconds_full(G_PRIORITY_DEFAULT_IDLE, 2, (GSourceFunc) save, this, NULL);
}

bool MetadataManager::save(MetadataManager * manager) {
	manager->timeoutId = 0;

	// TODO: remove unused items
	//	resize_items();

	gsize length = 0;
	char * data = g_key_file_to_data(manager->config, &length, NULL);
	char * fileName = FILENAME();
	GFile * file = g_file_new_for_path(fileName);

	if (!g_file_replace_contents(file, data, length, NULL, false, G_FILE_CREATE_PRIVATE, NULL, NULL, NULL)) {
		g_warning("could not write metadata file: %s", fileName);
	}

	g_free(data);
	g_free(fileName);
	g_object_unref(file);

	return false;
}

void MetadataManager::loadConfigFile() {
	if (this->config) {
		return;
	}

	this->config = g_key_file_new();
	g_key_file_set_list_separator(this->config, ',');

	char * file = FILENAME();

	if (g_file_test(file, G_FILE_TEST_EXISTS)) {
		GError * error = NULL;
		if (!g_key_file_load_from_file(config, file, G_KEY_FILE_NONE, &error)) {
			g_warning("Metadata file \"%s\" is invalid: %s", file, error->message);
			g_error_free(error);
		}
	}

	g_free(file);
}

bool MetadataManager::getInt(String uri, const char * name, int &value) {
	if (uri.isEmpty()) {
		return false;
	}
	loadConfigFile();

	GError * error = NULL;
	int v = g_key_file_get_integer(this->config, uri.c_str(), name, &error);
	if (error) {
		g_error_free(error);
		return false;
	}

	value = v;
	return true;
}

bool MetadataManager::getDouble(String uri, const char * name, double &value) {
	if (uri.isEmpty()) {
		return false;
	}
	loadConfigFile();

	GError * error = NULL;
	double v = g_key_file_get_double(this->config, uri.c_str(), name, &error);
	if (error) {
		g_error_free(error);
		return false;
	}

	value = v;
	return true;
}

bool MetadataManager::getString(String uri, const char * name, char * &value) {
	if (uri.isEmpty()) {
		return false;
	}
	loadConfigFile();

	GError * error = NULL;

	char * v = g_key_file_get_string(this->config, uri.c_str(), name, &error);
	if (error) {
		g_error_free(error);
		return false;
	}

	value = v;
	return true;
}
