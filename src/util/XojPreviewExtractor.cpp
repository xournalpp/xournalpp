#include "XojPreviewExtractor.h"

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
namespace bio = boost::iostreams;

#include <glibmm/base64.h>

#include <string>
using std::string;
#include <iostream>
using std::istream;
#include <fstream>
using std::ifstream;

#define TAG_PREVIEW_NAME "preview"
#define TAG_PAGE_NAME "page"
#define BUF_SIZE 8192

/**
 * @return The preview data, should be a binary PNG
 */
string XojPreviewExtractor::getData() const
{
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
PreviewExtractResult XojPreviewExtractor::readFile(std::string file)
{
	//check file extensions
	string ext = file.substr(file.length() - 4, file.length());
	for (string::size_type i = 0; i < ext.length(); i++)
	{
		if (tolower(ext[i]) != ".xoj"[i])
		{
			return PREVIEW_RESULT_BAD_FILE_EXTENSION;
		}
	}

	//open input file
	ifstream ifile(file, ifstream::in | ifstream::binary);
	if (!ifile.is_open())
	{
		return PREVIEW_RESULT_COULD_NOT_OPEN_FILE;
	}

	istream* in;
	bio::filtering_istreambuf inbuf;
	bool gzip;
	
	//check for gzip magic header
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
				this->data = Glib::Base64::decode(preview);
				
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


