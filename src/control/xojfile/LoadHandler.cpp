#include "LoadHandler.h"

#include <cstdlib>
#include <utility>

#include <config.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include "control/pagetype/PageTypeHandler.h"
#include "model/BackgroundImage.h"
#include "model/StrokeStyle.h"
#include "model/XojPage.h"

#include "GzUtil.h"
#include "LoadHandlerHelper.h"
#include "i18n.h"

#define error2(var, ...)                                                                \
    if (var == nullptr) {                                                               \
        var = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__); \
    }

#define error(...)                                                                        \
    if (error == nullptr) {                                                               \
        error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__); \
    }

LoadHandler::LoadHandler():
        attachedPdfMissing(false),
        removePdfBackgroundFlag(false),
        pdfReplacementAttach(false),
        pdfFilenameParsed(false),
        pos(PARSER_POS_NOT_STARTED),
        fileVersion(0),
        minimalFileVersion(0),
        zipFp(nullptr),
        zipContentFile(nullptr),
        gzFp(nullptr),
        layer(nullptr),
        stroke(nullptr),
        text(nullptr),
        image(nullptr),
        teximage(nullptr),
        attributeNames(nullptr),
        attributeValues(nullptr),
        elementName(nullptr),
        loadedTimeStamp(0),
        doc(&dHanlder) {
    this->error = nullptr;

    initAttributes();
}

LoadHandler::~LoadHandler() {
    if (this->audioFiles) {
        g_hash_table_unref(this->audioFiles);
    }
}

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

    this->page = nullptr;
    this->layer = nullptr;
    this->stroke = nullptr;
    this->image = nullptr;
    this->teximage = nullptr;
    this->text = nullptr;
    this->pages.clear();

    if (this->audioFiles) {
        g_hash_table_unref(this->audioFiles);
    }
    this->audioFiles = g_hash_table_new(g_str_hash, g_str_equal);
}

auto LoadHandler::getLastError() -> string { return this->lastError; }

auto LoadHandler::isAttachedPdfMissing() const -> bool { return this->attachedPdfMissing; }

auto LoadHandler::getMissingPdfFilename() -> string { return this->pdfMissing; }

void LoadHandler::removePdfBackground() { this->removePdfBackgroundFlag = true; }

void LoadHandler::setPdfReplacement(fs::path filepath, bool attachToDocument) {
    this->pdfReplacementFilepath = std::move(filepath);
    this->pdfReplacementAttach = attachToDocument;
}

auto LoadHandler::openFile(fs::path const& filepath) -> bool {
    this->filepath = filepath;
    int zipError = 0;
    this->zipFp = zip_open(filepath.u8string().c_str(), ZIP_RDONLY, &zipError);

    // Check if the file is actually an old XOPP-File and open it
    if (!this->zipFp && zipError == ZIP_ER_NOZIP) {
        this->gzFp = GzUtil::openPath(filepath, "r");
        this->isGzFile = true;
    }

    if (this->zipFp && !this->isGzFile) {
        // Check the mimetype
        zip_file_t* mimetypeFp = zip_fopen(this->zipFp, "mimetype", 0);
        if (!mimetypeFp) {
            this->lastError = zip_error_strerror(zip_get_error(zipFp));
            this->lastError = FS(_F("The file is no valid .xopp file (Mimetype missing): \"{1}\"") % filepath.string());
            return false;
        }
        char mimetype[25];
        // read the mimetype and a few more bytes to make sure we do not only read a subset
        zip_fread(mimetypeFp, mimetype, 25);
        if (!strcmp(mimetype, "application/xournal++")) {
            this->lastError = FS(_F("The file is no valid .xopp file (Mimetype wrong): \"{1}\"") % filepath.string());
            return false;
        }
        zip_fclose(mimetypeFp);

        // Get the file version
        zip_file_t* versionFp = zip_fopen(this->zipFp, "META-INF/version", 0);
        if (!versionFp) {
            this->lastError = FS(_F("The file is no valid .xopp file (Version missing): \"{1}\"") % filepath.string());
            return false;
        }
        char versionString[50];
        zip_fread(versionFp, versionString, 50);
        std::string versions(versionString);
        std::regex versionRegex("current=(\\d+?)(?:\n|\r\n)min=(\\d+?)");
        std::smatch match;
        if (std::regex_search(versions, match, versionRegex)) {
            this->fileVersion = std::stoi(match.str(1));
            this->minimalFileVersion = std::stoi(match.str(2));
        } else {
            this->lastError = FS(_F("The file is not a valid .xopp file (Version string corrupted): \"{1}\"") %
                                 filepath.string());
            return false;
        }
        zip_fclose(versionFp);

        // open the main content file
        this->zipContentFile = zip_fopen(this->zipFp, "content.xml", 0);
    }

    // Fail if neither utility could open the file
    if (!this->zipFp && !this->gzFp) {
        this->lastError = FS(_F("Could not open file: \"{1}\"") % filepath.string());
        return false;
    }
    return true;
}

auto LoadHandler::closeFile() -> bool {
    if (this->isGzFile) {
        return static_cast<bool>(gzclose(this->gzFp));
    }

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

    zip_int64_t lengthRead = zip_fread(this->zipContentFile, buffer, len);
    if (lengthRead > 0) {
        return lengthRead;
    }

    return -1;
}

auto LoadHandler::parseXml() -> bool {
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
    this->doc.addPages(pages.begin(), pages.end());

    if (this->pos != PASER_POS_FINISHED && this->lastError.empty()) {
        lastError = _("Document is not complete (maybe the end is cut off?)");
        return false;
    }
    if (this->pos == PASER_POS_FINISHED && this->doc.getPageCount() == 0) {
        lastError = _("Document is corrupted (no pages found in file)");
        return false;
    }

    doc.setCreateBackupOnSave(this->fileVersion >= 3);

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

        this->page = std::make_unique<XojPage>(width, height);

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
}

void LoadHandler::parseBgPixmap() {
    const char* domain = LoadHandlerHelper::getAttrib("domain", false, this);
    const fs::path filepath(LoadHandlerHelper::getAttrib("filename", false, this));

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
            error("%s", FC(_F("Could not read image: {1}. Error message: {2}") % fileToLoad.string() % error->message));
            g_error_free(error);
        }

        this->page->setBackgroundImage(img);
    } else if (!strcmp(domain, "attach")) {
        // This is the new zip file attach domain
        gpointer data = nullptr;
        gsize dataLength = 0;
        bool success = readZipAttachment(filepath.string(), data, dataLength);
        if (!success) {
            return;
        }

        GBytes* attachment = g_bytes_new_take(data, dataLength);
        GInputStream* inputStream = g_memory_input_stream_new_from_bytes(attachment);

        GError* error = nullptr;
        BackgroundImage img;
        img.loadFile(inputStream, filepath, &error);

        g_input_stream_close(inputStream, nullptr, nullptr);

        if (error) {
            error("%s", FC(_F("Could not read image: {1}. Error message: {2}") % filepath.string() % error->message));
            g_error_free(error);
        }

        this->page->setBackgroundImage(img);
    } else if (!strcmp(domain, "clone")) {
        PageRef p = doc.getPage(stoull(filepath.string()));

        if (p) {
            this->page->setBackgroundImage(p->getBackgroundImage());
        }
    } else {
        error("%s", FC(_F("Unknown pixmap::domain type: {1}") % domain));
    }

    this->page->setBackgroundType(PageType(PageTypeFormat::Image));
}

void LoadHandler::parseBgPdf() {
    int pageno = LoadHandlerHelper::getAttribInt("pageno", this);
    bool attachToDocument = false;
    string pdfFilename;

    this->page->setBackgroundPdfPageNr(pageno - 1);

    if (!this->pdfFilenameParsed) {

        if (this->pdfReplacementFilepath.empty()) {
            const char* domain = LoadHandlerHelper::getAttrib("domain", false, this);
            const char* sFilename = LoadHandlerHelper::getAttrib("filename", false, this);

            if (sFilename == nullptr) {
                error("PDF Filename missing!");
                return;
            }

            pdfFilename = sFilename;

            if (!strcmp("absolute", domain))  // Absolute OR relative path
            {
                if (!g_file_test(sFilename, G_FILE_TEST_EXISTS)) {
                    char* dirname = g_path_get_dirname(xournalFilepath.u8string().c_str());
                    char* file = g_path_get_basename(sFilename);

                    char* tmpFilename = g_build_path(G_DIR_SEPARATOR_S, dirname, file, nullptr);

                    if (g_file_test(tmpFilename, G_FILE_TEST_EXISTS)) {
                        pdfFilename = tmpFilename;
                    }

                    g_free(tmpFilename);
                    g_free(dirname);
                    g_free(file);
                }
            } else if (!strcmp("attach", domain)) {
                // Handle old format separately
                if (this->isGzFile) {
                    char* tmpFilename = g_strdup_printf("%s.%s", xournalFilepath.u8string().c_str(), sFilename);

                    if (g_file_test(tmpFilename, G_FILE_TEST_EXISTS)) {
                        pdfFilename = tmpFilename;
                    }

                    g_free(tmpFilename);
                } else {
                    gpointer data = nullptr;
                    gsize dataLength = 0;
                    bool success = readZipAttachment(pdfFilename, data, dataLength);
                    if (!success) {
                        return;
                    }

                    doc.readPdf(pdfFilename, false, attachToDocument, data, dataLength);

                    if (!doc.getLastErrorMsg().empty()) {
                        error("%s", FC(_F("Error reading PDF: {1}") % doc.getLastErrorMsg()));
                    }

                    this->pdfFilenameParsed = true;
                    return;
                }
            } else {
                error("%s", FC(_F("Unknown domain type: {1}") % domain));
                return;
            }

        } else {
            pdfFilename = this->pdfReplacementFilepath.string();
            attachToDocument = this->pdfReplacementAttach;
        }

        this->pdfFilenameParsed = true;

        if (g_file_test(pdfFilename.c_str(), G_FILE_TEST_EXISTS)) {
            doc.readPdf(pdfFilename, false, attachToDocument);
            if (!doc.getLastErrorMsg().empty()) {
                error("%s", FC(_F("Error reading PDF: {1}") % doc.getLastErrorMsg()));
            }
        } else {
            if (attachToDocument) {
                this->attachedPdfMissing = true;
            } else {
                this->pdfMissing = pdfFilename;
            }
        }
    }
}

void LoadHandler::parsePage() {
    if (!strcmp(elementName, "background")) {
        const char* type = LoadHandlerHelper::getAttrib("type", false, this);

        if (strcmp("solid", type) == 0) {
            parseBgSolid();
        } else if (strcmp("pixmap", type) == 0) {
            parseBgPixmap();
        } else if (strcmp("pdf", type) == 0) {
            if (this->removePdfBackgroundFlag) {
                this->page->setBackgroundType(PageType(PageTypeFormat::Plain));
                this->page->setBackgroundColor(0xffffffU);
            } else {
                parseBgPdf();
            }
        } else {
            error("%s", FC(_F("Unknown background type: {1}") % type));
        }
    } else if (!strcmp(elementName, "layer")) {
        this->pos = PARSER_POS_IN_LAYER;
        this->layer = new Layer();
        this->page->addLayer(this->layer);
    }
}

void LoadHandler::parseStroke() {
    this->stroke = new Stroke();
    this->layer->addElement(this->stroke);

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
            stroke->setAudioFilename(fn);
        } else {
            auto tempFile = getTempFileForPath(fn);
            if (!tempFile.empty()) {
                stroke->setAudioFilename(tempFile.string());
            }
        }
    }


    if (this->fileVersion < 4) {
        int ts = 0;
        if (LoadHandlerHelper::getAttribInt("ts", true, this, ts)) {
            stroke->setTimestamp(ts * 1000);
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

    const char* style = LoadHandlerHelper::getAttrib("style", true, this);
    if (style != nullptr) {
        stroke->setLineStyle(StrokeStyle::parseStyle(style));
    }

    const char* tool = LoadHandlerHelper::getAttrib("tool", false, this);

    if (strcmp("eraser", tool) == 0) {
        stroke->setToolType(STROKE_TOOL_ERASER);
    } else if (strcmp("pen", tool) == 0) {
        stroke->setToolType(STROKE_TOOL_PEN);
    } else if (strcmp("highlighter", tool) == 0) {
        stroke->setToolType(STROKE_TOOL_HIGHLIGHTER);
    } else {
        g_warning("%s", FC(_F("Unknown stroke type: \"{1}\", assuming pen") % tool));
    }

    /**
     * For each stroke being read, set the timestamp value
     * we've read just before.
     * Afterwards, clean the read timestamp data.
     */
    if (loadedFilename.length() != 0) {
        this->stroke->setTimestamp(loadedTimeStamp);
        this->stroke->setAudioFilename(loadedFilename);
        loadedFilename = "";
        loadedTimeStamp = 0;
    }
}

void LoadHandler::parseText() {
    this->text = new Text();
    this->layer->addElement(this->text);

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
            text->setAudioFilename(fn);
        } else {
            auto tempFile = getTempFileForPath(fn);
            if (!tempFile.empty()) {
                text->setAudioFilename(tempFile.string());
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

    this->image = new Image();
    this->layer->addElement(this->image);
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
    int imTextLen = strlen(imText);
    if (compatibilityTest != nullptr) {
        imTextLen = LoadHandlerHelper::getAttribInt("texlength", this);
    }

    this->teximage = new TexImage();
    this->layer->addElement(this->teximage);
    this->teximage->setX(left);
    this->teximage->setY(top);
    this->teximage->setWidth(right - left);
    this->teximage->setHeight(bottom - top);

    this->teximage->setText(string(imText, imTextLen));
}

void LoadHandler::parseAttachment() {
    if (this->pos != PARSER_POS_IN_IMAGE || this->pos != PARSER_POS_IN_TEXIMAGE) {
        g_warning("Found attachment tag as child of a tag that should not have such a child (ignoring this tag)");
        return;
    }
    const char* path = LoadHandlerHelper::getAttrib("path", false, this);

    // Todo(fabian) move the following 4 lines into readZipAttachment
    gpointer data = nullptr;
    gsize dataLength{0};
    string imgData = readZipAttachment(path, data, dataLength) ? string{static_cast<char*>(data), dataLength} : "";
    g_free(data);

    switch (this->pos) {
        case PARSER_POS_IN_IMAGE: {
            this->image->setImage(imgData);
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
    GFile* tmpFile = g_file_new_tmp("xournal_audio_XXXXXX.tmp", &fileStream, nullptr);
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
            g_object_unref(tmpFile);
            g_free(data);
            error("%s", FC(_F("Could not open attachment: {1}. Error message: Could not read file") % filename));
            return;
        }

        gboolean writeSuccessful =
                g_output_stream_write_all(outputStream, data, static_cast<gsize>(read), nullptr, nullptr, nullptr);
        if (!writeSuccessful) {
            g_object_unref(tmpFile);
            g_free(data);
            error("%s", FC(_F("Could not open attachment: {1}. Error message: Could not write file") % filename));
            return;
        }

        readBytes += read;
    }
    g_free(data);
    zip_fclose(attachmentFile);

    g_hash_table_insert(this->audioFiles, g_strdup(filename), g_file_get_path(tmpFile));
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
        handler->pos = PARSER_POS_STARTED;
        handler->page = nullptr;
    } else if (handler->pos == PARSER_POS_IN_LAYER && strcmp(elementName, "layer") == 0) {
        handler->pos = PARSER_POS_IN_PAGE;
        handler->layer = nullptr;
    } else if (handler->pos == PARSER_POS_IN_LAYER && strcmp(elementName, "timestamp") == 0) {
        /** Used for backwards compatibility against xoj files with timestamps) */
        handler->pos = PARSER_POS_IN_LAYER;
        handler->stroke = nullptr;
    } else if (handler->pos == PARSER_POS_IN_STROKE && strcmp(elementName, "stroke") == 0) {
        handler->pos = PARSER_POS_IN_LAYER;
        handler->stroke = nullptr;
    } else if (handler->pos == PARSER_POS_IN_TEXT && strcmp(elementName, "text") == 0) {
        handler->pos = PARSER_POS_IN_LAYER;
        handler->text = nullptr;
    } else if (handler->pos == PARSER_POS_IN_IMAGE && strcmp(elementName, "image") == 0) {
        handler->pos = PARSER_POS_IN_LAYER;
        handler->image = nullptr;
    } else if (handler->pos == PARSER_POS_IN_TEXIMAGE && strcmp(elementName, "teximage") == 0) {
        handler->pos = PARSER_POS_IN_LAYER;
        handler->teximage = nullptr;
    }
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
            textLen -= (ptr - text);
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
            if (static_cast<int>(handler->pressureBuffer.size()) >= handler->stroke->getPointCount() - 1) {
                handler->stroke->setPressure(handler->pressureBuffer);
                handler->pressureBuffer.clear();
            } else {
                g_warning("%s", FC(_F("xoj-File: {1}") % handler->filepath.string().c_str()));
                g_warning("%s", FC(_F("Wrong number of points, got {1}, expected {2}") %
                                   handler->pressureBuffer.size() % (handler->stroke->getPointCount() - 1)));
            }
        }
        handler->pressureBuffer.clear();
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

auto LoadHandler::parseBase64(const gchar* base64, gsize lenght) -> string {
    // We have to copy the string in order to null terminate it, sigh.
    auto* base64data = static_cast<gchar*>(g_memdup(base64, lenght + 1));
    base64data[lenght] = '\0';

    gsize binaryBufferLen = 0;
    guchar* binaryBuffer = g_base64_decode(base64data, &binaryBufferLen);
    g_free(base64data);

    string str = string(reinterpret_cast<char*>(binaryBuffer), binaryBufferLen);
    g_free(binaryBuffer);

    return str;
}

void LoadHandler::readImage(const gchar* base64string, gsize base64stringLen) {
    if (base64stringLen == 1 && !strcmp(base64string, "\n")) {
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

/**
 * Document should not be freed, it will be freed with LoadHandler!
 */
auto LoadHandler::loadDocument(fs::path const& filepath) -> Document* {
    initAttributes();
    doc.clearDocument();

    if (!openFile(filepath)) {
        return nullptr;
    }

    xournalFilepath = filepath;

    this->pdfFilenameParsed = false;

    if (!parseXml()) {
        closeFile();
        return nullptr;
    }

    if (fileVersion == 1) {
        // This is a Xournal document, not a Xournal++
        // Even if the new fileextension is .xopp, allow to
        // overwrite .xoj files which are created by Xournal++
        // Force the user to save is a bad idea, this will annoy the user
        // Rename files is also not that user friendly.

        doc.setFilepath("");
    } else {
        doc.setFilepath(filepath);
    }

    closeFile();

    return &this->doc;
}

// Todo(fabian): return data and length by value not by reference, to ensure data and length is assigned always
//      return string not a pointer. Ownage is not clear!
auto LoadHandler::readZipAttachment(fs::path const& filename, gpointer& data, gsize& length) -> bool {
    zip_stat_t attachmentFileStat;
    int statStatus = zip_stat(this->zipFp, filename.u8string().c_str(), 0, &attachmentFileStat);
    if (statStatus != 0) {
        error("%s", FC(_F("Could not open attachment: {1}. Error message: {2}") % filename.string() %
                       zip_error_strerror(zip_get_error(this->zipFp))));
        return false;
    }

    if (attachmentFileStat.valid & ZIP_STAT_SIZE) {
        length = attachmentFileStat.size;
    } else {
        error("%s",
              FC(_F("Could not open attachment: {1}. Error message: No valid file size provided") % filename.string()));
        return false;
    }

    zip_file_t* attachmentFile = zip_fopen(this->zipFp, filename.u8string().c_str(), 0);

    if (!attachmentFile) {
        error("%s", FC(_F("Could not open attachment: {1}. Error message: {2}") % filename.string() %
                       zip_error_strerror(zip_get_error(this->zipFp))));
        return false;
    }

    data = g_malloc(attachmentFileStat.size);
    zip_uint64_t readBytes = 0;
    while (readBytes < length) {
        zip_int64_t read = zip_fread(attachmentFile, data, attachmentFileStat.size);
        if (read == -1) {
            g_free(data);
            error("%s", FC(_F("Could not open attachment: {1}. Error message: No valid file size provided") %
                           filename.string()));
            return false;
        }

        readBytes += read;
    }

    zip_fclose(attachmentFile);

    return true;
}

auto LoadHandler::getTempFileForPath(fs::path const& filename) -> fs::path {
    gpointer tmpFilename = g_hash_table_lookup(this->audioFiles, filename.u8string().c_str());
    if (tmpFilename) {
        return string(static_cast<char*>(tmpFilename));
    }

    error("%s", FC(_F("Requested temporary file was not found for attachment {1}") % filename.string()));
    return "";
}

auto LoadHandler::getFileVersion() const -> int { return this->fileVersion; }

auto LoadHandler::getFileMinVersion() const -> int { return this->minimalFileVersion; }
