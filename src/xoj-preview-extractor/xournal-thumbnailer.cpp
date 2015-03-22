/*
 * Xournal++
 *
 * This small programm extracts a preview out of a xournal file
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#include <string>
#include <iostream>
#include <fstream>

#include <boost/locale.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/algorithm/string.hpp>

#include "../../config.h"

namespace bl = boost::locale;
namespace bi = boost::iostreams;
namespace ba = boost::algorithm;
using namespace std;

const char BASE64_TABLE[256] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52,
    53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1
};

#ifdef ENABLE_NLS
void initLocalisation()
{
    //locale generator (for future i18n)
    bl::generator gen;
    gen.add_messages_path(PACKAGE_LOCALE_DIR);
    gen.add_messages_domain(GETTEXT_PACKAGE);
    
    std::locale::global(gen("")); //"" - system default locale
    std::cout.imbue(std::locale());
}
#endif

const string TAG_PREVIEW_NAME = "preview";
const string TAG_PAGE_NAME = "page";

int main(int argc, char* argv[]) {
    
#ifdef ENABLE_NLS
    initLocalisation();
#endif

    //check args count
    if (argc != 3) {
        cerr << "xoj-preview-extractor: call with INPUT.xoj OUTPUT.png" << endl;
        return 1;
    }

    //check input file extension
    if (!ba::ends_with(bl::to_lower(argv[1]), ".xoj")) {
        cerr << bl::format("xoj-prevew-extractor: file \"{1}\" is not .xoj file") % argv[1] << endl;
        return 2;
    }
    
    ifstream ifile(argv[1], ios_base::in | ios_base::binary);
    ofstream ofile;
    ofstream dbgfile("dbg", ios_base::out | ios_base::binary);

    if (!ifile.is_open()) {
        cerr << bl::format("xoj-preview-extractor: open input file \"{1}\" failed") % argv[1] << endl;
        return 3;
    }
        
    bi::filtering_istreambuf inbuf;
    inbuf.push(bi::gzip_decompressor());
    inbuf.push(ifile);
    
    istream in(&inbuf);
    char c;
    string tmpTag;
    bool tmpTagAct = false;
    
    char buf[4];
    int bufPos = 0;
    
    while(in.get(c)) {
        //cout << c;
        if (tmpTagAct) {
            tmpTag += c;
            
            if (tmpTag == TAG_PREVIEW_NAME) {
                tmpTagAct = false;
                ofile.open(argv[2], ios_base::out | ios_base::binary);
                if (!ofile.is_open()) {
                    cerr << bl::format("xoj-preview-extractor: open output file \"{1}\" failed") % argv[2] << endl;
                    ifile.close();
                    return 4;
                }
            } else if (tmpTag == TAG_PAGE_NAME) {
                tmpTagAct = false;
                cerr << "xoj-preview-extractor: this file contains no preview" << endl;
                ifile.close();
                return 5;
            } else if (c == '>') {
                tmpTagAct = false;
            }
        } else if (c == '<') {
            if (ofile.is_open()) {
                cout << "xoj-preview-extractor: successfully extracted" << endl;
                ifile.close();
                ofile.close();
                dbgfile.close();
                return 0;
            }
            
            tmpTag.clear();
            tmpTagAct = true;
        } else if (ofile.is_open()) {
            dbgfile << BASE64_TABLE[c];
            
            if (BASE64_TABLE[c] != -1) {
                buf[bufPos++] = BASE64_TABLE[c];
            }
            
            if (bufPos == 4) {
                ofile << char((buf[0] << 2) + ((buf[1] & 0x30) >> 4));
                ofile << char(((buf[1] & 0xf) << 4) + ((buf[2] & 0x3c) >> 2));
                ofile << char(((buf[2] & 0x3) << 6) + buf[3]);
                bufPos = 0;
            }
        }
    }
    
    cerr << "xoj-preview-extractor: no preview and page found, maybe an invalid file?" << endl;
    ifile.close();
    return 10;
}

/*
    int count;
    unsigned char buffer[512];

    char inBuffer[4];
    
    int pa = 0;
    int pr = 0;
    int i = 0;
    int state = 0;
    int bufPos = 0;
    
    do {
        count = gzread(f, buffer, sizeof (buffer));

        i = 0;

        if (state == 0) {
            for (; i < count; i++) {
                if (TAG_PREVIEW[pr] == buffer[i]) {
                    pr++;
                    if (TAG_PREVIEW[pr] == 0) {
                        // now we are in preview data
                        state = 1;
                        i++;
                        fp = fopen(argv[2], "wb");
                        if (!fp) {
                            cerr << bl::format("xoj-preview-extractor: open output file \"{1}\"") % argv[2] << endl;
                            file.close();
                            return 3;
                        }
                        break;
                    }
                } else {
                    pr = 0;
                }

                if (TAG_PAGE[pa] == buffer[i]) {
                    pa++;
                    if (TAG_PAGE[pa] == 0) {
                        cerr << "xoj-preview-extractor: this file contains no preview" << endl;
                        file.close();
                        return 5;
                    }
                } else {
                    pa = 0;
                }
            }
        }

        for (; i < count; i++) {
            if (buffer[i] == '<') {
                file.close();
                cout << "xoj-preview-extractor: successfully extracted" << endl;
                return 0;
            }

            inBuffer[bufPos++] = buffer[i];
            if (bufPos == 4) {
                for (x = 0; x < 4; x++) {
                    inBuffer[x] = BASE64_TABLE[inBuffer[x]];
                }

                fputc((char) ((inBuffer[0] << 2) + ((inBuffer[1] & 0x30) >> 4)), fp);
                fputc((char) (((inBuffer[1] & 0xf) << 4) + ((inBuffer[2] & 0x3c) >> 2)), fp);
                fputc((char) (((inBuffer[2] & 0x3) << 6) + inBuffer[3]), fp);

                bufPos = 0;
            }
        }

    } while (count);

    file.close();

    cerr << "xoj-preview-extractor: no preview found, may an invalid file?" << endl;
    return 10;
}*/
