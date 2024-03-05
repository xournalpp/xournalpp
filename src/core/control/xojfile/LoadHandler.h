/*
 * Xournal++
 *
 * Loads a .xoj / .xopp document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>        // for size_t
#include <memory>         // for unique_ptr
#include <optional>       // for optional
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <vector>         // for vector

#include <glib.h>     // for gchar, GError, gsize, GMarkupPars...
#include <zip.h>      // for zip_file_t, zip_t
#include <zipconf.h>  // for zip_int64_t, zip_uint64_t
#include <zlib.h>     // for gzFile

#include "model/Document.h"         // for Document
#include "model/DocumentHandler.h"  // for DocumentHandler
#include "model/PageRef.h"          // for PageRef
#include "model/PageType.h"         // for PageType
#include "model/Stroke.h"           // for Stroke, StrokeTool,...
#include "util/Color.h"             // for Color

#include "LoadHandlerHelper.h"
#include "filesystem.h"  // for path

class Image;
class Layer;
class LineStyle;
class Point;
class TexImage;
class Text;


enum ParserPosition {
    PARSER_POS_NOT_STARTED = 1,  // Waiting for opening <xounal> tag
    PARSER_POS_STARTED,          // Waiting for Metainfo or contents like <page>
    PARSER_POS_IN_PAGE,          // Starting page tag read
    PARSER_POS_IN_LAYER,         // Starting layer tag read
    PARSER_POS_IN_STROKE,        // Starting layer tag read
    PARSER_POS_IN_TEXT,          // Starting text tag read
    PARSER_POS_IN_IMAGE,         // Starting image tag read
    PARSER_POS_IN_TEXIMAGE,      // Starting latex tag read

    PASER_POS_FINISHED  // Document is parsed
};

class LoadHandler {
public:
    LoadHandler();
    virtual ~LoadHandler();

public:
    std::unique_ptr<Document> loadDocument(fs::path const& filepath);

    std::string getLastError();
    bool isAttachedPdfMissing() const;
    std::string getMissingPdfFilename() const;

    /** @return The version of the loaded file */
    int getFileVersion() const;

public:
    // interface for XmlParser
    void addXournal(const std::string& creator, int fileversion);
    void addMrWriter(const std::string& creator);
    void finalizeDocument();
    void addPage(double width, double height);
    void finalizePage();
    void addAudioAttachment(const fs::path& filename);
    void addBackground(const std::optional<std::string>& name);
    void setBgSolid(const PageType& bg, const Color& color);
    void setBgPixmap(bool attach, const fs::path& filename);
    void setBgPixmapCloned(size_t pageNr);
    void setBgPdf(size_t pageno);
    void loadBgPdf(bool attach, const fs::path& filename);
    void addLayer(const std::optional<std::string>& name);
    void finalizeLayer();
    void addStroke(StrokeTool tool, const Color& color, double width, int fill, StrokeCapStyle capStyle,
                   const std::optional<LineStyle>& lineStyle, const fs::path& filename, size_t timestamp);
    void setStrokePoints(std::vector<Point>&& pointVector, std::vector<double> pressures);
    void finalizeStroke();
    void addText(const std::string& font, double size, double x, double y, const Color& color, const fs::path& filename,
                 size_t timestamp);
    void setTextContents(const std::string& contents);
    void finalizeText();
    void addImage(double left, double top, double right, double bottom);
    void setImageData(std::string&& data);
    void setImageAttachment(const fs::path& filename);
    void finalizeImage();
    void addTexImage(double left, double top, double right, double bottom, const std::string& text);
    void setTexImageData(std::string&& data);
    void setTexImageAttachment(const fs::path& filename);
    void finalizeTexImage();

private:
    void parseStart();
    void parseContents();
    void parsePage();
    void parseLayer();
    void parseAudio();

    void parseStroke();
    void parseText();
    void parseImage();
    void parseTexImage();

private:
    void initAttributes();

    std::string readLine();
    zip_int64_t readContentFile(char* buffer, zip_uint64_t len);
    bool closeFile();
    bool openFile(fs::path const& filepath);
    bool parseXml();

    void fixNullPressureValues(std::vector<double> pressures);
    static void parserText(GMarkupParseContext* context, const gchar* text, gsize textLen, gpointer userdata,
                           GError** error);
    static void parserEndElement(GMarkupParseContext* context, const gchar* elementName, gpointer userdata,
                                 GError** error);
    static void parserStartElement(GMarkupParseContext* context, const gchar* elementName, const gchar** attributeNames,
                                   const gchar** attributeValues, gpointer userdata, GError** error);

    const char* getAttrib(const char* name, bool optional = false);
    double getAttribDouble(const char* name);
    int getAttribInt(const char* name);

    void parseBgSolid();
    void parseBgPixmap();
    void parseBgPdf();
    void parseAttachment();

    void readImage(const gchar* base64string, gsize base64stringLen);
    void readTexImage(const gchar* base64string, gsize base64stringLen);

private:
    static std::string parseBase64(const gchar* base64, gsize length);

    /**
     * Returns the contents of the zip attachment with the given file name, or
     * nullopt if there is no such file.
     */
    std::unique_ptr<std::string> readZipAttachment(fs::path const& filename);

    void setAudioAttributes(AudioElement& elem, const fs::path& filename, size_t timestamp);
    fs::path getTempFileForPath(fs::path const& filename);

private:
    std::string lastError;
    std::string pdfMissing;
    bool attachedPdfMissing;

    fs::path filepath;

    bool pdfFilenameParsed;

    ParserPosition pos;

    std::string creator;
    int fileVersion;
    int minimalFileVersion;

    zip_t* zipFp;
    zip_file_t* zipContentFile;
    gzFile gzFp;
    bool isGzFile = false;

    std::vector<double> pressureBuffer;

    std::vector<PageRef> pages;
    PageRef page;
    std::unique_ptr<Layer> layer;
    std::unique_ptr<Stroke> stroke;
    std::unique_ptr<Text> text;
    std::unique_ptr<Image> image;
    std::unique_ptr<TexImage> teximage;
    std::unordered_map<std::string, fs::path> audioFiles;

    const char* endRootTag = "xournal";

    fs::path xournalFilepath;

    GError* error;
    const gchar** attributeNames;
    const gchar** attributeValues;
    const gchar* elementName;

    int loadedTimeStamp;
    std::string loadedFilename;

    DocumentHandler dHanlder;
    std::unique_ptr<Document> doc;

    friend Color LoadHandlerHelper::parseBackgroundColor(LoadHandler* loadHandler);
    friend bool LoadHandlerHelper::parseColor(const char* text, Color& color, LoadHandler* loadHandler);

    friend const char* LoadHandlerHelper::getAttrib(const char* name, bool optional, LoadHandler* loadHandler);
    friend double LoadHandlerHelper::getAttribDouble(const char* name, LoadHandler* loadHandler);
    friend int LoadHandlerHelper::getAttribInt(const char* name, LoadHandler* loadHandler);
    friend bool LoadHandlerHelper::getAttribInt(const char* name, bool optional, LoadHandler* loadHandler, int& rValue);
    friend size_t LoadHandlerHelper::getAttribSizeT(const char* name, LoadHandler* loadHandler);
    friend bool LoadHandlerHelper::getAttribSizeT(const char* name, bool optional, LoadHandler* loadHandler,
                                                  size_t& rValue);
};
