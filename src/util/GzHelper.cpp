#include "GzHelper.h"
#include "Stacktrace.h"

#include <iostream>
#include <sstream>
using namespace std;

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
namespace bio = boost::iostreams;

GzHelper::GzHelper() { }

GzHelper::~GzHelper() { }

string GzHelper::gzcompress(const string& str, int level)
{
	if ((level < -1) || (level > 9))
	{
		cerr << bl::format("GzHelper::gzcompress compression level ({1}) must be within -1..9")
							%level << endl;
		Stacktrace::printStracktrace();
		level = -1;
	}
	
    std::stringstream compressed;
    std::stringstream decompressed;
    decompressed << str;
    boost::iostreams::filtering_istreambuf out;
	
	bio::zlib_params params = bio::zlib::default_compression;
	if (level != -1) params.level = level;
	
	out.push(bio::zlib_compressor(params));
    out.push(decompressed);
    bio::copy(out, compressed);
    return compressed.str();
}

string GzHelper::gzuncompress(const string& str)
{
    std::stringstream compressed;
    std::stringstream decompressed;
    compressed << str;
    bio::filtering_istreambuf in;
    in.push(bio::zlib_decompressor());
    in.push(compressed);
    bio::copy(in, decompressed);
    return decompressed.str();
}
