#include "control/xojfile/GzInputStream.h"

#include <cerrno>     // for errno
#include <cstring>    // for strerror
#include <stdexcept>  // for runtime_rrror
#include <string>     // for string

#include <zlib.h>  // for gzclose, gzread, gzeof, gzerror

#include "util/GzUtil.h"  // for GzUtil
#include "util/i18n.h"    // for FS, _F


GzInputStream::GzInputStream(): file(nullptr) {}

GzInputStream::GzInputStream(const fs::path& filepath): file(nullptr) { open(filepath); }

GzInputStream::~GzInputStream() {
    if (this->file) {
        gzclose(this->file);
    }
}


auto GzInputStream::read(char* buffer, unsigned int len) noexcept -> int {
    const int bytesRead = gzread(this->file, buffer, len);

    if (bytesRead > 0) {
        return bytesRead;
    } else if (gzeof(this->file)) {
        return 0;
    } else {
        return -1;
    }
}

void GzInputStream::open(const fs::path& filepath) {
    this->file = GzUtil::openPath(filepath, "r");

    if (!this->file) {
        const std::string error =
                FS(_F("Error opening file \"{1}\":\n{2}") % filepath.u8string() % std::strerror(errno));
        throw std::runtime_error(error);
    }
}

void GzInputStream::close() {
    if (!this->file) {
        return;
    }

    auto result = gzclose(this->file);
    this->file = nullptr;

    if (result != Z_OK) {
        std::string error = FS(_F("Error occurred while closing file\n"
                                  "Error code {1}") %
                               result);
        if (result == Z_ERRNO) {
            // fs error. Fetch the precise message
            error += std::string("\n") + std::strerror(errno);
        }
        throw std::runtime_error(error);
    }
}
