#include "MetadataManager.h"

#include <Util.h>
#include <XojMsgBox.h>

#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fstream>
#include <sstream>
#include <algorithm> // std::sort

using namespace std;

MetadataEntry::MetadataEntry()
 : valid(false),
   zoom(1),
   page(0),
   time(0)
{
}


MetadataManager::MetadataManager()
 : metadata(nullptr)
{
	g_mutex_init(&this->mutex);
}

MetadataManager::~MetadataManager()
{
	documentChanged();
}

/**
 * Delete an old metadata file
 */
void MetadataManager::deleteMetadataFile(string path)
{
	// be carefull, delete the Metadata file, NOT the Document!
	if (path.substr(path.size() - 9) != ".metadata")
	{
		g_warning("Try to delete non-metadata file: %s", path.c_str());
		return;
	}

	int result = g_unlink(path.c_str());
	if (result != 0)
	{
		g_warning("Could not delete metadata file %s", path.c_str());
	}
}

/**
 * Document was closed, a new document was opened etc.
 */
void MetadataManager::documentChanged()
{
	g_mutex_lock(&this->mutex);
	MetadataEntry* m = metadata;
	metadata = nullptr;
	g_mutex_unlock(&this->mutex);

	if (m == nullptr)
	{
		return;
	}

	storeMetadata(m);
	delete m;
}

bool sortMetadata(MetadataEntry& a, MetadataEntry& b)
{
	return a.time > b.time;
}

/**
 * Load the metadata list (sorted)
 */
vector<MetadataEntry> MetadataManager::loadList()
{
	Path folder = Util::getConfigSubfolder("metadata");

	vector<MetadataEntry> data;

	GError* error = nullptr;
	GDir* home = g_dir_open(folder.c_str(), 0, &error);
	if (error != nullptr)
	{
		XojMsgBox::showErrorToUser(nullptr, error->message);
		g_error_free(error);
		return data;
	}

	const gchar* file;
	while ((file = g_dir_read_name(home)) != nullptr)
	{
		string path = folder.str();
		path += "/";
		path += file;

		MetadataEntry entry = loadMetadataFile(path, file);

		if (entry.valid)
		{
			data.push_back(entry);
		}
	}
	g_dir_close(home);

	std::sort(data.begin(), data.end(), sortMetadata);

	return data;
}

/**
 * Parse a single metadata file
 */
MetadataEntry MetadataManager::loadMetadataFile(string path, string file)
{
	MetadataEntry entry;
	entry.metadataFile = path;

	string line;
	ifstream infile(path.c_str());

	string time = file.substr(0, file.size() - 9);
	entry.time = strtoll(time.c_str(), nullptr, 10);

	if (!getline(infile, line))
	{
		deleteMetadataFile(path);
		// Not valid
		return entry;
	}

	if (line != "XOJ-METADATA/1.0")
	{
		deleteMetadataFile(path);
		// Not valid
		return entry;
	}

	if (!getline(infile, line))
	{
		deleteMetadataFile(path);
		// Not valid
		return entry;
	}

	entry.path = line;

	if (!getline(infile, line))
	{
		deleteMetadataFile(path);
		// Not valid
		return entry;
	}

	if (line.length() < 6 || line.substr(0, 5) != "page=")
	{
		deleteMetadataFile(path);
		// Not valid
		return entry;
	}

	entry.page = strtoll(line.substr(5).c_str(), nullptr, 10);

	if (!getline(infile, line))
	{
		deleteMetadataFile(path);
		// Not valid
		return entry;
	}

	if (line.length() < 6 || line.substr(0, 5) != "zoom=")
	{
		deleteMetadataFile(path);
		// Not valid
		return entry;
	}

	entry.zoom = strtod(line.substr(5).c_str(), nullptr);

	entry.valid = true;
	return entry;
}

/**
 * Get the metadata for a file
 */
MetadataEntry MetadataManager::getForFile(string file)
{
	vector<MetadataEntry> files = loadList();

	MetadataEntry entry;
	for (MetadataEntry e : files)
	{
		if (e.path == file)
		{
			entry = e;
			break;
		}
	}

	for (int i = 20; i < (int)files.size(); i++)
	{
		string path = files[i].metadataFile;
		deleteMetadataFile(path);
	}

	return entry;
}

/**
 * Store metadata to file
 */
void MetadataManager::storeMetadata(MetadataEntry* m)
{
	vector<MetadataEntry> files = loadList();
	for (MetadataEntry e : files)
	{
		if (e.path == m->path)
		{
			// This is an old entry with the same path
			deleteMetadataFile(e.metadataFile);
		}
	}

	Path folder = Util::getConfigSubfolder("metadata");
	string path = folder.str();
	path += "/";
	gint64 time = g_get_real_time();
	path += std::to_string(time);
	path += ".metadata";

	ofstream out;
	out.open(path.c_str());
	out << "XOJ-METADATA/1.0\n";
	out << m->path << "\n";
	out << "page=" << m->page << "\n";
	out << "zoom=" << m->zoom << "\n";
	out.close();
}

/**
 * Store the current data into metadata
 */
void MetadataManager::storeMetadata(string file, int page, double zoom)
{
	if (file == "")
	{
		return;
	}

	g_mutex_lock(&this->mutex);
	if (metadata == nullptr)
	{
		metadata = new MetadataEntry();
	}

	metadata->metadataFile = "";
	metadata->valid = true;
	metadata->path = file;
	metadata->zoom = zoom;
	metadata->page = page;
	metadata->time = g_get_real_time();
	g_mutex_unlock(&this->mutex);
}
