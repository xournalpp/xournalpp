/*
 * Xournal++
 *
 * Extracts a preview of an .xoj file, used by xournal-thumbnailer and xournalpp
 * Because of this xournal type checks cannot be used
 *
 * @author Andreas Butti <andreasbutti@gmail.com>
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
using std::string;

enum PreviewExtractResult {
	/**
	 * Successfully read an image from file
	 */
	PREVIEW_RESULT_IMAGE_READ = 0,
	
	/**
	 * File extension is wrong
	 */
	PREVIEW_RESULT_BAD_FILE_EXTENSION,

	/**
	 * The file could not be openend / found
	 */
	PREVIEW_RESULT_COULD_NOT_OPEN_FILE,

	/**
	 * The preview could not be extracted
	 */
	PREVIEW_RESULT_ERROR_READING_PREVIEW,

	/**
	 * The file contains no preview
	 */
	PREVIEW_RESULT_NO_PREVIEW,
};

class XojPreviewExtractor
{
public:

	/**
	 * Try to read the preview from file
	 * @param file .xoj File
	 * @return If an image was read, or the error
	 */
	PreviewExtractResult readFile(string file);

	/**
	 * @return The preview data, should be a binary PNG
	 */
	string getData() const;

	// Member
private:

	/**
	 * Preview data
	 */
	string data;
};
