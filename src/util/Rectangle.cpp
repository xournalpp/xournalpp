#include "util/Rectangle.h"

#include <iostream>

namespace xoj::util {
auto operator<<(std::ostream& os, Rectangle<double> const& r) -> std::ostream& {
    return os << "Rectangle(" << r.x << ", " << r.y << ", " << r.width << ", " << r.height << ")";
}
}  // namespace xoj::util
