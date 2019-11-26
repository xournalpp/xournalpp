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
	MetadataEntry getForFile(const string& file);

	/**
	 * Store the current data into metadata
	 */
	void storeMetadata(const string& file, int page, double zoom);

	/**
	 * Document was closed, a new document was opened etc.
	 */
	void documentChanged();

private:
	/**
	 * Delete an old metadata file
	 */
	static void deleteMetadataFile(const string& path);

	/**
	 * Parse a single metadata file
	 */
	static MetadataEntry loadMetadataFile(const string& path, const string& file);

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
	GMutex mutex{};
	MetadataEntry* metadata;
};
