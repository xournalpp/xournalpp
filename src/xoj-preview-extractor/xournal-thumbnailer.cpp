/*
 * Xournal++
 *
 * This small program extracts a preview out of a xoj file
 *
 * @author MarPiRK
 * https://github.com/xournalpp/xournalpp
 *
 * @license GPL
 */

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/locale/format.hpp>
namespace bi = boost::iostreams;
namespace bl = boost::locale;

#include <glibmm/base64.h>

#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#define ENABLE_NLS 0								//it's not yet implemendet, so including it is pointless
#define LOG_MSG_PREFIX "xoj-preview-extractor: "

#define TAG_PREVIEW_NAME "preview"
#define TAG_PAGE_NAME "page"
#define BUF_SIZE 8192

#if ENABLE_NLS
void initLocalisation()
{
	//locale generator (for future i18n)
	bl::generator gen;
	gen.add_messages_path(PACKAGE_LOCALE_DIR);
	gen.add_messages_domain(GETTEXT_PACKAGE);

	std::locale::global(gen("")); //"" - system default locale
	std::cout.imbue(std::locale());
}
#endif //ENABLE_NLS

#define CLOSE		\
	if (gzip)		\
	{				\
		delete in;	\
	}				\
	ifile.close()

int main(int argc, char* argv[])
{

#if ENABLE_NLS
	initLocalisation();
#endif

	//check args count
	if (argc != 3)
	{
		cerr << LOG_MSG_PREFIX "call with INPUT.xoj OUTPUT.png" << endl;
		return 1;
	}

	//check file extensions
	string ext = argv[1] + strlen(argv[1]) - 4;
	for (int i = 0; i < ext.length(); i++)
	{
		if (tolower(ext[i]) != ".xoj"[i])
		{
			cerr << LOG_MSG_PREFIX << bl::format("file \"{1}\" is not .xoj file") % argv[1] << endl;
			return 2;
		}
	}

	ext = argv[2] + strlen(argv[2]) - 4; //last 4 chars
	for (int i = 0; i < ext.length(); i++)
	{
		if (tolower(ext[i]) != ".png"[i])
		{
			cerr << LOG_MSG_PREFIX << bl::format("file \"{1}\" is not .png file") % argv[2] << endl;
			return 2;
		}
	}

	//open input file
	ifstream ifile(argv[1], ifstream::in | ifstream::binary);
	if (!ifile.is_open())
	{
		cerr << LOG_MSG_PREFIX << bl::format("open input file \"{1}\" failed") % argv[1] << endl;
		return 3;
	}

	istream* in;
	bi::filtering_istreambuf inbuf;
	bool gzip;
	
	//check for gzip magic header
	if (ifile.get() == 0x1F && ifile.get() == 0x8B)
	{
		gzip = true;
		ifile.seekg(0); //seek back to beginning
		
		inbuf.push(bi::gzip_decompressor());
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
					cerr << LOG_MSG_PREFIX "this file contains no preview" << endl;
					CLOSE;
					return 5;
				}
				inTag = false;
			}
		}
		else
		{
			if (inTag)
			{
				ofstream ofile(argv[2], ofstream::out | ofstream::binary);
				if (!ofile.is_open())
				{
					cerr << LOG_MSG_PREFIX << bl::format("open output file \"{1}\" failed") % argv[2] << endl;
					CLOSE;
					return 4;
				}
				ofile << Glib::Base64::decode(preview);
				ofile.close();

				cout << LOG_MSG_PREFIX "successfully extracted" << endl;

				CLOSE;
				return 0;
			}
			else
			{
				preview += buf;
			}
		}
	}

	cerr << LOG_MSG_PREFIX "no preview and page found, maybe an invalid file?" << endl;
	CLOSE;
	return 10;
}
