#include "XojPreviewExtractor.h"

#include <zlib.h>

/**
 * Constructor
 */
XojPreviewExtractor::XojPreviewExtractor()
{
	this->data = NULL;
	this->dataLength = 0;
}

/**
 * Destructor
 */
XojPreviewExtractor::~XojPreviewExtractor()
{
	g_free(this->data);
	this->data = NULL;
}


/**
 * @return The preview data, should be a binary PNG
 */
const unsigned char* XojPreviewExtractor::getData() const
{
	return this->data;
}

/**
 * @return Length of the data array
 */
gsize XojPreviewExtractor::getDataLength() const
{
	return this->dataLength;
}


/**
 * Try to read the preview from file
 * @param file .xoj File
 * @return true if a preview was read, false if not
 */
PreviewExtractResult XojPreviewExtractor::readFile(std::string file)
{
	gzFile inputFile = gzopen(file.c_str(), "r");
	char buffer[2048]; // 2k

	if (inputFile == NULL)
	{
		return PREVIEW_RESULT_COULD_NOT_OPEN_FILE;
	}

	std::string data;
	std::string preview;

	PreviewExtractResult result = PREVIEW_RESULT_IMAGE_READ;

	do {
		int count = gzread(inputFile, buffer, sizeof(buffer));
		if (count == 0)
		{
			result = PREVIEW_RESULT_ERROR_READING_PREVIEW;
			break;
		}
		data.append(buffer, count);

		// End of preview found, we don't need more of the file
		size_t posPreviewEnd = data.find("</preview>");
		if (posPreviewEnd != std::string::npos)
		{
			size_t posPreviewStart = data.find("<preview>");
			if (posPreviewStart == std::string::npos)
			{
				result = PREVIEW_RESULT_ERROR_READING_PREVIEW;
				break;
			}

			preview = data.substr(posPreviewStart + 9, posPreviewEnd - posPreviewStart - 9);

			break;
		}

		// The content has started, but there is no preview
		if (data.find("<page") != std::string::npos)
		{
			result = PREVIEW_RESULT_NO_PREVIEW;
		}

	} while(data.size() < 20480 && result == PREVIEW_RESULT_IMAGE_READ); // max, 20k, no error occured

	data.clear();
	gzclose(inputFile);

	if (preview.size() > 0)
	{
		// free old buffer (if any)
		g_free(this->data);
		this->data = g_base64_decode (preview.c_str(), &this->dataLength);
	}

	return result;
}


