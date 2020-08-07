#include "GzUtil.h"

auto GzUtil::openPath(const fs::path& path, const std::string& flags) -> gzFile {
#ifdef _WIN32
    gzFile fp = gzopen_w(path.c_str(), flags.c_str());

    return fp;
#else
    return gzopen(path.c_str(), flags.c_str());
#endif
}
