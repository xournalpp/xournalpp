/*
 * Xournal++
 *
 * Extracts a preview of an .xoj file, used by xournalpp-thumbnailer and xournalpp
 * Because of this xournal type checks cannot be used
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <glib.h>  // for gsize

#include "filesystem.h"  // for path

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

class XojPreviewExtractor {
public:
    XojPreviewExtractor();
    ~XojPreviewExtractor();

public:
    /**
     * Try to read the preview from file
     * @param file .xoj File
     * @return If an image was read, or the error
     */
    PreviewExtractResult readFile(const fs::path& file);

    /**
     * Try to read the preview from byte buffer
     * @param buffer Buffer
     * @param len Buffer len
     * @return If an image was read, or the error
     */
    PreviewExtractResult readPreview(char* buffer, int len);

    /**
     * @return The preview data, should be a binary PNG
     */
    unsigned char* getData(gsize& dataLen);

    // Member
private:
    /**
     * Preview data
     */
    unsigned char* data = nullptr;
    gsize dataLen = 0;
};
