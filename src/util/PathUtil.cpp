#include "PathUtil.h"

#include <array>

#include <glib.h>
#include <glib/gstdio.h>

#include "XojMsgBox.h"

/**
 * Read a file to a string
 *
 * @param output Read contents
 * @param path Path to read
 * @param showErrorToUser Show an error to the user, if the file could not be read
 *
 * @return true if the file was read, false if not
 */
auto PathUtil::readString(string& output, Path& path, bool showErrorToUser) -> bool {
    gchar* contents = nullptr;
    gsize length = 0;
    GError* error = nullptr;
    if (g_file_get_contents(path.c_str(), &contents, &length, &error)) {
        output = contents;
        g_free(contents);
        return true;
    }


    if (showErrorToUser) {
        XojMsgBox::showErrorToUser(nullptr, error->message);
    }

    g_error_free(error);
    return false;
}

auto PathUtil::copy(const Path& src, const Path& dest) -> bool {
    std::array<char, 16 * 1024> buffer{};  // 16k

    FILE* fpRead = g_fopen(src.c_str(), "rbe");
    if (!fpRead) {
        return false;
    }

    FILE* fpWrite = g_fopen(dest.c_str(), "wbe");
    if (!fpWrite) {
        fclose(fpRead);
        return false;
    }

    while (!feof(fpRead)) {
        size_t bytes = fread(buffer.data(), 1, buffer.size(), fpRead);
        if (bytes) {
            fwrite(buffer.data(), 1, bytes, fpWrite);
        }
    }

    fclose(fpRead);
    fclose(fpWrite);

    return true;
}
