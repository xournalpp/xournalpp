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

#include <config.h>
#include <config-paths.h>
#include <i18n.h>
#include <XojPreviewExtractor.h>

#include <boost/locale/format.hpp>
#include <boost/locale/generator.hpp>
namespace bl = boost::locale;

#include <fstream>
using std::ofstream;
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
#include <string>
using std::string;

void initLocalisation()
{
	//locale generator
	bl::generator gen;
#ifdef ENABLE_NLS
	gen.add_messages_path(PACKAGE_LOCALE_DIR);
	gen.add_messages_domain(GETTEXT_PACKAGE);
	
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain(GETTEXT_PACKAGE);
#endif //ENABLE_NLS

	std::locale::global(gen("")); //"" - system default locale
	std::cout.imbue(std::locale());
}

int main(int argc, char* argv[])
{
	initLocalisation();

	//check args count
	if (argc != 3)
	{
		cerr << _("xoj-preview-extractor: call with INPUT.xoj OUTPUT.png") << endl;
		return 1;
	}
	
	//check output file extension
	string ext = argv[2] + strlen(argv[2]) - 4; //last 4 chars
	for (int i = 0; i < ext.length(); i++)
	{
		if (tolower(ext[i]) != ".png"[i])
		{
			cerr << _F("xoj-preview-extractor: file \"{1}\" is not .png file") % argv[2] << endl;
			return 2;
		}
	}
	
	XojPreviewExtractor extractor;
	PreviewExtractResult result = extractor.readFile(argv[1]);

	switch (result)
	{
	case PREVIEW_RESULT_IMAGE_READ:
		// continue to write preview
		break;
		
	case PREVIEW_RESULT_BAD_FILE_EXTENSION:
		cerr << _F("xoj-preview-extractor: file \"{1}\" is not .xoj file") % argv[2] << endl;
		return 2;

	case PREVIEW_RESULT_COULD_NOT_OPEN_FILE:
		cerr << _F("xoj-preview-extractor: opening input file \"{1}\" failed") % argv[1] << endl;
		return 3;

	case PREVIEW_RESULT_NO_PREVIEW:
		cerr << _F("xoj-preview-extractor: file \"{1}\" contains no preview") % argv[1] << endl;
		return 4;

	case PREVIEW_RESULT_ERROR_READING_PREVIEW:
	default:
		cerr << _("xoj-preview-extractor: no preview and page found, maybe an invalid file?") << endl;
		return 5;
	}
	
	ofstream ofile(argv[2], ofstream::out | ofstream::binary);
	if (!ofile.is_open())
	{
		cerr << _F("xoj-preview-extractor: opening output file \"{1}\" failed") % argv[2] << endl;
		return 6;
	}
	ofile << extractor.getData();
	ofile.close();

	cout << _("xoj-preview-extractor: successfully extracted") << endl;
	return 0;
}
