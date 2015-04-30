#include "GzHelper.h"

#include "Stacktrace.h"

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

#include <iostream>
#include <sstream>
using namespace std;

GzHelper::GzHelper() { }

GzHelper::~GzHelper() { }

string GzHelper::gzcompress(const string& str, const bio::zlib_params& params)
{	
    std::stringstream compressed;
    std::stringstream decompressed;
    decompressed << str;
    boost::iostreams::filtering_istreambuf out;
	
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
