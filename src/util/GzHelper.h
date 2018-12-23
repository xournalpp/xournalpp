/*
 * Xournal++
 *
 * Helper functions for ZLib GZip Compression
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <boost/iostreams/filter/gzip.hpp>
namespace bio = boost::iostreams;

#include <string>
using std::string;

class GzHelper
{
private:
	GzHelper();
	virtual ~GzHelper();

public:
	static string gzcompress(const string& str, const bio::zlib_params& = bio::zlib::default_compression);
	static string gzuncompress(const string& str);
};
