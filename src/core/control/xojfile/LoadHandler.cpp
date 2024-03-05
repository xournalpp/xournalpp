#include "LoadHandler.h"

#include <algorithm>      // for copy
#include <cmath>          // for isnan
#include <cstdlib>        // for atoi, size_t
#include <cstring>        // for strcmp, strlen
#include <ctime>          // for clock
#include <exception>      // for exception
#include <iterator>       // for back_inserter
#include <memory>         // for make_unique, make_shared...
#include <optional>       // for optional
#include <regex>          // for regex_search, smatch
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <utility>        // for move, forward
#include <vector>         // for vector

#include <gio/gio.h>      // for g_file_get_path, g_fil...
#include <glib.h>         // for g_message...
#include <glibconfig.h>   // for gssize...
#include <zip.h>          // for zip_file_t, zip_fopen,...
#include <zipconf.h>      // for zip_int64_t, zip_uint64_t

#include "control/pagetype/PageTypeHandler.h"   // for PageTypeHandler
#include "control/xojfile/GzInputStream.h"      // for GzInputStream
#include "control/xojfile/LoadHandlerHelper.h"  // for getAttrib, getAttribDo...
#include "control/xojfile/XmlParser.h"          // for XmlParser
#include "model/BackgroundImage.h"              // for BackgroundImage
#include "model/Document.h"                     // for Document
#include "model/Font.h"                         // for XojFont
#include "model/Image.h"                        // for Image
#include "model/Layer.h"                        // for Layer
#include "model/PageType.h"                     // for PageType, PageTypeFormat
#include "model/Point.h"                        // for Point
#include "model/Stroke.h"                       // for Stroke, StrokeCapStyle
#include "model/StrokeStyle.h"                  // for StrokeStyle
#include "model/TexImage.h"                     // for TexImage
#include "model/Text.h"                         // for Text
#include "model/XojPage.h"                      // for XojPage
#include "util/Assert.h"                        // for xoj_assert
#include "util/Color.h"                         // for Color
#include "util/GzUtil.h"                        // for GzUtil
#include "util/LoopUtil.h"
#include "util/PlaceholderString.h"    // for PlaceholderString
#include "util/i18n.h"                 // for _F, FC, FS, _
#include "util/raii/CLibrariesSPtr.h"  // for adopt
#include "util/raii/GObjectSPtr.h"
#include "util/safe_casts.h"  // for as_signed, as_unsigned

#include "filesystem.h"  // for path

using std::string;

#define error2(var, ...)                                                                \
    if (var == nullptr) {                                                               \
        var = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__); \
    }

#define error(...)                                                                        \
    if (error == nullptr) {                                                               \
        error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__); \
    }

namespace {
constexpr size_t MAX_VERSION_LENGTH = 50;
constexpr size_t MAX_MIMETYPE_LENGTH = 25;
}  // namespace

LoadHandler::LoadHandler():
        attachedPdfMissing(false),
        pdfFilenameParsed(false),
        pos(PARSER_POS_NOT_STARTED),
        fileVersion(0),
        minimalFileVersion(0),
        zipFp(nullptr),
        zipContentFile(nullptr),
        gzFp(nullptr),
        attributeNames(nullptr),
        attributeValues(nullptr),
        elementName(nullptr),
        loadedTimeStamp(0) {
    this->error = nullptr;
}

LoadHandler::~LoadHandler() = default;

void LoadHandler::initAttributes() {
    this->zipFp = nullptr;
    this->zipContentFile = nullptr;
    this->gzFp = nullptr;
    this->isGzFile = false;
    this->error = nullptr;
    this->attributeNames = nullptr;
    this->attributeValues = nullptr;
    this->elementName = nullptr;
    this->pdfFilenameParsed = false;
    this->attachedPdfMissing = false;

    this->page.reset();
    this->layer.reset();
    this->stroke.reset();
    this->image.reset();
    this->teximage.reset();
    this->text.reset();

    this->pages.clear();
    this->audioFiles.clear();
}

auto LoadHandler::getLastError() -> string { return this->lastError; }

auto LoadHandler::isAttachedPdfMissing() const -> bool { return this->attachedPdfMissing; }

auto LoadHandler::getMissingPdfFilename() const -> string { return this->pdfMissing; }


void LoadHandler::addXournal(const std::string& creator, int fileversion) {
    this->creator = creator;
    this->fileVersion = fileversion;
}

void LoadHandler::addMrWriter(const std::string& creator) {
    this->creator = creator;
    this->fileVersion = 1;
}

void LoadHandler::finalizeDocument() {
    this->doc->addPages(this->pages.begin(), this->pages.end());
    this->pages.clear();
}

void LoadHandler::addPage(double width, double height) {
    this->page = std::make_shared<XojPage>(width, height, /*suppressLayerCreation*/ true);
    this->pages.emplace_back(this->page);
}

void LoadHandler::finalizePage() {
    // handle unnecessary layer insertion in case of existing layers in file
    if (this->page->getLayerCount() == 0) {
        this->page->addLayer(new Layer());
    }
    // this->pages already holds a reference to the page
    this->page.reset();
}

void LoadHandler::addAudioAttachment(const fs::path& filename) {
    // Verify if attachment exists and is valid
    zip_stat_t attachmentFileStat;
    const int statStatus = zip_stat(this->zipFp, filename.string().c_str(), 0, &attachmentFileStat);
    if (statStatus != 0) {
        g_warning("Could not open attachment: %s. Error message: %s", filename.string().c_str(),
                  zip_error_strerror(zip_get_error(this->zipFp)));
        return;
    }

    gsize length = 0;
    if (attachmentFileStat.valid & ZIP_STAT_SIZE) {
        length = attachmentFileStat.size;
    } else {
        g_warning("Could not open attachment: %s. Error message: No valid file size provided",
                  filename.string().c_str());
        return;
    }

    zip_file_t* attachmentFile = zip_fopen(this->zipFp, filename.string().c_str(), 0);

    if (!attachmentFile) {
        g_warning("Could not open attachment: %s. Error message: %s", filename.string().c_str(),
                  zip_error_strerror(zip_get_error(this->zipFp)));
        return;
    }

    // Extract audio to temporary file
    GFileIOStream* fileStream = nullptr;
    const xoj::util::GObjectSPtr<GFile> tmpFile(g_file_new_tmp("xournal_audio_XXXXXX.tmp", &fileStream, nullptr),
                                                xoj::util::adopt);
    if (!tmpFile) {
        g_warning("Unable to create temporary file for audio attachment.");
        return;
    }

    GOutputStream* outputStream = g_io_stream_get_output_stream(G_IO_STREAM(fileStream));

    gpointer data = g_malloc(1024);
    zip_uint64_t readBytes = 0;
    while (readBytes < length) {
        const zip_int64_t read = zip_fread(attachmentFile, data, 1024);
        if (read == -1) {
            g_free(data);
            zip_fclose(attachmentFile);
            g_warning("Could not open attachment: %s. Error message: Could not read file", filename.string().c_str());
            return;
        }

        const gboolean writeSuccessful =
                g_output_stream_write_all(outputStream, data, static_cast<gsize>(read), nullptr, nullptr, nullptr);
        if (!writeSuccessful) {
            g_free(data);
            zip_fclose(attachmentFile);
            g_warning("Could not open attachment: %s. Error message: Could not write temporary file",
                      filename.string().c_str());
            return;
        }

        readBytes += static_cast<zip_uint64_t>(read);
    }
    g_free(data);
    zip_fclose(attachmentFile);

    // Map the filename to the extracted temporary filepath
    char* tmpPath = g_file_get_path(tmpFile.get());
    // fs::path does not have a std::hash specialization on all compilers
    this->audioFiles[filename.string()] = tmpPath;
    g_free(tmpPath);
}

void LoadHandler::addBackground(const std::optional<std::string>& name) {
    if (name) {
        this->page->setBackgroundName(*name);
    }
}

void LoadHandler::setBgSolid(const PageType& bg, const Color& color) {
    this->page->setBackgroundType(bg);
    this->page->setBackgroundColor(color);
}

void LoadHandler::setBgPixmap(bool attach, const fs::path& filename) {
    BackgroundImage img;

    if (!attach || (attach && this->isGzFile)) {
        fs::path fileToLoad;
        if (attach) {
            fileToLoad = this->filepath;
            fileToLoad += ".";
            fileToLoad += filename;
        } else {
            fileToLoad = filename;
        }

        GError* error = nullptr;
        img.loadFile(fileToLoad, &error);

        if (error) {
            g_warning("Could not read image: %s. Error message: %s", fileToLoad.string().c_str(), error->message);
            g_error_free(error);
        }
    } else {
        // This is the zip file attach domain
        const auto readResult = readZipAttachment(filename);  ///< Do not remove the const qualifier - see below
        if (!readResult) {
            return;
        }
        const std::string& imgData = *readResult;  ///< Do not remove the const qualifier - see below

        /**
         * To avoid an unecessary copy, the data is still managed by the std::unique_ptr<std::string> instance. The
         * input stream assumes the data will not be modified: do not remove the const qualifier on readResult or
         * imgData
         */
        const xoj::util::GObjectSPtr<GInputStream> inputStream(
                g_memory_input_stream_new_from_data(imgData.data(), static_cast<gssize>(imgData.size()), nullptr),
                xoj::util::adopt);

        GError* error = nullptr;
        img.loadFile(inputStream.get(), filename, &error);

        g_input_stream_close(inputStream.get(), nullptr, nullptr);

        if (error) {
            g_warning("Could not read image: %s. Error message: %s", filename.string().c_str(), error->message);
            g_error_free(error);
        }
    }
    this->page->setBackgroundImage(img);
    this->page->setBackgroundType(PageType(PageTypeFormat::Image));
}

void LoadHandler::setBgPixmapCloned(size_t pageNr) {
    if (pageNr >= this->doc->getPageCount()) {
        g_warning("Cloned pixmap page background on page %ld references inexistent page %ld. Skipping",
                  this->doc->getPageCount(), pageNr);
        return;
    }

    auto p = this->doc->getPage(pageNr);
    if (p) {
        this->page->setBackgroundImage(p->getBackgroundImage());
    }
    this->page->setBackgroundType(PageType(PageTypeFormat::Image));
}

void LoadHandler::setBgPdf(size_t pageno) { this->page->setBackgroundPdfPageNr(pageno); }

void LoadHandler::loadBgPdf(bool attach, const fs::path& filename) {
    if (this->isGzFile || !attach) {
        // Resolve correct file path
        fs::path pdfFilepath;
        if (attach) {
            // For a file xyz.xopp, the PDF background is saved in the same
            // directory as xyz.xopp.[filename]
            pdfFilepath = (fs::path(this->xournalFilepath) += ".") += (filename);
        } else {
            // domain = "absolute" doesn't forcefully mean an absolute path
            if (filename.is_relative()) {
                // The path is relative to the .xopp file's location
                pdfFilepath = fs::path(this->xournalFilepath).remove_filename() / filename;
            } else {
                pdfFilepath = filename;
            }
        }

        // pdfFilepath should point to the background PDF file
        if (fs::is_regular_file(pdfFilepath)) {
            doc->readPdf(pdfFilepath, false, attach);

            if (!doc->getLastErrorMsg().empty()) {
                g_warning("Error reading PDF: %s", doc->getLastErrorMsg().c_str());
            }
        } else {
            // Even if loading the PDF failed, tell the document what should
            // have been loaded so the background is not silently removed on save
            doc->setPdfAttributes(pdfFilepath, attach);
            if (attach) {
                this->attachedPdfMissing = true;
            } else {
                this->pdfMissing = pdfFilepath.u8string();
            }
        }
    } else {
        // filename should point to an attachment inside the zip archive
        auto pdfBytes = readZipAttachment(filename);
        if (pdfBytes) {
            doc->readPdf(filename, false, attach, std::move(pdfBytes));

            if (!doc->getLastErrorMsg().empty()) {
                g_warning("Error reading PDF: %s", doc->getLastErrorMsg().c_str());
            }
        } else {
            doc->setPdfAttributes(filename, true);
            this->attachedPdfMissing = true;
        }
    }
    this->pdfFilenameParsed = true;
}

void LoadHandler::addLayer(const std::optional<std::string>& name) {
    xoj_assert(!this->layer);
    this->layer = std::make_unique<Layer>();

    if (name) {
        this->layer->setName(*name);
    }
}

void LoadHandler::finalizeLayer() {
    xoj_assert(this->layer);
    this->page->addLayer(this->layer.release());
}

void LoadHandler::addStroke(StrokeTool tool, const Color& color, double width, int fill, StrokeCapStyle capStyle,
                            const std::optional<LineStyle>& lineStyle, const fs::path& filename, size_t timestamp) {
    xoj_assert(!this->stroke);
    this->stroke = std::make_unique<Stroke>();

    this->stroke->setToolType(tool);
    this->stroke->setColor(color);
    this->stroke->setWidth(width);
    this->stroke->setFill(fill);
    this->stroke->setStrokeCapStyle(capStyle);

    if (lineStyle) {
        this->stroke->setLineStyle(*lineStyle);
    }

    setAudioAttributes(*this->stroke, filename, timestamp);
}

void LoadHandler::setStrokePoints(std::vector<Point>&& pointVector, std::vector<double> pressures) {
    xoj_assert(this->stroke);

    if (pointVector.size() < 2) {
        g_warning("LoadHandler: Ignoring stroke with less than two points");
        return;
    }
    this->stroke->setPointVector(std::move(pointVector));

    if (!pressures.empty()) {
        if (pressures.size() + 1 >= this->stroke->getPointCount()) {
            if (!std::all_of(pressures.begin(), pressures.end(), [](double pressure) { return pressure > 0; })) {
                // Warning: this may delete this->stroke if no positive pressure values are provided
                // Do not dereference this->stroke after that
                fixNullPressureValues(std::move(pressures));
            } else {
                this->stroke->setPressure(pressures);
            }
        } else {
            g_warning("LoadHandler: Wrong number of pressure values: got %ld, expected %ld", pressures.size(),
                      (this->stroke->getPointCount() - 1));
        }
    }
}

void LoadHandler::finalizeStroke() {
    // fixNullPressureValues() may have deleted the stroke. Check if it still
    // exists before accessing it.
    if (this->stroke) {
        this->layer->addElement(std::move(this->stroke));
    }
}

void LoadHandler::addText(const std::string& font, double size, double x, double y, const Color& color,
                          const fs::path& filename, size_t timestamp) {
    xoj_assert(!this->text);
    this->text = std::make_unique<Text>();

    XojFont& f = this->text->getFont();
    f.setName(font);
    f.setSize(size);
    this->text->setX(x);
    this->text->setY(y);
    this->text->setColor(color);

    setAudioAttributes(*this->text, filename, timestamp);
}

void LoadHandler::setTextContents(const std::string& contents) {
    xoj_assert(this->text);

    this->text->setText(contents);
}

void LoadHandler::finalizeText() {
    xoj_assert(this->text);

    this->layer->addElement(std::move(this->text));
}

void LoadHandler::addImage(double left, double top, double right, double bottom) {
    xoj_assert(!this->image);
    this->image = std::make_unique<Image>();

    this->image->setX(left);
    this->image->setY(top);
    this->image->setWidth(right - left);
    this->image->setHeight(bottom - top);
}

void LoadHandler::setImageData(std::string&& data) {
    xoj_assert(this->image);

    this->image->setImage(std::move(data));
}

void LoadHandler::setImageAttachment(const fs::path& filename) {
    xoj_assert(this->image);

    auto imageData = readZipAttachment(filename);
    if (imageData) {
        this->image->setImage(std::move(*imageData));
    }
}

void LoadHandler::finalizeImage() {
    xoj_assert(this->image);

    this->layer->addElement(std::move(this->image));
}

void LoadHandler::addTexImage(double left, double top, double right, double bottom, const std::string& text) {
    xoj_assert(!this->teximage);
    this->teximage = std::make_unique<TexImage>();

    this->teximage->setX(left);
    this->teximage->setY(top);
    this->teximage->setWidth(right - left);
    this->teximage->setHeight(bottom - top);

    this->teximage->setText(text);
}

void LoadHandler::setTexImageData(std::string&& data) {
    xoj_assert(this->teximage);

    this->teximage->loadData(std::move(data));
}

void LoadHandler::finalizeTexImage() {
    xoj_assert(this->teximage);

    this->layer->addElement(std::move(this->teximage));
}

void LoadHandler::setTexImageAttachment(const fs::path& filename) {
    xoj_assert(this->teximage);

    auto imageData = readZipAttachment(filename);
    if (imageData) {
        this->teximage->loadData(std::move(*imageData));
    }
}


auto LoadHandler::openFile(fs::path const& filepath) -> bool {
    this->filepath = filepath;
    int zipError = 0;
    this->zipFp = zip_open(filepath.u8string().c_str(), ZIP_RDONLY, &zipError);

    // Check if the file is actually an old XOPP-File and open it
    if (!this->zipFp && zipError == ZIP_ER_NOZIP) {
        this->gzFp = GzUtil::openPath(filepath, "r");
        this->isGzFile = true;

        // Trying out the new XML parser
        try {
            GzInputStream input(filepath);
            const auto start = clock();
            XmlParser parser(input, this);
            parser.parse();
            const auto stop = clock();
            g_message("Time taken by new parser: %ld clocks", (stop - start));
            this->pages.clear();
        } catch (const std::exception& e) {
            g_error("An error occured during file parsing: %s", e.what());
        }
    }

    if (this->zipFp && !this->isGzFile) {
        // Check the mimetype
        zip_file_t* mimetypeFp = zip_fopen(this->zipFp, "mimetype", 0);
        if (!mimetypeFp) {
            this->lastError = zip_error_strerror(zip_get_error(zipFp));
            this->lastError =
                    FS(_F("The file is no valid .xopp file (Mimetype missing): \"{1}\"") % filepath.u8string());
            return false;
        }
        char mimetype[MAX_MIMETYPE_LENGTH + 1] = {};
        // read the mimetype and a few more bytes to make sure we do not only read a subset
        zip_fread(mimetypeFp, mimetype, MAX_MIMETYPE_LENGTH);
        if (!strcmp(mimetype, "application/xournal++")) {
            zip_fclose(mimetypeFp);
            this->lastError = FS(_F("The file is no valid .xopp file (Mimetype wrong): \"{1}\"") % filepath.u8string());
            return false;
        }
        zip_fclose(mimetypeFp);

        // Get the file version
        zip_file_t* versionFp = zip_fopen(this->zipFp, "META-INF/version", 0);
        if (!versionFp) {
            this->lastError =
                    FS(_F("The file is no valid .xopp file (Version missing): \"{1}\"") % filepath.u8string());
            return false;
        }
        char versionString[MAX_VERSION_LENGTH + 1] = {};
        zip_fread(versionFp, versionString, MAX_VERSION_LENGTH);
        std::string versions(versionString);
        std::regex versionRegex("current=(\\d+?)(?:\n|\r\n)min=(\\d+?)");
        std::smatch match;
        if (std::regex_search(versions, match, versionRegex)) {
            this->fileVersion = std::stoi(match.str(1));
            this->minimalFileVersion = std::stoi(match.str(2));
        } else {
            zip_fclose(versionFp);
            this->lastError = FS(_F("The file is not a valid .xopp file (Version string corrupted): \"{1}\"") %
                                 filepath.u8string());
            return false;
        }
        zip_fclose(versionFp);

        // open the main content file
        this->zipContentFile = zip_fopen(this->zipFp, "content.xml", 0);
        if (!this->zipContentFile) {
            this->lastError = FS(_F("Failed to open content.xml in zip archive: \"{1}\"") %
                                 zip_error_strerror(zip_get_error(zipFp)));
            return false;
        }
    }

    // Fail if neither utility could open the file
    if (!this->zipFp && !this->gzFp) {
        this->lastError = FS(_F("Could not open file: \"{1}\"") % filepath.u8string());
        return false;
    }
    return true;
}

auto LoadHandler::closeFile() -> bool {
    if (this->isGzFile) {
        return static_cast<bool>(gzclose(this->gzFp));
    }

    xoj_assert(this->zipContentFile != nullptr);
    zip_fclose(this->zipContentFile);
    int zipError = zip_close(this->zipFp);
    return zipError == 0;
}

auto LoadHandler::readContentFile(char* buffer, zip_uint64_t len) -> zip_int64_t {
    if (this->isGzFile) {
        if (gzeof(this->gzFp)) {
            return -1;
        }
        return gzread(this->gzFp, buffer, static_cast<unsigned int>(len));
    }

    xoj_assert(this->zipContentFile != nullptr);
    zip_int64_t lengthRead = zip_fread(this->zipContentFile, buffer, len);
    if (lengthRead > 0) {
        return lengthRead;
    }

    return -1;
}

auto LoadHandler::parseXml() -> bool {
    xoj_assert(this->doc);
    const GMarkupParser parser = {LoadHandler::parserStartElement, LoadHandler::parserEndElement,
                                  LoadHandler::parserText, nullptr, nullptr};
    this->error = nullptr;
    gboolean valid = true;

    this->pos = PARSER_POS_NOT_STARTED;
    this->creator = "Unknown";
    this->fileVersion = 1;

    GMarkupParseContext* context =
            g_markup_parse_context_new(&parser, static_cast<GMarkupParseFlags>(0), this, nullptr);

    zip_int64_t len = 0;
    do {
        char buffer[1024];
        len = readContentFile(buffer, sizeof(buffer));
        if (len > 0) {
            valid = g_markup_parse_context_parse(context, buffer, len, &error);
        }

        if (error) {
            g_warning("LoadHandler::parseXml: %s\n", error->message);
            valid = false;
            break;
        }
    } while (len >= 0 && valid && !error);

    if (valid) {
        valid = g_markup_parse_context_end_parse(context, &error);
    } else {
        if (error != nullptr && error->message != nullptr) {
            this->lastError = FS(_F("XML Parser error: {1}") % error->message);
            g_error_free(error);
        } else {
            this->lastError = _("Unknown parser error");
        }
        g_warning("LoadHandler::parseXml: %s\n", this->lastError.c_str());
    }

    g_markup_parse_context_free(context);

    // Add all parsed pages to the document
    this->doc->addPages(pages.begin(), pages.end());

    if (this->pos != PASER_POS_FINISHED && this->lastError.empty()) {
        lastError = _("Document is not complete (maybe the end is cut off?)");
        return false;
    }
    if (this->pos == PASER_POS_FINISHED && this->doc->getPageCount() == 0) {
        lastError = _("Document is corrupted (no pages found in file)");
        return false;
    }

    doc->setCreateBackupOnSave(true);

    return valid;
}

void LoadHandler::parseStart() {
    if (strcmp(elementName, "xournal") == 0) {
        endRootTag = "xournal";

        // Read the document version
        const char* version = LoadHandlerHelper::getAttrib("version", true, this);
        if (version) {
            this->creator = "Xournal ";
            this->creator += version;
        }

        const char* fileversion = LoadHandlerHelper::getAttrib("fileversion", true, this);
        if (fileversion) {
            this->fileVersion = atoi(fileversion);
        }
        const char* creator = LoadHandlerHelper::getAttrib("creator", true, this);
        if (creator) {
            this->creator = creator;
        }

        this->pos = PARSER_POS_STARTED;
    } else if (strcmp(elementName, "MrWriter") == 0) {
        endRootTag = "MrWriter";

        // Read the document version
        const char* version = LoadHandlerHelper::getAttrib("version", true, this);
        if (version) {
            this->creator = "MrWriter ";
            this->creator += version;
        }

        // Document version 1:
        // Handle it the same as a Xournal document, and don't allow to overwrite
        this->fileVersion = 1;
        this->pos = PARSER_POS_STARTED;
    } else {
        error("%s", FC(_F("Unexpected root tag: {1}") % elementName));
    }
}

void LoadHandler::parseContents() {
    if (strcmp(elementName, "page") == 0) {
        this->pos = PARSER_POS_IN_PAGE;

        double width = LoadHandlerHelper::getAttribDouble("width", this);
        double height = LoadHandlerHelper::getAttribDouble("height", this);

        this->page = std::make_unique<XojPage>(width, height, /*suppressLayer*/ true);

        pages.push_back(this->page);
    } else if (strcmp(elementName, "audio") == 0) {
        this->parseAudio();
    } else if (strcmp(elementName, "title") == 0) {
        // Ignore this tag, it says nothing...
    } else if (strcmp(elementName, "preview") == 0) {
        // Ignore this tag, we don't need a preview
    } else {
        g_warning("%s", FC(_F("Unexpected tag in document: \"{1}\"") % elementName));
    }
}

void LoadHandler::parseBgSolid() {
    PageType bg;
    const char* style = LoadHandlerHelper::getAttrib("style", false, this);
    if (style != nullptr) {
        bg.format = PageTypeHandler::getPageTypeFormatForString(style);
    }

    const char* config = LoadHandlerHelper::getAttrib("config", true, this);
    if (config != nullptr) {
        bg.config = config;
    }

    this->page->setBackgroundType(bg);

    Color color = LoadHandlerHelper::parseBackgroundColor(this);
    this->page->setBackgroundColor(color);

    const char* name = LoadHandlerHelper::getAttrib("name", true, this);
    if (name != nullptr) {
        this->page->setBackgroundName(name);
    }
}

void LoadHandler::parseBgPixmap() {
    const char* domain = LoadHandlerHelper::getAttrib("domain", false, this);
    const fs::path filepath = fs::u8path(LoadHandlerHelper::getAttrib("filename", false, this));
    // in case of a cloned background image, filename is a string representation of the page number from which the image
    // is cloned

    if (!strcmp(domain, "absolute") || (!strcmp(domain, "attach") && this->isGzFile)) {
        fs::path fileToLoad;
        if (!strcmp(domain, "attach")) {
            fileToLoad = this->filepath;
            fileToLoad += ".";
            fileToLoad += filepath;
        } else {
            fileToLoad = filepath;
        }

        GError* error = nullptr;
        BackgroundImage img;
        img.loadFile(fileToLoad, &error);

        if (error) {
            error("%s",
                  FC(_F("Could not read image: {1}. Error message: {2}") % fileToLoad.u8string() % error->message));
            g_error_free(error);
        }

        this->page->setBackgroundImage(img);
    } else if (!strcmp(domain, "attach")) {
        // This is the new zip file attach domain
        const auto readResult = readZipAttachment(filepath);  ///< Do not remove the const qualifier - see below
        if (!readResult) {
            return;
        }
        const std::string& imgData = *readResult;  ///< Do not remove the const qualifier - see below

        /**
         * To avoid an unecessary copy, the data is still managed by the std::unique_ptr<std::string> instance. The
         * input stream assumes the data will not be modified: do not remove the const qualifier on readResult or
         * imgData
         */
        xoj::util::GObjectSPtr<GInputStream> inputStream(
                g_memory_input_stream_new_from_data(imgData.data(), static_cast<gssize>(imgData.size()), nullptr),
                xoj::util::adopt);

        GError* error = nullptr;
        BackgroundImage img;
        img.loadFile(inputStream.get(), filepath, &error);

        g_input_stream_close(inputStream.get(), nullptr, nullptr);

        if (error) {
            error("%s", FC(_F("Could not read image: {1}. Error message: {2}") % filepath.string() % error->message));
            g_error_free(error);
        }

        this->page->setBackgroundImage(img);
    } else if (!strcmp(domain, "clone")) {
        gchar* endptr = nullptr;
        auto const& filename = filepath.u8string();
        auto nr = static_cast<size_t>(g_ascii_strtoull(filename.c_str(), &endptr, 10));
        if (endptr == filename.c_str()) {
            error("%s", FC(_F("Could not read page number for cloned background image: {1}.") % filepath.string()));
        }
        PageRef p = pages[nr];

        if (p) {
            this->page->setBackgroundImage(p->getBackgroundImage());
        }
    } else {
        error("%s", FC(_F("Unknown pixmap::domain type: {1}") % domain));
    }

    this->page->setBackgroundType(PageType(PageTypeFormat::Image));
}

void LoadHandler::parseBgPdf() {
    xoj_assert(this->doc);
    int pageno = LoadHandlerHelper::getAttribInt("pageno", this);
    bool attachToDocument = false;
    fs::path pdfFilename;

    this->page->setBackgroundPdfPageNr(as_unsigned(pageno) - 1);

    if (!this->pdfFilenameParsed) {

        const char* domain = LoadHandlerHelper::getAttrib("domain", false, this);
        {
            const char* sFilename = LoadHandlerHelper::getAttrib("filename", false, this);
            if (sFilename == nullptr) {
                error("PDF Filename missing!");
                return;
            }
            pdfFilename = fs::u8path(sFilename);
        }

        if (!strcmp("absolute", domain))  // Absolute OR relative path
        {
            if (pdfFilename.is_relative()) {
                pdfFilename = xournalFilepath.remove_filename() / pdfFilename;
            }
        } else if (!strcmp("attach", domain)) {
            attachToDocument = true;
            // Handle old format separately
            if (this->isGzFile) {
                pdfFilename = (fs::path{xournalFilepath} += ".") += pdfFilename;
            } else {
                auto pdfBytes = readZipAttachment(pdfFilename);
                if (!pdfBytes) {
                    return;
                }
                doc->readPdf(pdfFilename, false, attachToDocument, std::move(pdfBytes));

                if (!doc->getLastErrorMsg().empty()) {
                    error("%s", FC(_F("Error reading PDF: {1}") % doc->getLastErrorMsg()));
                }

                this->pdfFilenameParsed = true;
                return;
            }
        } else {
            error("%s", FC(_F("Unknown domain type: {1}") % domain));
            return;
        }

        this->pdfFilenameParsed = true;

        if (fs::is_regular_file(pdfFilename)) {
            doc->readPdf(pdfFilename, false, attachToDocument);
            if (!doc->getLastErrorMsg().empty()) {
                error("%s", FC(_F("Error reading PDF: {1}") % doc->getLastErrorMsg()));
            }
        } else {
            doc->setPdfAttributes(pdfFilename, attachToDocument);
            if (attachToDocument) {
                this->attachedPdfMissing = true;
            } else {
                this->pdfMissing = pdfFilename.u8string();
            }
        }
    }
}

void LoadHandler::parsePage() {
    if (!strcmp(elementName, "background")) {
        const char* name = LoadHandlerHelper::getAttrib("name", true, this);
        if (name != nullptr) {
            this->page->setBackgroundName(name);
        }

        const char* type = LoadHandlerHelper::getAttrib("type", false, this);

        if (strcmp("solid", type) == 0) {
            parseBgSolid();
        } else if (strcmp("pixmap", type) == 0) {
            parseBgPixmap();
        } else if (strcmp("pdf", type) == 0) {
            parseBgPdf();
        } else {
            error("%s", FC(_F("Unknown background type: {1}") % type));
        }
    } else if (!strcmp(elementName, "layer")) {
        this->pos = PARSER_POS_IN_LAYER;
        this->layer = std::make_unique<Layer>();

        const char* name = LoadHandlerHelper::getAttrib("name", true, this);
        if (name != nullptr) {
            this->layer->setName(name);
        }
    }
}

void LoadHandler::parseStroke() {
    this->stroke = std::make_unique<Stroke>();

    const char* width = LoadHandlerHelper::getAttrib("width", false, this);

    char* endPtr = nullptr;
    stroke->setWidth(g_ascii_strtod(width, &endPtr));
    if (endPtr == width) {
        error("%s", FC(_F("Error reading width of a stroke: {1}") % width));
        return;
    }

    // MrWriter writes pressures as separate field
    const char* pressure = LoadHandlerHelper::getAttrib("pressures", true, this);
    if (pressure == nullptr) {
        // Xournal / Xournal++ uses the width field
        pressure = endPtr;
    }

    while (*pressure != 0) {
        char* tmpptr = nullptr;
        double val = g_ascii_strtod(pressure, &tmpptr);
        if (tmpptr == pressure) {
            break;
        }
        pressure = tmpptr;
        this->pressureBuffer.push_back(val);
    }

    Color color{0U};
    const char* sColor = LoadHandlerHelper::getAttrib("color", false, this);
    if (!LoadHandlerHelper::parseColor(sColor, color, this)) {
        return;
    }
    stroke->setColor(color);

    /** read stroke timestamps (xopp fileformat) */
    const char* fn = LoadHandlerHelper::getAttrib("fn", true, this);
    if (fn != nullptr && strlen(fn) > 0) {
        if (this->isGzFile) {
            stroke->setAudioFilename(fs::u8path(fn));
        } else {
            auto tempFile = getTempFileForPath(fn);
            if (!tempFile.empty()) {
                stroke->setAudioFilename(tempFile);
            }
        }
    }


    if (this->fileVersion < 4) {
        int ts = 0;
        if (LoadHandlerHelper::getAttribInt("ts", true, this, ts)) {
            stroke->setTimestamp(as_unsigned(ts) * 1000);
        }
    } else {
        size_t ts = 0;
        if (LoadHandlerHelper::getAttribSizeT("ts", true, this, ts)) {
            stroke->setTimestamp(ts);
        }
    }

    int fill = -1;
    if (LoadHandlerHelper::getAttribInt("fill", true, this, fill)) {
        stroke->setFill(fill);
    }

    const char* capStyleStr = LoadHandlerHelper::getAttrib("capStyle", true, this);
    if (capStyleStr != nullptr) {
        if (strcmp("butt", capStyleStr) == 0) {
            stroke->setStrokeCapStyle(StrokeCapStyle::BUTT);
        } else if (strcmp("round", capStyleStr) == 0) {
            stroke->setStrokeCapStyle(StrokeCapStyle::ROUND);
        } else if (strcmp("square", capStyleStr) == 0) {
            stroke->setStrokeCapStyle(StrokeCapStyle::SQUARE);
        } else {
            g_warning("%s", FC(_F("Unknown stroke cap type: \"{1}\", assuming round") % capStyleStr));
        }
    }

    const char* style = LoadHandlerHelper::getAttrib("style", true, this);
    if (style != nullptr) {
        stroke->setLineStyle(StrokeStyle::parseStyle(style));
    }

    const char* tool = LoadHandlerHelper::getAttrib("tool", false, this);

    if (strcmp("eraser", tool) == 0) {
        stroke->setToolType(StrokeTool::ERASER);
    } else if (strcmp("pen", tool) == 0) {
        stroke->setToolType(StrokeTool::PEN);
    } else if (strcmp("highlighter", tool) == 0) {
        stroke->setToolType(StrokeTool::HIGHLIGHTER);
    } else {
        g_warning("%s", FC(_F("Unknown stroke type: \"{1}\", assuming pen") % tool));
    }

    /**
     * For each stroke being read, set the timestamp value
     * we've read just before.
     * Afterwards, clean the read timestamp data.
     */
    if (loadedFilename.length() != 0) {
        this->stroke->setTimestamp(as_unsigned(loadedTimeStamp));
        this->stroke->setAudioFilename(loadedFilename);
        loadedFilename = "";
        loadedTimeStamp = 0;
    }
}

void LoadHandler::parseText() {
    this->text = std::make_unique<Text>();

    const char* sFont = LoadHandlerHelper::getAttrib("font", false, this);
    double fontSize = LoadHandlerHelper::getAttribDouble("size", this);
    double x = LoadHandlerHelper::getAttribDouble("x", this);
    double y = LoadHandlerHelper::getAttribDouble("y", this);

    this->text->setX(x);
    this->text->setY(y);

    XojFont& f = text->getFont();
    f.setName(sFont);
    f.setSize(fontSize);
    const char* sColor = LoadHandlerHelper::getAttrib("color", false, this);
    Color color{0U};
    LoadHandlerHelper::parseColor(sColor, color, this);
    text->setColor(color);

    const char* fn = LoadHandlerHelper::getAttrib("fn", true, this);
    if (fn != nullptr && strlen(fn) > 0) {
        if (this->isGzFile) {
            text->setAudioFilename(fs::u8path(fn));
        } else {
            auto tempFile = getTempFileForPath(fn);
            if (!tempFile.empty()) {
                text->setAudioFilename(tempFile);
            }
        }
    }

    size_t ts = 0;
    if (LoadHandlerHelper::getAttribSizeT("ts", true, this, ts)) {
        text->setTimestamp(ts);
    }
}

void LoadHandler::parseImage() {
    double left = LoadHandlerHelper::getAttribDouble("left", this);
    double top = LoadHandlerHelper::getAttribDouble("top", this);
    double right = LoadHandlerHelper::getAttribDouble("right", this);
    double bottom = LoadHandlerHelper::getAttribDouble("bottom", this);

    xoj_assert(this->image == nullptr);
    this->image = std::make_unique<Image>();
    this->image->setX(left);
    this->image->setY(top);
    this->image->setWidth(right - left);
    this->image->setHeight(bottom - top);
}

void LoadHandler::parseTexImage() {
    double left = LoadHandlerHelper::getAttribDouble("left", this);
    double top = LoadHandlerHelper::getAttribDouble("top", this);
    double right = LoadHandlerHelper::getAttribDouble("right", this);
    double bottom = LoadHandlerHelper::getAttribDouble("bottom", this);

    const char* imText = LoadHandlerHelper::getAttrib("text", false, this);
    const char* compatibilityTest = LoadHandlerHelper::getAttrib("texlength", true, this);
    auto imTextLen = strlen(imText);
    if (compatibilityTest != nullptr) {
        imTextLen = LoadHandlerHelper::getAttribSizeT("texlength", this);
    }

    this->teximage = std::make_unique<TexImage>();
    this->teximage->setX(left);
    this->teximage->setY(top);
    this->teximage->setWidth(right - left);
    this->teximage->setHeight(bottom - top);

    this->teximage->setText(string(imText, imTextLen));
}

void LoadHandler::parseAttachment() {
    if (this->pos != PARSER_POS_IN_IMAGE && this->pos != PARSER_POS_IN_TEXIMAGE) {
        g_warning("Found attachment tag as child of a tag that should not have such a child (ignoring this tag)");
        return;
    }
    const char* path = LoadHandlerHelper::getAttrib("path", false, this);

    auto readResult = readZipAttachment(path);
    if (!readResult) {
        return;
    }
    std::string& imgData = *readResult;

    switch (this->pos) {
        case PARSER_POS_IN_IMAGE: {
            this->image->setImage(std::move(imgData));
            break;
        }
        case PARSER_POS_IN_TEXIMAGE: {
            this->teximage->loadData(std::move(imgData), nullptr);
            break;
        }
        default:
            break;
    }
}

void LoadHandler::parseLayer() {
    /**
     * read the timestamp before each stroke.
     * Used for backwards compatibility
     * against xoj files with timestamps)
     **/
    if (!strcmp(elementName, "timestamp")) {
        loadedTimeStamp = LoadHandlerHelper::getAttribInt("ts", this);
        loadedFilename = LoadHandlerHelper::getAttrib("fn", false, this);
    }
    if (!strcmp(elementName, "stroke"))  // start of a stroke
    {
        this->pos = PARSER_POS_IN_STROKE;
        parseStroke();
    } else if (!strcmp(elementName, "text"))  // start of a text item
    {
        this->pos = PARSER_POS_IN_TEXT;
        parseText();
    } else if (!strcmp(elementName, "image"))  // start of a image item
    {
        this->pos = PARSER_POS_IN_IMAGE;
        parseImage();
    } else if (!strcmp(elementName, "teximage"))  // start of a image item
    {
        this->pos = PARSER_POS_IN_TEXIMAGE;
        parseTexImage();
    }
}

/**
 * Create a temporary file for the attached audio file.
 * The OS should take care of removing the file.
 */
void LoadHandler::parseAudio() {
    const char* filename = LoadHandlerHelper::getAttrib("fn", false, this);

    GFileIOStream* fileStream = nullptr;
    xoj::util::GObjectSPtr<GFile> tmpFile(g_file_new_tmp("xournal_audio_XXXXXX.tmp", &fileStream, nullptr),
                                          xoj::util::adopt);
    if (!tmpFile) {
        g_warning("Unable to create temporary file for audio attachment.");
        return;
    }

    GOutputStream* outputStream = g_io_stream_get_output_stream(G_IO_STREAM(fileStream));

    zip_stat_t attachmentFileStat;
    int statStatus = zip_stat(this->zipFp, filename, 0, &attachmentFileStat);
    if (statStatus != 0) {
        error("%s", FC(_F("Could not open attachment: {1}. Error message: {2}") % filename %
                       zip_error_strerror(zip_get_error(this->zipFp))));
        return;
    }

    gsize length = 0;
    if (attachmentFileStat.valid & ZIP_STAT_SIZE) {
        length = attachmentFileStat.size;
    } else {
        error("%s", FC(_F("Could not open attachment: {1}. Error message: No valid file size provided") % filename));
        return;
    }

    zip_file_t* attachmentFile = zip_fopen(this->zipFp, filename, 0);

    if (!attachmentFile) {
        error("%s", FC(_F("Could not open attachment: {1}. Error message: {2}") % filename %
                       zip_error_strerror(zip_get_error(this->zipFp))));
        return;
    }

    gpointer data = g_malloc(1024);
    zip_uint64_t readBytes = 0;
    while (readBytes < length) {
        zip_int64_t read = zip_fread(attachmentFile, data, 1024);
        if (read == -1) {
            g_free(data);
            zip_fclose(attachmentFile);
            error("%s", FC(_F("Could not open attachment: {1}. Error message: Could not read file") % filename));
            return;
        }

        gboolean writeSuccessful =
                g_output_stream_write_all(outputStream, data, static_cast<gsize>(read), nullptr, nullptr, nullptr);
        if (!writeSuccessful) {
            g_free(data);
            zip_fclose(attachmentFile);
            error("%s", FC(_F("Could not open attachment: {1}. Error message: Could not write file") % filename));
            return;
        }

        readBytes += static_cast<zip_uint64_t>(read);
    }
    g_free(data);
    zip_fclose(attachmentFile);

    char* tmpPath = g_file_get_path(tmpFile.get());
    this->audioFiles[filename] = tmpPath;
    g_free(tmpPath);
}

void LoadHandler::parserStartElement(GMarkupParseContext* context, const gchar* elementName,
                                     const gchar** attributeNames, const gchar** attributeValues, gpointer userdata,
                                     GError** error) {
    auto* handler = static_cast<LoadHandler*>(userdata);
    // Return on error
    if (*error) {
        return;
    }
    handler->attributeNames = attributeNames;
    handler->attributeValues = attributeValues;
    handler->elementName = elementName;

    if (handler->pos == PARSER_POS_NOT_STARTED) {
        handler->parseStart();
    } else if (handler->pos == PARSER_POS_STARTED) {
        handler->parseContents();
    } else if (handler->pos == PARSER_POS_IN_PAGE) {
        handler->parsePage();
    } else if (handler->pos == PARSER_POS_IN_LAYER) {
        handler->parseLayer();
    } else if (handler->pos == PARSER_POS_IN_IMAGE || handler->pos == PARSER_POS_IN_TEXIMAGE) {
        // Handle the attachment node within the appropriate nodes
        if (!strcmp(elementName, "attachment")) {
            handler->parseAttachment();
        }
    }

    handler->attributeNames = nullptr;
    handler->attributeValues = nullptr;
    handler->elementName = nullptr;
}

void LoadHandler::parserEndElement(GMarkupParseContext* context, const gchar* elementName, gpointer userdata,
                                   GError** error) {
    // Return on error
    if (*error) {
        return;
    }

    auto* handler = static_cast<LoadHandler*>(userdata);
    if (handler->pos == PARSER_POS_STARTED && strcmp(elementName, handler->endRootTag) == 0) {
        handler->pos = PASER_POS_FINISHED;
    } else if (handler->pos == PARSER_POS_IN_PAGE && strcmp(elementName, "page") == 0) {
        // handle unnecessary layer insertion in case of existing layers in file
        if (handler->page->getLayerCount() == 0) {
            handler->page->addLayer(new Layer());
        }
        handler->pos = PARSER_POS_STARTED;
        // A handle to the page is still held by handler->pages
        handler->page.reset();
    } else if (handler->pos == PARSER_POS_IN_LAYER && strcmp(elementName, "layer") == 0) {
        handler->pos = PARSER_POS_IN_PAGE;
        handler->page->addLayer(handler->layer.release());
    } else if (handler->pos == PARSER_POS_IN_LAYER && strcmp(elementName, "timestamp") == 0) {
        /** Used for backwards compatibility against xoj files with timestamps) */
        handler->pos = PARSER_POS_IN_LAYER;
    } else if (handler->pos == PARSER_POS_IN_STROKE && strcmp(elementName, "stroke") == 0) {
        handler->pos = PARSER_POS_IN_LAYER;
        if (handler->stroke) {
            // fixNullPressureValues() may have deleted the stroke. Check if it
            // still exists before accessing it.
            handler->layer->addElement(std::move(handler->stroke));
        }
    } else if (handler->pos == PARSER_POS_IN_TEXT && strcmp(elementName, "text") == 0) {
        handler->pos = PARSER_POS_IN_LAYER;
        handler->layer->addElement(std::move(handler->text));
    } else if (handler->pos == PARSER_POS_IN_IMAGE && strcmp(elementName, "image") == 0) {
        xoj_assert_message(handler->image->getImage() != nullptr, "image can't be rendered");
        handler->pos = PARSER_POS_IN_LAYER;
        handler->layer->addElement(std::move(handler->image));
    } else if (handler->pos == PARSER_POS_IN_TEXIMAGE && strcmp(elementName, "teximage") == 0) {
        handler->pos = PARSER_POS_IN_LAYER;
        handler->layer->addElement(std::move(handler->teximage));
    }
}

void LoadHandler::fixNullPressureValues(std::vector<double> pressures) {
    /*
     * Due to various bugs (see e.g. https://github.com/xournalpp/xournalpp/issues/3643), old files may contain strokes
     * with non-positive pressure values.
     *
     * Those strokes thus fails the somewhat reasonable assumption that pressure values should be positive.
     * The portions of stroke with non-positive pressure values are in any case invisible.
     *
     * This function fixes corrupted strokes by
     *  1- removing every point with a non-positive pressure value.
     *  2- if required, splitting the affected stroke into several strokes.
     *  3- entirely deleting strokes without any valid pressure value
     */
    auto& pts = stroke->getPointVector();
    if (pressures.size() >= pts.size()) {
        // Too many pressure values. Drop the last ones
        xoj_assert(pts.size() >= 2);  // An error has already been returned otherwise
        pressures.resize(pts.size() - 1);
    }

    auto nextPositive = std::find_if(pressures.begin(), pressures.end(), [](double v) { return v > 0; });

    std::vector<std::vector<Point>> strokePortions;
    while (nextPositive != pressures.end()) {
        auto nextNonPositive =
                std::find_if(nextPositive, pressures.end(), [](double v) { return v <= 0 || std::isnan(v); });
        size_t nValidPressureValues = static_cast<size_t>(std::distance(nextPositive, nextNonPositive));

        std::vector<Point> ps;
        ps.reserve(nValidPressureValues + 1);

        std::transform(nextPositive, nextNonPositive,
                       std::next(pts.begin(), std::distance(pressures.begin(), nextPositive)), std::back_inserter(ps),
                       [](double v, const Point& p) { return Point(p.x, p.y, v); });

        // pts.size() == pressures.size() + 1 so the following iterator is always dereferencable, even if
        // nextNonPositive == pressures.end()
        ps.emplace_back(*std::next(pts.begin(), std::distance(pressures.begin(), nextNonPositive)));

        xoj_assert(ps.size() == nValidPressureValues + 1);
        strokePortions.emplace_back(std::move(ps));

        if (nextNonPositive == pressures.end()) {
            break;
        }
        nextPositive = std::find_if(nextNonPositive, pressures.end(), [](double v) { return v > 0; });
    }

    if (strokePortions.empty()) {
        // There was no valid pressure values! Delete the stroke entirely
        g_warning("Found a stroke with only non-positive pressure values! Removing this invisible stroke.");
        this->stroke.reset();
        return;
    }

    g_warning("Found a stroke with some non-positive pressure values. Removing the affected points.");
    for_first_then_each(
            strokePortions, [&](std::vector<Point>& points) { this->stroke->setPointVector(std::move(points)); },
            [&](std::vector<Point>& points) {
                auto newStroke = std::make_unique<Stroke>();
                newStroke->applyStyleFrom(this->stroke.get());
                newStroke->setPointVector(std::move(points));
                this->layer->addElement(std::move(this->stroke));
                this->stroke = std::move(newStroke);
            });
}

void LoadHandler::parserText(GMarkupParseContext* context, const gchar* text, gsize textLen, gpointer userdata,
                             GError** error) {
    // Return on error
    if (*error) {
        return;
    }

    auto* handler = static_cast<LoadHandler*>(userdata);
    if (handler->pos == PARSER_POS_IN_STROKE) {
        const char* ptr = text;
        int n = 0;

        bool xRead = false;
        double x = 0;

        while (textLen > 0) {
            double tmp = g_ascii_strtod(text, const_cast<char**>(&ptr));
            if (ptr == text) {
                break;
            }
            textLen -= as_unsigned(ptr - text);
            text = ptr;
            n++;

            if (!xRead) {
                xRead = true;
                x = tmp;
            } else {
                xRead = false;
                handler->stroke->addPoint(Point(x, tmp));
            }
        }
        handler->stroke->freeUnusedPointItems();

        if (n < 4 || (n & 1)) {
            error2(*error, "%s", FC(_F("Wrong count of points ({1})") % n));
            return;
        }

        if (!handler->pressureBuffer.empty()) {
            if (handler->pressureBuffer.size() + 1 >= handler->stroke->getPointCount()) {
                auto firstNonPositive = std::find_if(handler->pressureBuffer.begin(), handler->pressureBuffer.end(),
                                                     [](double v) { return v <= 0 || std::isnan(v); });
                if (firstNonPositive != handler->pressureBuffer.end()) {
                    // Warning: this may delete handler->stroke if no positive pressure values are provided
                    // Do not dereference handler->stroke after that
                    handler->fixNullPressureValues(std::move(handler->pressureBuffer));
                } else {
                    handler->stroke->setPressure(handler->pressureBuffer);
                }
            } else {
                g_warning("%s", FC(_F("xoj-File: {1}") % handler->filepath.u8string()));
                g_warning("%s", FC(_F("Wrong number of pressure values, got {1}, expected {2}") %
                                   handler->pressureBuffer.size() % (handler->stroke->getPointCount() - 1)));
            }
            handler->pressureBuffer.clear();
        }
    } else if (handler->pos == PARSER_POS_IN_TEXT) {
        gchar* txt = g_strndup(text, textLen);
        handler->text->setText(txt);
        g_free(txt);
    } else if (handler->pos == PARSER_POS_IN_IMAGE) {
        handler->readImage(text, textLen);
    } else if (handler->pos == PARSER_POS_IN_TEXIMAGE) {
        handler->readTexImage(text, textLen);
    }
}

auto LoadHandler::parseBase64(const gchar* base64, gsize length) -> string {
    // We have to copy the string in order to null terminate it, sigh.
    auto* base64data = static_cast<gchar*>(g_memdup(base64, static_cast<guint>(length) + 1));
    base64data[length] = '\0';

    gsize binaryBufferLen = 0;
    guchar* binaryBuffer = g_base64_decode(base64data, &binaryBufferLen);
    g_free(base64data);

    string str = string(reinterpret_cast<char*>(binaryBuffer), binaryBufferLen);
    g_free(binaryBuffer);

    return str;
}

void LoadHandler::readImage(const gchar* base64string, gsize base64stringLen) {
    xoj_assert(this->image != nullptr);
    if (base64stringLen == 0 || (base64stringLen == 1 && base64string[0] == '\n') || this->image->hasData()) {
        return;
    }

    this->image->setImage(parseBase64(const_cast<char*>(base64string), base64stringLen));
}

void LoadHandler::readTexImage(const gchar* base64string, gsize base64stringLen) {
    if (base64stringLen == 1 && !strcmp(base64string, "\n")) {
        return;
    }

    this->teximage->loadData(parseBase64(const_cast<char*>(base64string), base64stringLen));
}

auto LoadHandler::loadDocument(fs::path const& filepath) -> std::unique_ptr<Document> {
    initAttributes();
    this->doc = std::make_unique<Document>(&dHanlder);

    if (!openFile(filepath)) {
        this->doc.reset();
        return nullptr;
    }

    const auto start = clock();


    xournalFilepath = filepath;

    this->pdfFilenameParsed = false;

    if (!parseXml()) {
        closeFile();
        this->doc.reset();
        return nullptr;
    }

    const auto stop = clock();
    g_message("Time taken by old parser: %ld clocks", stop - start);


    if (fileVersion == 1) {
        // This is a Xournal document, not a Xournal++
        // Even if the new fileextension is .xopp, allow to
        // overwrite .xoj files which are created by Xournal++
        // Force the user to save is a bad idea, this will annoy the user
        // Rename files is also not that user friendly.

        doc->setFilepath("");
    } else {
        doc->setFilepath(filepath);
    }

    closeFile();

    return std::move(this->doc);
}

auto LoadHandler::readZipAttachment(fs::path const& filename) -> std::unique_ptr<std::string> {
    zip_stat_t attachmentFileStat;
    const int statStatus = zip_stat(this->zipFp, filename.u8string().c_str(), 0, &attachmentFileStat);
    if (statStatus != 0) {
        error("%s", FC(_F("Could not open attachment: {1}. Error message: {2}") % filename.string() %
                       zip_error_strerror(zip_get_error(this->zipFp))));
        return {};
    }

    if (!(attachmentFileStat.valid & ZIP_STAT_SIZE)) {
        error("%s",
              FC(_F("Could not open attachment: {1}. Error message: No valid file size provided") % filename.string()));
        return {};
    }
    const zip_uint64_t length = attachmentFileStat.size;

    zip_file_t* attachmentFile = zip_fopen(this->zipFp, filename.u8string().c_str(), 0);
    if (!attachmentFile) {
        error("%s", FC(_F("Could not open attachment: {1}. Error message: {2}") % filename.string() %
                       zip_error_strerror(zip_get_error(this->zipFp))));
        return {};
    }

    auto data = std::make_unique<std::string>(length, 0);
    zip_uint64_t readBytes = 0;
    while (readBytes < length) {
        const zip_int64_t read = zip_fread(attachmentFile, data->data() + readBytes, length - readBytes);
        if (read == -1) {
            zip_fclose(attachmentFile);
            error("%s", FC(_F("Could not open attachment: {1}. Error message: No valid file size provided") %
                           filename.u8string()));
            return {};
        }

        readBytes += static_cast<zip_uint64_t>(read);
    }

    zip_fclose(attachmentFile);

    return data;
}

void LoadHandler::setAudioAttributes(AudioElement& elem, const fs::path& filename, size_t timestamp) {
    if (!filename.empty()) {
        if (this->isGzFile) {
            this->text->setAudioFilename(filename);
        } else {
            const auto tempFile = getTempFileForPath(filename);
            if (!tempFile.empty()) {
                this->text->setAudioFilename(tempFile);
            }
        }

        if (this->fileVersion < 4) {
            timestamp *= 1000;
        }
        elem.setTimestamp(timestamp);
    }
}

auto LoadHandler::getTempFileForPath(fs::path const& filename) -> fs::path {
    auto it = this->audioFiles.find(filename.string());

    if (it == this->audioFiles.end()) {
        error("%s", FC(_F("Requested temporary file was not found for attachment {1}") % filename.u8string()));
        return {};
    } else {
        return it->second;
    }
}

auto LoadHandler::getFileVersion() const -> int { return this->fileVersion; }
