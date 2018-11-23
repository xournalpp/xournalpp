#include "XojPreviewExtractor.h"

#include <glib.h>
#include <glib/gstdio.h>

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/algorithm/string.hpp>
namespace bio = boost::iostreams;

#include <iostream>
using std::istream;
#include <fstream>
using std::ifstream;
#include <memory>
using std::unique_ptr;

#define TAG_PREVIEW_NAME "preview"
#define TAG_PAGE_NAME "page"
#define BUF_SIZE 8192

XojPreviewExtractor::XojPreviewExtractor()
 : data(NULL),
   dataLen(0)
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

#define CLOSE		\
	if (gzip)		\
	{				\
		delete in;	\
	}				\
	ifile.close();	\
	return 

/**
 * Try to read the preview from file
 * @param file .xoj File
 * @return true if a preview was read, false if not
 */
PreviewExtractResult XojPreviewExtractor::readFile(string file)
{
	// check file extensions
	string ext = "";
	size_t dotPos = file.find_last_of(".");
	if (dotPos != string::npos)
	{
		ext = file.substr(dotPos);
		boost::algorithm::to_lower(ext);
	}

	if (!(ext == ".xoj" || ext == ".xopp"))
	{
		return PREVIEW_RESULT_BAD_FILE_EXTENSION;
	}

	// open input file
	ifstream ifile(file, ifstream::in | ifstream::binary);
	if (!ifile.is_open())
	{
		return PREVIEW_RESULT_COULD_NOT_OPEN_FILE;
	}

	istream* in;
	bio::filtering_istreambuf inbuf;
	bool gzip;
	
	// check for gzip magic header
	if (ifile.get() == 0x1F && ifile.get() == 0x8B)
	{
		gzip = true;
		ifile.seekg(0); //seek back to beginning
		
		inbuf.push(bio::gzip_decompressor());
		inbuf.push(ifile);

		in = new istream(&inbuf);
	}
	else
	{
		gzip = false;
		in = &ifile;
	}
	
	char buf[BUF_SIZE];
	bool inPreview = false;
	bool inTag = false;
	string preview;
	while (!in->eof())
	{
		if (in->peek() == '<')
		{
			in->ignore();
			inTag = true;
			continue;
		}
		
		in->get(buf, BUF_SIZE, '<');
		if (!inPreview)
		{
			if (inTag)
			{
				if (strncmp(TAG_PREVIEW_NAME, buf, sizeof(TAG_PREVIEW_NAME) - 1) == 0)
				{
					inPreview = true;
					preview += (buf + sizeof(TAG_PREVIEW_NAME));
				}
				else if (strncmp(TAG_PAGE_NAME, buf, sizeof(TAG_PAGE_NAME) - 1) == 0)
				{
					CLOSE PREVIEW_RESULT_NO_PREVIEW;
				}
				inTag = false;
			}
		}
		else
		{
			if (inTag)
			{
				this->data = g_base64_decode(preview.c_str(), &this->dataLen);
				CLOSE PREVIEW_RESULT_IMAGE_READ;
			}
			else
			{
				preview += buf;
			}
		}
	}

	CLOSE PREVIEW_RESULT_ERROR_READING_PREVIEW;
}


