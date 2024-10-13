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
    /**
     * @param errorMessages Either `nullptr` or a pointer to a string to which
     *                      error messages that may be relevant for the user
     *                      will be written. Error messages written to this
     *                      string are also printed as warnings to the console.
     *                      The pointer must remain valid until the LoadHandler
     *                      object is destroyed.
     */
    LoadHandler(std::string* errorMessages = nullptr);
    virtual ~LoadHandler();

public:
    /**
     * Load document located at `filepath`
     * @returns A valid pointer to a `Document`
     * @exception Throws a `std::runtime_error` if a fatal error is encountered.
     */
    std::unique_ptr<Document> loadDocument(fs::path const& filepath);

    /**
     * The attached PDF file was not found.
     * Here "attached" refers to either a file in the zip archive, or a file in
     * the same directory as the document prefixed with the filename of the
     * document.
     */
    bool isAttachedPdfMissing() const;

    /** @return The name of the PDF background in case it was not found */
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
    /** Clear any attributes that may have been used before */
    void initAttributes();

    /**
     * Open a file for reading
     * If the file is a zip file, initializes `zipFp` for access to the other
     * files in the archive.
     * @returns A pointer to an XML input stream, reading either directly from
     *          the gzip file, or from "content.xml" in the zip archive
     * @exception Throws a `std::runtime_error` if the file could not be opened
     *            or required contents could not be found.
     */
    std::unique_ptr<InputStream> openFile(fs::path const& filepath);
    /** Reset `zipFp`, closing the zip archive if it is open. */
    void closeFile() noexcept;
    /**
     * Parse the contents of `xmlContentStream`
     * The document will get built during this call.
     * @param xmlContentStream An InputStream reading from the XML file. It is
     *                         "consumed" and destroyed at function exit.
     * @exception Throws a `std::runtime_error` if the document is corrupted and
     *            cannot be loaded.
     */
    void parseXml(std::unique_ptr<InputStream> xmlContentStream);

    /**
     * Remove points of the current `stroke` that have an invalid pressure.
     * Splits up the stroke into valid segments and adds them to the layer,
     * except for the last one, which is left in `stroke`. If no pressure points
     * are valid, the stroke is removed entirely and `stroke` is reset.
     */
    void fixNullPressureValues(std::vector<double> pressures);

    /**
     * Returns the contents of the zip attachment with the given file name, or
     * nullptr if there is no such file.
     */
    std::unique_ptr<std::string> readZipAttachment(fs::path const& filename);

    /** Set audio attributes for `elem`, for any file type and file version. */
    void setAudioAttributes(AudioElement& elem, fs::path filename, size_t timestamp);

    /** @return The path of a temporary file extracted from the zip archive. */
    fs::path getTempFileForPath(fs::path const& filename);

    /**
     * Get the absolute file path for external files such as background images
     * and PDFs. This function should not be called on the filename of an
     * attachment in the zip file format.
     */
    fs::path getAbsoluteFilepath(const fs::path& filename, bool attach) const;

    /**
     * Store an error for retrieval through `getErrorMessages()` and print it
     * to the console as a warning.
     */
    void logError(std::string error);

private:
    fs::path xournalFilepath;

    bool parsingComplete;
    std::string* errorMessages;

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
