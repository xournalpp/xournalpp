#include "util/XojPreviewExtractor.h"

#include <array>    // for array
#include <cstring>  // for strlen, strncmp
#include <string>   // for allocator, string

#include <glib.h>     // for g_free, g_base64_decode, g_malloc, gsize
#include <zip.h>      // for zip_close, zip_fclose, zip_stat_t, zip_fopen
#include <zipconf.h>  // for zip_int64_t, zip_uint64_t
#include <zlib.h>     // for gzclose, gzread, gzFile

#include "util/GzUtil.h"    // for GzUtil
#include "util/PathUtil.h"  // for hasXournalFileExt

#include "filesystem.h"  // for path

const char* TAG_PREVIEW_NAME = "preview";
const int TAG_PREVIEW_NAME_LEN = strlen(TAG_PREVIEW_NAME);
const char* TAG_PAGE_NAME = "page";
const int TAG_PAGE_NAME_LEN = strlen(TAG_PAGE_NAME);
const char* TAG_PREVIEW_END_NAME = "/preview";
const int TAG_PREVIEW_END_NAME_LEN = strlen(TAG_PREVIEW_END_NAME);
constexpr auto BUF_SIZE = 8192;

XojPreviewExtractor::XojPreviewExtractor() = default;

XojPreviewExtractor::~XojPreviewExtractor() {
    g_free(data);
    data = nullptr;
    dataLen = 0;
}

/**
 * @return The preview data, should be a binary PNG
 */
auto XojPreviewExtractor::getData(gsize& dataLen) -> unsigned char* {
    dataLen = this->dataLen;
    return this->data;
}

/**
 * Try to read the preview from byte buffer
 * @param buffer Buffer
 * @param len Buffer len
 * @return If an image was read, or the error
 */
auto XojPreviewExtractor::readPreview(char* buffer, int len) -> PreviewExtractResult {
    bool inTag = false;
    int startTag = 0;
    int startPreview = -1;
    int endPreview = -1;
    int pageStart = -1;
    for (int i = 0; i < len; i++) {
        if (inTag) {
            if (buffer[i] == '>') {
                inTag = false;
                int tagLen = i - startTag;
                if (tagLen == TAG_PREVIEW_NAME_LEN &&
                    strncmp(TAG_PREVIEW_NAME, buffer + startTag, TAG_PREVIEW_NAME_LEN) == 0) {
                    startPreview = i + 1;
                }
                if (tagLen == TAG_PREVIEW_END_NAME_LEN &&
                    strncmp(TAG_PREVIEW_END_NAME, buffer + startTag, TAG_PREVIEW_END_NAME_LEN) == 0) {
                    endPreview = i - TAG_PREVIEW_END_NAME_LEN - 1;
                    break;
                }
                if (tagLen >= TAG_PAGE_NAME_LEN && strncmp(TAG_PAGE_NAME, buffer + startTag, TAG_PAGE_NAME_LEN) == 0) {
                    pageStart = i;
                    break;
                }
            }
            continue;
        }

        if (buffer[i] == '<') {
            inTag = true;
            startTag = i + 1;
            continue;
        }
    }

    if (startPreview != -1 && endPreview != -1) {
        buffer[endPreview] = 0;
        this->data = g_base64_decode(buffer + startPreview, &dataLen);
        return PREVIEW_RESULT_IMAGE_READ;
    }

    if (pageStart != -1) {
        return PREVIEW_RESULT_NO_PREVIEW;
    }

    return PREVIEW_RESULT_ERROR_READING_PREVIEW;
}

/**
 * Try to read the preview from file
 * @param file .xoj File
 * @return true if a preview was read, false if not
 */
auto XojPreviewExtractor::readFile(const fs::path& file) -> PreviewExtractResult {
    // check file extensions
    if (!Util::hasXournalFileExt(file)) {
        return PREVIEW_RESULT_BAD_FILE_EXTENSION;
    }
    // read the new file format
    int zipError = 0;
    zip_t* zipFp = zip_open(file.u8string().c_str(), ZIP_RDONLY, &zipError);

    if (!zipFp && zipError == ZIP_ER_NOZIP) {
        gzFile fp = GzUtil::openPath(file, "r");
        if (!fp) {
            return PREVIEW_RESULT_COULD_NOT_OPEN_FILE;
        }

        // The <preview> Tag is within the first 179 Bytes
        // The Preview should end within the first 8k

        std::array<char, BUF_SIZE> buffer{};
        int readLen = gzread(fp, buffer.data(), BUF_SIZE);

        PreviewExtractResult result = readPreview(buffer.data(), readLen);

        gzclose(fp);
        return result;
    }
    if (!zipFp) {
        return PREVIEW_RESULT_COULD_NOT_OPEN_FILE;
    }

    zip_stat_t thumbStat;
    int statStatus = zip_stat(zipFp, "thumbnails/thumbnail.png", 0, &thumbStat);
    if (statStatus != 0) {
        zip_close(zipFp);
        return PREVIEW_RESULT_NO_PREVIEW;
    }

    if (thumbStat.valid & ZIP_STAT_SIZE) {
        dataLen = thumbStat.size;
    } else {
        zip_close(zipFp);
        return PREVIEW_RESULT_ERROR_READING_PREVIEW;
    }

    zip_file_t* thumb = zip_fopen(zipFp, "thumbnails/thumbnail.png", 0);

    if (!thumb) {
        zip_close(zipFp);
        return PREVIEW_RESULT_ERROR_READING_PREVIEW;
    }

    data = static_cast<unsigned char*>(g_malloc(thumbStat.size));
    zip_uint64_t readBytes = 0;
    while (readBytes < dataLen) {
        zip_int64_t read = zip_fread(thumb, data, thumbStat.size);
        if (read == -1) {
            g_free(data);
            zip_fclose(thumb);
            zip_close(zipFp);
            return PREVIEW_RESULT_ERROR_READING_PREVIEW;
        }
        readBytes += read;
    }

    zip_fclose(thumb);
    zip_close(zipFp);
    return PREVIEW_RESULT_IMAGE_READ;
}
