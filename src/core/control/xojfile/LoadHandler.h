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

#include <zip.h>  // for zip_t

#include "model/Document.h"         // for Document
#include "model/DocumentHandler.h"  // for DocumentHandler
#include "model/PageRef.h"          // for PageRef
#include "model/Stroke.h"           // for Stroke, StrokeTool,...
#include "util/Color.h"             // for Color
#include "util/PathUtil.h"          // for Util::hash

#include "filesystem.h"  // for path

class AudioElement;
class Image;
class InputStream;
class Layer;
class LineStyle;
class PageType;
class Point;
class TexImage;
class Text;


class LoadHandler {
public:
    LoadHandler();
    virtual ~LoadHandler();

public:
    std::unique_ptr<Document> loadDocument(fs::path const& filepath);

    bool hasErrorMessages() const;
    std::string getErrorMessages() const;
    bool isAttachedPdfMissing() const;
    const fs::path& getMissingPdfFilename() const;

    /** @return The version of the loaded file */
    int getFileVersion() const;

public:
    // interface for XmlParser
    void addXournal(std::string creator, int fileversion);
    void addMrWriter(std::string creator);
    void finalizeDocument();
    void addPage(double width, double height);
    void finalizePage();
    void addAudioAttachment(fs::path filename);
    void setBgName(std::string name);
    void setBgSolid(const PageType& bg, Color color);
    void setBgPixmap(bool attach, const fs::path& filename);
    void setBgPixmapCloned(size_t pageNr);
    void setBgPdf(size_t pageno);
    void loadBgPdf(bool attach, const fs::path& filename);
    void addLayer(const std::optional<std::string>& name);
    void finalizeLayer();
    void addStroke(StrokeTool tool, Color color, double width, int fill, StrokeCapStyle capStyle,
                   const std::optional<LineStyle>& lineStyle, fs::path filename, size_t timestamp);
    void setStrokePoints(std::vector<Point> pointVector, std::vector<double> pressures);
    void finalizeStroke();
    void addText(std::string font, double size, double x, double y, Color color, fs::path filename, size_t timestamp);
    void setTextContents(std::string contents);
    void finalizeText();
    void addImage(double left, double top, double right, double bottom);
    void setImageData(std::string data);
    void setImageAttachment(const fs::path& filename);
    void finalizeImage();
    void addTexImage(double left, double top, double right, double bottom, std::string text);
    void setTexImageData(std::string data);
    void setTexImageAttachment(const fs::path& filename);
    void finalizeTexImage();

private:
    void initAttributes();

    std::unique_ptr<InputStream> openFile(fs::path const& filepath);
    void closeFile() noexcept;
    void parseXml(std::unique_ptr<InputStream> xmlContentStream);

    void fixNullPressureValues(std::vector<double> pressures);

    /**
     * Returns the contents of the zip attachment with the given file name, or
     * nullptr if there is no such file.
     */
    std::unique_ptr<std::string> readZipAttachment(fs::path const& filename);

    void setAudioAttributes(AudioElement& elem, fs::path filename, size_t timestamp);
    fs::path getTempFileForPath(fs::path const& filename);

    /**
     * Get the absolute file path for external files such as background images
     * and PDFs. This function should not be called on the filename of an
     * attachment in the zip file format.
     */
    fs::path getAbsoluteFilepath(const fs::path& filename, bool attach) const;

    void logError(std::string&& error);

private:
    fs::path xournalFilepath;

    bool parsingComplete;
    std::vector<std::string> errorMessages;

    fs::path missingPdf;
    bool attachedPdfMissing;
    bool pdfFilenameParsed;

    std::string creator;
    int fileVersion;
    int minimalFileVersion;

    struct zip_deleter {
        void operator()(zip_t* ptr) { zip_close(ptr); }
    };
    using zip_wrapper = std::unique_ptr<zip_t, zip_deleter>;
    zip_wrapper zipFp;
    bool isGzFile;

    std::vector<PageRef> pages;
    // todo(cpp20): remove the custom hash
    std::unordered_map<fs::path, fs::path, Util::hash<fs::path>> audioFiles;

    PageRef page;
    std::unique_ptr<Layer> layer;
    std::unique_ptr<Stroke> stroke;
    std::unique_ptr<Text> text;
    std::unique_ptr<Image> image;
    std::unique_ptr<TexImage> teximage;

    DocumentHandler dHanlder;
    std::unique_ptr<Document> doc;
};
