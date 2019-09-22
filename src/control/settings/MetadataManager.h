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

#include <XournalType.h>

class MetadataEntry
{
public:
	MetadataEntry();

public:
	string metadataFile;
	bool valid;
	string path;
	double zoom;
	int page;
	gint64 time;
};

class MetadataManager
{
public:
	MetadataManager();
	virtual ~MetadataManager();

public:
	/**
	 * Get the metadata for a file
	 */
	MetadataEntry getForFile(string file);

	/**
	 * Store the current data into metadata
	 */
	void storeMetadata(string file, int page, double zoom);

	/**
	 * Document was closed, a new document was opened etc.
	 */
	void documentChanged();

private:
	/**
	 * Delete an old metadata file
	 */
	void deleteMetadataFile(string path);

	/**
	 * Parse a single metadata file
	 */
	MetadataEntry loadMetadataFile(string path, string file);

	/**
	 * Store metadata to file
	 */
	void storeMetadata(MetadataEntry* m);

private:
	/**
	 * Load the metadata list (sorted)
	 */
	vector<MetadataEntry> loadList();


private:
	GMutex mutex;
	MetadataEntry* metadata;
};
