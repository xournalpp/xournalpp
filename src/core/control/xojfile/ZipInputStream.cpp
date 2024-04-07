#include "control/xojfile/ZipInputStream.h"

#include <stdexcept>  // for runtime_error
#include <string>     // for string

#include <zip.h>

#include "util/Assert.h"  // for xoj_assert
#include "util/i18n.h"    // for FS, _F

#include "filesystem.h"  // for path


ZipInputStream::ZipInputStream(): file(nullptr) {}

ZipInputStream::ZipInputStream(zip_t* archive, const fs::path& filepath): file(nullptr) { open(archive, filepath); }

ZipInputStream::~ZipInputStream() {
    if (this->file) {
        zip_fclose(this->file);
    }
}


auto ZipInputStream::read(char* buffer, unsigned int len) noexcept -> int {
    return static_cast<int>(zip_fread(this->file, buffer, len));
}

void ZipInputStream::open(zip_t* archive, const fs::path& filepath) {
    xoj_assert(!this->file);
    this->file = zip_fopen(archive, filepath.string().c_str(), 0);

    if (!this->file) {
        const std::string error =
                FS(_F("Error opening file \"{1}\" in zip archive:\n{2}") % filepath.u8string() % zip_strerror(archive));
        throw std::runtime_error(error);
    }
}

void ZipInputStream::close() {
    if (!this->file) {
        return;
    }

    auto result = zip_fclose(this->file);
    this->file = nullptr;

    if (result != 0) {
        const std::string error = FS(_F("Error occurred while closing file\n"
                                        "Error code {1}") %
                                     result);
        throw std::runtime_error(error);
    }
}
