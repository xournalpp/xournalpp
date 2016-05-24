#include "GzHelper.h"

#include "Stacktrace.h"

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
namespace bio = boost::iostreams;

#include <sstream>
using std::stringstream;

GzHelper::GzHelper() { }

GzHelper::~GzHelper() { }

string GzHelper::gzcompress(const string& str, const bio::zlib_params& params)
{	
    stringstream decompressed;
    decompressed << str;
	
    bio::filtering_istreambuf out;
	out.push(bio::zlib_compressor(params));
    out.push(decompressed);
	
    stringstream compressed;
    bio::copy(out, compressed);
    return compressed.str();
}

string GzHelper::gzuncompress(const string& str)
{
    stringstream compressed;
    compressed << str;
    
	bio::filtering_istreambuf in;
    in.push(bio::zlib_decompressor());
    in.push(compressed);
	
    stringstream decompressed;
    bio::copy(in, decompressed);
    return decompressed.str();
}
