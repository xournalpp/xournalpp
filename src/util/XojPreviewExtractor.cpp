#include "XojPreviewExtractor.h"

#include <glib.h>
#include <zlib.h>
#include <string.h>

#include <GzUtil.h>

const char* TAG_PREVIEW_NAME = "preview";
const int TAG_PREVIEW_NAME_LEN = strlen(TAG_PREVIEW_NAME);
const char* TAG_PAGE_NAME = "page";
const int TAG_PAGE_NAME_LEN = strlen(TAG_PAGE_NAME);
const char* TAG_PREVIEW_END_NAME = "/preview";
const int TAG_PREVIEW_END_NAME_LEN = strlen(TAG_PREVIEW_END_NAME);
#define BUF_SIZE 8192

XojPreviewExtractor::XojPreviewExtractor()
{
}

XojPreviewExtractor::~XojPreviewExtractor()
{
	g_free(data);
	data = NULL;
	dataLen = 0;
}

/**
 * @return The preview data, should be a binary PNG
 */
unsigned char* XojPreviewExtractor::getData(gsize& dataLen)
{
	dataLen = this->dataLen;
	return this->data;
}

/**
 * Try to read the preview from byte buffer
 * @param buffer Buffer
 * @param len Buffer len
 * @return If an image was read, or the error
 */
PreviewExtractResult XojPreviewExtractor::readPreview(char* buffer, int len)
{
	bool inTag = false;
	int startTag = 0;
	int startPreview = -1;
	int endPreview = -1;
	int pageStart = -1;
	for (int i = 0; i < len; i++)
	{
		if (inTag)
		{
			if (buffer[i] == '>')
			{
				inTag = false;
				int tagLen = i - startTag;
				if (tagLen == TAG_PREVIEW_NAME_LEN && strncmp(TAG_PREVIEW_NAME, buffer + startTag, TAG_PREVIEW_NAME_LEN) == 0)
				{
					startPreview = i + 1;
				}
				if (tagLen == TAG_PREVIEW_END_NAME_LEN && strncmp(TAG_PREVIEW_END_NAME, buffer + startTag, TAG_PREVIEW_END_NAME_LEN) == 0)
				{
					endPreview = i - TAG_PREVIEW_END_NAME_LEN - 1;
					break;
				}
				if (tagLen >= TAG_PAGE_NAME_LEN && strncmp(TAG_PAGE_NAME, buffer + startTag, TAG_PAGE_NAME_LEN) == 0)
				{
					pageStart = i;
					break;
				}
			}
			continue;
		}

		if (buffer[i] == '<')
		{
			inTag = true;
			startTag = i + 1;
			continue;
		}
	}

	if (startPreview != -1 && endPreview != -1)
	{
		buffer[endPreview] = 0;
		this->data = g_base64_decode(buffer + startPreview, &dataLen);
		return PREVIEW_RESULT_IMAGE_READ;
	}

	if (pageStart != -1)
	{
		return PREVIEW_RESULT_NO_PREVIEW;
	}

	return PREVIEW_RESULT_ERROR_READING_PREVIEW;
}

/**
 * Try to read the preview from file
 * @param file .xoj File
 * @return true if a preview was read, false if not
 */
PreviewExtractResult XojPreviewExtractor::readFile(Path file)
{
	// check file extensions
	if (!file.hasXournalFileExt())
	{
		return PREVIEW_RESULT_BAD_FILE_EXTENSION;
	}

	gzFile fp = GzUtil::openPath(file, "r");
	if (!fp)
	{
		return PREVIEW_RESULT_COULD_NOT_OPEN_FILE;
	}

	// The <preview> Tag is within the first 179 Bytes
	// The Preview should end within the first 8k

	char buffer[BUF_SIZE];
	int readLen = gzread(fp, buffer, BUF_SIZE);

	PreviewExtractResult result = readPreview(buffer, readLen);

	gzclose(fp);
	return result;
}


