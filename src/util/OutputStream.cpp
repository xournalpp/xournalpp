#include "util/OutputStream.h"

#include <cassert>
#include <cerrno>
#include <cstring>  // for strlen
#include <utility>  // for move

#include "util/GzUtil.h"  // for GzUtil
#include "util/i18n.h"    // for FS, _F
#include "util/safe_casts.h"

OutputStream::OutputStream() = default;

OutputStream::~OutputStream() = default;

void OutputStream::write(const std::string& str) { write(str.c_str(), str.length()); }

void OutputStream::write(const char* str) { write(str, std::strlen(str)); }

////////////////////////////////////////////////////////
/// GzOutputStream /////////////////////////////////////
////////////////////////////////////////////////////////

GzOutputStream::GzOutputStream(fs::path file): file(std::move(file)) {
    this->fp = GzUtil::openPath(this->file, "w");
    if (this->fp == nullptr) {
        this->error = FS(_F("Error opening file: \"{1}\"") % this->file.u8string());
        this->error = this->error + "\n" + std::strerror(errno);
    }
}

GzOutputStream::~GzOutputStream() {
    if (this->fp) {
        close();
    }
    this->fp = nullptr;
}

auto GzOutputStream::getLastError() const -> const std::string& { return this->error; }

void GzOutputStream::write(const char* data, size_t len) {
    assert(len != 0 && this->fp);
    auto written = gzwrite(this->fp, data, strict_cast<unsigned int>(len));
    if (as_unsigned(written) != len) {
        int errnum = 0;
        const char* error = gzerror(this->fp, &errnum);
        if (errnum != Z_OK) {
            this->error = FS(_F("Error writing data to file: \"{1}\"") % this->file.u8string());
            this->error += "\n" + FS(_F("Error code {1}. Message:") % errnum) + "\n";
            if (errnum == Z_ERRNO) {
                // fs error. Fetch the precise message
                this->error += std::strerror(errno);
            } else {
                this->error += error;
            }
        }
    }
}

void GzOutputStream::close() {
    if (!this->fp) {
        return;
    }

    auto errnum = gzclose(this->fp);
    this->fp = nullptr;

    if (errnum != Z_OK) {
        this->error = FS(_F("Error occurred while closing file: \"{1}\"") % this->file.u8string());
        this->error += "\n" + FS(_F("Error code {1}") % errnum);
        if (errnum == Z_ERRNO) {
            // fs error. Fetch the precise message
            this->error = this->error + "\n" + std::strerror(errno);
        }
    }
}
