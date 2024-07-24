#include "LoadHandler.h"

#include <algorithm>      // for copy
#include <array>          // for array
#include <cmath>          // for isnan
#include <cstdlib>        // for atoi, size_t
#include <cstring>        // for strcmp, strlen
#include <exception>      // for exception
#include <iterator>       // for back_inserter
#include <memory>         // for make_unique, make_shared...
#include <optional>       // for optional
#include <regex>          // for regex_search, match_results
#include <stdexcept>      // for runtime_error
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <utility>        // for move, forward, exchange
#include <vector>         // for vector

#include <gio/gio.h>     // for g_file_get_path, g_fil...
#include <glib.h>        // for g_message...
#include <glibconfig.h>  // for gssize...
#include <zip.h>         // for zip_file_t, zip_fopen,...
#include <zipconf.h>     // for zip_int64_t, zip_uint64_t

#include "control/xojfile/GzInputStream.h"   // for GzInputStream
#include "control/xojfile/XmlParser.h"       // for XmlParser
#include "control/xojfile/ZipInputStream.h"  // for ZipInputStream
#include "model/BackgroundImage.h"           // for BackgroundImage
#include "model/Document.h"                  // for Document
#include "model/Font.h"                      // for XojFont
#include "model/Image.h"                     // for Image
#include "model/Layer.h"                     // for Layer
#include "model/PageType.h"                  // for PageType, PageTypeFormat
#include "model/Point.h"                     // for Point
#include "model/Stroke.h"                    // for Stroke, StrokeCapStyle
#include "model/TexImage.h"                  // for TexImage
#include "model/Text.h"                      // for Text
#include "model/XojPage.h"                   // for XojPage
#include "util/Assert.h"                     // for xoj_assert
#include "util/Color.h"                      // for Color
#include "util/LoopUtil.h"                   // for for_first_then_each
#include "util/PlaceholderString.h"          // for PlaceholderString
#include "util/i18n.h"                       // for _F, FC, FS, _
#include "util/raii/CLibrariesSPtr.h"        // for adopt
#include "util/raii/GLibGuards.h"            // for GErrorGuard
#include "util/raii/GObjectSPtr.h"           // for GObjectSPtr

#include "filesystem.h"  // for path, is_regular_file


namespace {
constexpr size_t MAX_VERSION_LENGTH = 50;
constexpr size_t MAX_MIMETYPE_LENGTH = 25;

struct zip_file_deleter {
    void operator()(zip_file_t* ptr) { zip_fclose(ptr); }
};
}  // namespace

using zip_file_wrapper = std::unique_ptr<zip_file_t, zip_file_deleter>;

LoadHandler::LoadHandler():
        parsingComplete(false),
        attachedPdfMissing(false),
        pdfFilenameParsed(false),
        fileVersion(0),
        minimalFileVersion(0),
        isGzFile(false) {}

LoadHandler::~LoadHandler() = default;

void LoadHandler::initAttributes() {
    this->parsingComplete = false;
    this->errorMessages.clear();

    this->missingPdf.clear();
    this->attachedPdfMissing = false;
    this->pdfFilenameParsed = false;

    this->creator.clear();
    this->fileVersion = 0;
    this->minimalFileVersion = 0;

    this->zipFp.reset();
    this->isGzFile = false;
    this->xmlContentStream.reset();

    this->pages.clear();
    this->audioFiles.clear();

    this->page.reset();
    this->layer.reset();
    this->stroke.reset();
    this->image.reset();
    this->teximage.reset();
    this->text.reset();
}

auto LoadHandler::hasErrorMessages() const -> bool { return !this->errorMessages.empty(); }

auto LoadHandler::getErrorMessages() const -> std::string {
    std::string messages;
    for (const auto& error: this->errorMessages) {
        messages += '\n' + error;
    }
    return messages;
}

auto LoadHandler::isAttachedPdfMissing() const -> bool { return this->attachedPdfMissing; }

auto LoadHandler::getMissingPdfFilename() const -> const fs::path& { return this->missingPdf; }

auto LoadHandler::getFileVersion() const -> int { return this->fileVersion; }


void LoadHandler::addXournal(std::string creator, int fileversion) {
    this->creator = std::move(creator);
    this->fileVersion = fileversion;
}

void LoadHandler::addMrWriter(std::string creator) {
    this->creator = std::move(creator);
    this->fileVersion = 1;
}

void LoadHandler::finalizeDocument() {
    this->doc->addPages(this->pages.begin(), this->pages.end());
    this->pages.clear();
    this->parsingComplete = true;
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

void LoadHandler::addAudioAttachment(fs::path filename) {
    // Verify if attachment exists and is valid
    zip_stat_t attachmentFileStat;
    const int statStatus = zip_stat(this->zipFp.get(), filename.u8string().c_str(), 0, &attachmentFileStat);
    if (statStatus != 0) {
        logError(FS(PlaceholderString("Could not open attachment: {1}. Error message: {2}") %
                    filename.u8string().c_str() % zip_error_strerror(zip_get_error(this->zipFp.get()))));
        return;
    }

    zip_int64_t length = 0;
    if (attachmentFileStat.valid & ZIP_STAT_SIZE) {
        length = static_cast<zip_int64_t>(attachmentFileStat.size);
    } else {
        logError(FS(PlaceholderString("Could not open attachment: {1}. Error message: No valid file size provided") %
                    filename.u8string().c_str()));
        return;
    }

    auto attachmentFile = zip_file_wrapper(zip_fopen(this->zipFp.get(), filename.u8string().c_str(), 0));

    if (!attachmentFile) {
        logError(FS(PlaceholderString("Could not open attachment: {1}. Error message: {2}") %
                    filename.u8string().c_str() % zip_error_strerror(zip_get_error(this->zipFp.get()))));
        return;
    }

    // Extract audio to temporary file
    GFileIOStream* fileStream = nullptr;
    const xoj::util::GObjectSPtr<GFile> tmpFile(g_file_new_tmp("xournal_audio_XXXXXX.tmp", &fileStream, nullptr),
                                                xoj::util::adopt);
    if (!tmpFile) {
        logError("Unable to create temporary file for audio attachment.");
        return;
    }

    GOutputStream* outputStream = g_io_stream_get_output_stream(G_IO_STREAM(fileStream));

    auto data = std::make_unique<std::array<std::byte, 1024>>();
    zip_int64_t readBytes = 0;
    while (readBytes < length) {
        const zip_int64_t read = zip_fread(attachmentFile.get(), data->data(), data->size());
        if (read == -1) {
            logError(FS(PlaceholderString("Could not open attachment: {1}. Error message: Could not read file") %
                        filename.u8string().c_str()));
            return;
        }

        const gboolean writeSuccessful = g_output_stream_write_all(outputStream, data->data(), static_cast<gsize>(read),
                                                                   nullptr, nullptr, nullptr);
        if (!writeSuccessful) {
            logError(FS(
                    PlaceholderString("Could not open attachment: {1}. Error message: Could not write temporary file") %
                    filename.u8string().c_str()));
            return;
        }

        readBytes += read;
    }

    // Map the filename to the extracted temporary filepath
    const char* tmpPath = g_file_peek_path(tmpFile.get());
    // fs::path does not have a std::hash specialization on all compilers
    this->audioFiles[filename] = tmpPath;
}

void LoadHandler::setBgName(std::string name) {
    xoj_assert(!name.empty());
    this->page->setBackgroundName(std::move(name));
}

void LoadHandler::setBgSolid(const PageType& bg, Color color) {
    this->page->setBackgroundType(bg);
    this->page->setBackgroundColor(color);
}

void LoadHandler::setBgPixmap(bool attach, const fs::path& filename) {
    BackgroundImage img;

    if (this->isGzFile || !attach) {
        fs::path fileToLoad = getAbsoluteFilepath(filename, attach);

        xoj::util::GErrorGuard error{};
        img.loadFile(fileToLoad, xoj::util::out_ptr(error));

        if (error) {
            logError(FS(PlaceholderString("Could not read image: {1}. Error message: {2}") %
                        fileToLoad.u8string().c_str() % error->message));
        }
    } else {
        // The image is stored in an attachemt inside the zip archive
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

        xoj::util::GErrorGuard error{};
        img.loadFile(inputStream.get(), filename, xoj::util::out_ptr(error));

        g_input_stream_close(inputStream.get(), nullptr, nullptr);

        if (error) {
            logError(FS(PlaceholderString("Could not read image: {1}. Error message: {2}") %
                        filename.u8string().c_str() % error->message));
        }
    }
    this->page->setBackgroundImage(img);
    this->page->setBackgroundType(PageType(PageTypeFormat::Image));
}

void LoadHandler::setBgPixmapCloned(size_t pageNr) {
    if (pageNr >= this->doc->getPageCount()) {
        logError(FS(PlaceholderString("Cloned pixmap page background on page {1} references inexistent page {2}") %
                    this->doc->getPageCount() % pageNr));
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
        fs::path pdfFilepath = getAbsoluteFilepath(filename, attach);

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
                this->missingPdf = pdfFilepath;
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

void LoadHandler::addStroke(StrokeTool tool, Color color, double width, int fill, StrokeCapStyle capStyle,
                            const std::optional<LineStyle>& lineStyle, fs::path filename, size_t timestamp) {
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

    setAudioAttributes(*this->stroke, std::move(filename), timestamp);
}

void LoadHandler::setStrokePoints(std::vector<Point> pointVector, std::vector<double> pressures) {
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

void LoadHandler::addText(std::string font, double size, double x, double y, Color color, fs::path filename,
                          size_t timestamp) {
    xoj_assert(!this->text);
    this->text = std::make_unique<Text>();

    XojFont& f = this->text->getFont();
    f.setName(std::move(font));
    f.setSize(size);
    this->text->setX(x);
    this->text->setY(y);
    this->text->setColor(color);

    setAudioAttributes(*this->text, std::move(filename), timestamp);
}

void LoadHandler::setTextContents(std::string contents) {
    xoj_assert(this->text);

    this->text->setText(std::move(contents));
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

void LoadHandler::setImageData(std::string data) {
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

void LoadHandler::addTexImage(double left, double top, double right, double bottom, std::string text) {
    xoj_assert(!this->teximage);
    this->teximage = std::make_unique<TexImage>();

    this->teximage->setX(left);
    this->teximage->setY(top);
    this->teximage->setWidth(right - left);
    this->teximage->setHeight(bottom - top);

    this->teximage->setText(std::move(text));
}

void LoadHandler::setTexImageData(std::string data) {
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


void LoadHandler::openFile(fs::path const& filepath) {
    this->xournalFilepath = filepath;
    int zipError = 0;
    this->zipFp = zip_wrapper(zip_open(filepath.u8string().c_str(), ZIP_RDONLY, &zipError));

    // Try to open the file as a gzip
    if (!this->zipFp && zipError == ZIP_ER_NOZIP) {
        this->xmlContentStream = std::make_unique<GzInputStream>(filepath);
        this->isGzFile = true;
    }

    if (this->zipFp && !this->isGzFile) {
        // Check the mimetype
        auto mimetypeFp = zip_file_wrapper(zip_fopen(this->zipFp.get(), "mimetype", 0));
        if (!mimetypeFp) {
            throw std::runtime_error(
                    FS(_F("The file \"{1}\" is no valid .xopp file (Mimetype missing). Error message: {2}") %
                       filepath.u8string() % zip_error_strerror(zip_get_error(zipFp.get()))));
        }
        std::array<char, MAX_MIMETYPE_LENGTH + 1> mimetype = {};
        // read the mimetype and a few more bytes to make sure we do not only read a subset
        zip_fread(mimetypeFp.get(), mimetype.data(), MAX_MIMETYPE_LENGTH);
        if (!strcmp(mimetype.data(), "application/xournal++")) {
            throw std::runtime_error(FS(_F("The file \"{1}\" is no valid .xopp file (Mimetype wrong): \"{2}\"") %
                                        filepath.u8string() % mimetype.data()));
        }

        // Get the file version
        auto versionFp = zip_file_wrapper(zip_fopen(this->zipFp.get(), "META-INF/version", 0));
        if (!versionFp) {
            throw std::runtime_error(
                    FS(_F("The file \"{1}\" is no valid .xopp file (Version missing). Error message: {2}") %
                       filepath.u8string() % zip_error_strerror(zip_get_error(zipFp.get()))));
        }
        std::array<char, MAX_VERSION_LENGTH + 1> versionString = {};
        const auto length = std::max(zip_fread(versionFp.get(), versionString.data(), MAX_VERSION_LENGTH),
                                     static_cast<zip_int64_t>(0));
        const std::regex versionRegex("current=(\\d+?)(?:\n|\r\n)min=(\\d+?)");
        std::match_results<char*> match;
        if (std::regex_search(versionString.data(), versionString.data() + length, match, versionRegex)) {
            this->fileVersion = std::stoi(match.str(1));
            this->minimalFileVersion = std::stoi(match.str(2));
        } else {
            throw std::runtime_error(
                    FS(_F("The file \"{1}\" is not a valid .xopp file (Version string corrupted): \"{2}\"") %
                       filepath.u8string() % std::string(versionString.data(), versionString.data() + length)));
        }

        // open the main content file
        this->xmlContentStream = std::make_unique<ZipInputStream>(this->zipFp.get(), "content.xml");
    }

    // Fail if neither utility could open the file
    if (!this->xmlContentStream) {
        throw std::runtime_error(FS(_F("Could not open file: \"{1}\"") % filepath.u8string()));
    }
}

void LoadHandler::closeFile() noexcept {
    this->xmlContentStream.reset();
    this->zipFp.reset();
}

void LoadHandler::parseXml() {
    xoj_assert(this->doc);
    xoj_assert(this->xmlContentStream);

    XmlParser parser(*this->xmlContentStream, this);
    auto res = parser.parse();

    if (res != 0) {
        logError("File is not finished after end of document");
    }
    if (!this->parsingComplete) {
        throw std::runtime_error(_("Document is not complete (maybe the end is cut off?)"));
    }
    if (this->parsingComplete && this->doc->getPageCount() == 0) {
        throw std::runtime_error(_("Document is corrupted (no pages found in file)"));
    }

    doc->setCreateBackupOnSave(true);
}


auto LoadHandler::loadDocument(fs::path const& filepath) -> std::unique_ptr<Document> {
    initAttributes();
    this->doc = std::make_unique<Document>(&dHanlder);

    try {
        openFile(filepath);
        parseXml();
    } catch (const std::exception& e) {
        // Loading failed in an unrecoverable way
        closeFile();
        this->doc.reset();
        g_warning("LoadHandler: Failed to load document. Error message: %s", e.what());
        this->errorMessages.emplace_back(e.what());
        return nullptr;
    }


    if (this->fileVersion == 1 || hasErrorMessages()) {
        // Either, this file was created by Xournal, not Xournal++, or loading
        // the file failed to some extent (i.e. file is corrupt or uses unknown
        // features). In all cases, do not set the doc's filename, in order to
        // prompt a "Save as" dialog when the user saves the document. In this
        // way, we do not silently make the file will become incompatible with
        // Xournal, nor erase data that we couldn't parse.

        this->doc->setFilepath("");
    } else {
        this->doc->setFilepath(filepath);
    }

    closeFile();

    return std::move(this->doc);
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
        const auto nValidPressureValues = static_cast<size_t>(std::distance(nextPositive, nextNonPositive));

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
                this->layer->addElement(std::exchange(this->stroke, std::move(newStroke)));
            });
}

auto LoadHandler::readZipAttachment(fs::path const& filename) -> std::unique_ptr<std::string> {
    zip_stat_t attachmentFileStat;
    const int statStatus = zip_stat(this->zipFp.get(), filename.u8string().c_str(), 0, &attachmentFileStat);
    if (statStatus != 0) {
        logError(FS(PlaceholderString("Could not open attachment: {1}. Error message: {2}") % filename.u8string() %
                    zip_error_strerror(zip_get_error(this->zipFp.get()))));
        return {};
    }

    if (!(attachmentFileStat.valid & ZIP_STAT_SIZE)) {
        logError(FS(PlaceholderString("Could not open attachment: {1}. Error message: No valid file size provided") %
                    filename.u8string()));
        return {};
    }
    const zip_uint64_t length = attachmentFileStat.size;

    auto attachmentFile = zip_file_wrapper(zip_fopen(this->zipFp.get(), filename.u8string().c_str(), 0));
    if (!attachmentFile) {
        logError(FS(PlaceholderString("Could not open attachment: {1}. Error message: {2}") % filename.u8string() %
                    zip_error_strerror(zip_get_error(this->zipFp.get()))));
        return {};
    }

    auto data = std::make_unique<std::string>(length, 0);
    zip_uint64_t readBytes = 0;
    while (readBytes < length) {
        const zip_int64_t read = zip_fread(attachmentFile.get(), data->data() + readBytes, length - readBytes);
        if (read == -1) {
            logError(
                    FS(PlaceholderString("Could not open attachment: {1}. Error message: No valid file size provided") %
                       filename.u8string()));
            return {};
        }

        readBytes += static_cast<zip_uint64_t>(read);
    }

    return data;
}

void LoadHandler::setAudioAttributes(AudioElement& elem, fs::path filename, size_t timestamp) {
    if (!filename.empty()) {
        if (this->isGzFile) {
            elem.setAudioFilename(std::move(filename));
        } else {
            auto tempFile = getTempFileForPath(filename);
            if (!tempFile.empty()) {
                elem.setAudioFilename(std::move(tempFile));
            }
        }

        if (this->fileVersion < 4) {
            timestamp *= 1000;
        }
        elem.setTimestamp(timestamp);
    }
}

auto LoadHandler::getTempFileForPath(fs::path const& filename) -> fs::path {
    auto it = this->audioFiles.find(filename);

    if (it == this->audioFiles.end()) {
        logError(FS(PlaceholderString("Requested temporary file was not found for attachment {1}") %
                    filename.u8string()));
        return {};
    } else {
        return it->second;
    }
}

auto LoadHandler::getAbsoluteFilepath(const fs::path& filename, bool attach) const -> fs::path {
    fs::path absolutePath;

    if (attach) {
        // For a file xyz.xopp, the PDF background is saved in the same
        // directory as xyz.xopp.[filename]
        absolutePath = (fs::path(this->xournalFilepath) += ".") += (filename);
    } else {
        // domain = "absolute" doesn't forcefully mean an absolute path
        if (filename.is_relative()) {
            // The path is relative to the .xopp file's location
            absolutePath = fs::path(this->xournalFilepath).remove_filename() / filename;
        } else {
            absolutePath = filename;
        }
    }
    return absolutePath;
}

void LoadHandler::logError(std::string&& error) {
    this->errorMessages.emplace_back(std::move(error));
    g_warning("%s", this->errorMessages.back().c_str());
}
