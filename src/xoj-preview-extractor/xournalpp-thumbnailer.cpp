/*
 * Xournal++
 *
 * This small program extracts a preview out of a xoj file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GPL
 */

// Set to true to write a log with errors and debug logs to /tmp/xojtmb.log
#include "filesystem.h"
#define DEBUG_THUMBNAILER false


#include <algorithm>
#include <fstream>
#include <iostream>

#include <config-paths.h>
#include <config.h>

#include "XojPreviewExtractor.h"
#include "i18n.h"
using std::cerr;
using std::cout;
using std::endl;
#include <cairo-svg.h>
#include <cairo.h>
#include <librsvg/rsvg.h>

void initLocalisation() {
#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    textdomain(GETTEXT_PACKAGE);
#endif  // ENABLE_NLS

    std::locale::global(std::locale(""));  //"" - system default locale
    std::cout.imbue(std::locale());
}

void logMessage(string msg, bool error) {
    if (error) {
        cerr << msg << endl;
    } else {
        cout << msg << endl;
    }

#if DEBUG_THUMBNAILER
    std::ofstream ofs;
    ofs.open("/tmp/xojtmb.log", std::ofstream::out | std::ofstream::app);

    if (error) {
        ofs << "E: ";
    } else {
        ofs << "I: ";
    }

    ofs << msg << endl;

    ofs.close();
#endif
}

static const std::string iconName = "com.github.xournalpp.xournalpp";

/**
 * Search for Xournal++ icon based on the freedesktop icon theme specification
 */
fs::path findAppIcon() {
    std::vector<fs::path> basedirs;
#if DEBUG_THUMBNAILER
    basedirs.emplace_back(fs::u8path("../ui/pixmaps"));
#endif
    // $HOME/.icons
    basedirs.emplace_back(fs::u8path(g_get_home_dir()) / ".icons");
    // $XDG_DATA_DIRS/icons
    if (const char* datadirs = g_getenv("XDG_DATA_DIRS")) {
        std::string dds = datadirs;
        std::string::size_type lastp = 0;
        std::string::size_type p;
        while ((p = dds.find(":", lastp)) != std::string::npos) {
            std::string path = dds.substr(lastp, p - lastp);
            basedirs.emplace_back(fs::u8path(path) / "icons");
            lastp = p + 1;
        }
    }
    basedirs.emplace_back(fs::u8path("/usr/share/pixmaps"));

    const auto iconFile = iconName + ".svg";
    // Search through base directories
    for (auto&& d: basedirs) {
        fs::path svgPath;
        if (fs::exists((svgPath = d / "hicolor/scalable/apps" / iconFile))) {
            return svgPath;
        } else if (fs::exists((svgPath = d / iconFile))) {
            return svgPath;
        }
    }

    return "";
}

int main(int argc, char* argv[]) {
    initLocalisation();

    // check args count
    if (argc != 3) {
        logMessage(_("xoj-preview-extractor: call with INPUT.xoj OUTPUT.png"), true);
        return 1;
    }

    XojPreviewExtractor extractor;
    PreviewExtractResult result = extractor.readFile(argv[1]);

    switch (result) {
        case PREVIEW_RESULT_IMAGE_READ:
            // continue to write preview
            break;

        case PREVIEW_RESULT_BAD_FILE_EXTENSION:
            logMessage((_F("xoj-preview-extractor: file \"{1}\" is not .xoj file") % argv[1]).str(), true);
            return 2;

        case PREVIEW_RESULT_COULD_NOT_OPEN_FILE:
            logMessage((_F("xoj-preview-extractor: opening input file \"{1}\" failed") % argv[1]).str(), true);
            return 3;

        case PREVIEW_RESULT_NO_PREVIEW:
            logMessage((_F("xoj-preview-extractor: file \"{1}\" contains no preview") % argv[1]).str(), true);
            return 4;

        case PREVIEW_RESULT_ERROR_READING_PREVIEW:
        default:
            logMessage(_("xoj-preview-extractor: no preview and page found, maybe an invalid file?"), true);
            return 5;
    }


    gsize dataLen = 0;
    unsigned char* imageData = extractor.getData(dataLen);

    // The following code is for rendering the Xournal++ icon on top of thumbnails.

    // Struct for reading imageData into a cairo surface
    struct ReadClosure {
        unsigned int pos;
        unsigned char* data;
        gsize maxLen;
    };
    cairo_read_func_t processRead =
            (cairo_read_func_t) + [](ReadClosure* closure, unsigned char* data, unsigned int length) {
                if (closure->pos + length > closure->maxLen) {
                    return CAIRO_STATUS_READ_ERROR;
                }

                for (auto i = 0; i < length; i++) {
                    data[i] = closure->data[closure->pos + i];
                }
                closure->pos += length;
                return CAIRO_STATUS_SUCCESS;
            };
    ReadClosure closure{0, imageData, dataLen};
    cairo_surface_t* thumbnail = cairo_image_surface_create_from_png_stream(processRead, &closure);
    // This application is short-lived, so we'll purposefully be sloppy and let the OS free memory.
    if (cairo_surface_status(thumbnail) == CAIRO_STATUS_SUCCESS) {
        GError* err = nullptr;
        const auto width = cairo_image_surface_get_width(thumbnail);
        const auto height = cairo_image_surface_get_height(thumbnail);
        const auto iconSize = 0.5 * std::min(width, height);

        const auto svgPath = findAppIcon();
        RsvgHandle* handle = rsvg_handle_new_from_file(svgPath.c_str(), &err);
        if (err) {
            logMessage((_F("xoj-preview-extractor: could not find icon \"{1}\"") % iconName).str(), true);
        } else {
            rsvg_handle_set_dpi(handle, 90);  // does the dpi matter for an icon overlay?
            RsvgDimensionData dims;
            rsvg_handle_get_dimensions(handle, &dims);

            // Render at bottom right
            cairo_t* cr = cairo_create(thumbnail);
            cairo_translate(cr, width - iconSize, height - iconSize);
            cairo_scale(cr, iconSize / dims.width, iconSize / dims.height);
            rsvg_handle_render_cairo(handle, cr);
        }
        cairo_surface_write_to_png(thumbnail, argv[2]);
    } else {
        // Cairo was unable to load the image, so fallback to writing the PNG data to disk.
        FILE* fp = fopen(argv[2], "wb");
        if (!fp) {
            logMessage((_F("xoj-preview-extractor: opening output file \"{1}\" failed") % argv[2]).str(), true);
            return 6;
        }
        fwrite(imageData, dataLen, 1, fp);
        fclose(fp);
    }

    logMessage(_("xoj-preview-extractor: successfully extracted"), false);
    return 0;
}
