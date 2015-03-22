/*
 * Xournal++
 *
 * Part of the PDF export
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PDFWRITER_H__
#define __PDFWRITER_H__

#include <gtk/gtk.h>
#include <StringUtils.h>
#include "PdfXRef.h"
#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

class PdfWriter
{
public:
    PdfWriter(PdfXRef* xref);
    virtual ~PdfWriter();

public:
    void close();
    bool openFile(path filename);

public:
    bool writeLen(string data, int len);
    bool write(string data);
    bool writef(const char* data, ...);
    bool writeTxt(string data);
    bool write(int data);

    void startStream();
    void endStream();

    bool writeInfo(string title);
    bool writeObj();


    string getLastError();

    int getObjectId();
    int getNextObjectId();
    int getDataCount();

public:
    static void setCompressPdfOutput(bool compress);

private:
    XOJ_TYPE_ATTRIB;

    static bool compressPdfOutput;

    int dataCount;
    bool inStream;
    GString* stream;

    GFileOutputStream* out;

    string lastError;

    PdfXRef* xref;

    int objectId;

};

#endif /* __PDFWRITER_H__ */
