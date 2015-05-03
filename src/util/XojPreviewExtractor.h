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
#include <glib.h>

enum PreviewExtractResult {
	/**
	 * Successfully read an image from file
	 */
	PREVIEW_RESULT_IMAGE_READ = 0,

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
	 * Constructor
	 */
	XojPreviewExtractor();

	/**
	 * Destructor, frees allocated image buffer, if any
	 */
	virtual ~XojPreviewExtractor();

public:

	/**
	 * Try to read the preview from file
	 * @param file .xoj File
	 * @return If an image was read, or the error
	 */
	PreviewExtractResult readFile(std::string file);

	/**
	 * @return The preview data, should be a binary PNG
	 */
	const unsigned char* getData() const;

	/**
	 * @return Length of the data array
	 */
	gsize getDataLength() const;

	// Member
private:

	/**
	 * Preview data
	 */
	unsigned char* data;

	/**
	 * Size of the data array
	 */
	gsize dataLength;
};
