#include "Mask.h"

#include <cassert>
#include <cmath>
#include <iostream>

#include <cairo.h>

#include "util/Range.h"

#include "config-debug.h"

using namespace xoj::view;

#ifdef DEBUG_MASKS
#include <string>
namespace {  // cairo helper functions, for handling various surface types
std::string getSurfaceTypeName(cairo_surface_t*);
};
#define IF_DBG_MASKS(f) f
#else
#define IF_DBG_MASKS(f)
#endif

Mask::Mask(cairo_surface_t* target, const Range& extent, double zoom, cairo_content_t contentType):
        xOffset(static_cast<int>(std::floor(extent.minX * zoom))),
        yOffset(static_cast<int>(std::floor(extent.minY * zoom))),
        zoom(zoom) {
    constructorImpl(target, extent, zoom, contentType);
}

Mask::Mask(int DPIScaling, const Range& extent, double zoom, cairo_content_t contentType):
        xOffset(static_cast<int>(std::floor(extent.minX * zoom))),
        yOffset(static_cast<int>(std::floor(extent.minY * zoom))),
        zoom(zoom) {
    constructorImpl(DPIScaling, extent, zoom, contentType);
}

template <typename DPIInfoType>
class SurfaceCreator {};
template <>
class SurfaceCreator<cairo_surface_t*> {
public:
    static constexpr auto create = cairo_surface_create_similar;
};
template <>
class SurfaceCreator<int> {
public:
    static cairo_surface_t* create(int DPIScaling, cairo_content_t contentType, int width, int height) {
        cairo_surface_t* surf =
                cairo_image_surface_create(contentType == CAIRO_CONTENT_ALPHA ? CAIRO_FORMAT_A8 : CAIRO_FORMAT_ARGB32,
                                           width * DPIScaling, height * DPIScaling);
        cairo_surface_set_device_scale(surf, DPIScaling, DPIScaling);
        return surf;
    }
};

template <typename DPIInfoType>
void Mask::constructorImpl(DPIInfoType dpiInfo, const Range& extent, double zoom, cairo_content_t contentType) {
    assert(dpiInfo);
    assert(extent.isValid() ||
           ((std::cout << "Invalid range in Mask(): X  " << extent.minX << " -- " << extent.maxX << std::endl
                       << "                         Y  " << extent.minY << " -- " << extent.maxY << std::endl) &&
            false));
    assert(zoom > 0.0);

    const int width = static_cast<int>(std::ceil(extent.maxX * zoom)) - xOffset;
    const int height = static_cast<int>(std::ceil(extent.maxY * zoom)) - yOffset;

    /*
     * Create the most suitable kind of surface.
     *
     * Note that width and height are in device space coordinates (i.e. before DPI scaling).
     * If `dpiInfo` asks for a device scaling, then the number of pixels in the resulting surface will be multiplied
     * accordingly, and the scaling is applied to the new surface.
     */
    cairo_surface_t* surf = SurfaceCreator<DPIInfoType>::create(dpiInfo, contentType, width, height);

    IF_DBG_MASKS({
        std::cout << "Creating mask of type: " << getSurfaceTypeName(surf) << std::endl;
        std::cout << "  Its size: " << width << " x " << height << " (in device space)" << std::endl;
        double x;
        double y;
        cairo_surface_get_device_scale(surf, &x, &y);
        std::cout << "  Its DPI scaling: " << x << " x " << y << std::endl;
    });

    this->cr.reset(cairo_create(surf), xoj::util::adopt);
    cairo_surface_destroy(surf);  // surf is now owned by this->cr

    cairo_translate(this->cr.get(), -xOffset, -yOffset);
    cairo_scale(this->cr.get(), zoom, zoom);

    IF_DBG_MASKS({
        xoj::util::CairoSaveGuard saveGuard(cr.get());
        cairo_set_operator(cr.get(), CAIRO_OPERATOR_OVER);
        cairo_set_source_rgba(cr.get(), 1.0, 0.0, 0.0, 0.3);
        cairo_paint(cr.get());
    });
}

auto Mask::get() -> cairo_t* { return cr.get(); }

bool Mask::isInitialized() const { return cr; }

void Mask::blitTo(cairo_t* targetCr) const {
    assert(isInitialized());
    xoj::util::CairoSaveGuard guard(targetCr);
    cairo_scale(targetCr, 1. / zoom, 1. / zoom);
    cairo_mask_surface(targetCr, cairo_get_target(const_cast<cairo_t*>(cr.get())), xOffset, yOffset);
}

void Mask::paintTo(cairo_t* targetCr) const {
    assert(isInitialized());
    xoj::util::CairoSaveGuard guard(targetCr);
    cairo_scale(targetCr, 1. / zoom, 1. / zoom);
    cairo_set_source_surface(targetCr, cairo_get_target(const_cast<cairo_t*>(cr.get())), xOffset, yOffset);
    cairo_paint(targetCr);
}

void Mask::wipe() {
    assert(isInitialized());
    xoj::util::CairoSaveGuard saveGuard(cr.get());
    cairo_set_operator(cr.get(), CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr.get());
    IF_DBG_MASKS({
        cairo_set_operator(cr.get(), CAIRO_OPERATOR_OVER);
        cairo_set_source_rgba(cr.get(), 1.0, 0.0, 0.0, 0.3);
        cairo_paint(cr.get());
    });
}

void Mask::wipeRange(const Range& rg) {
    assert(isInitialized());
    xoj::util::CairoSaveGuard saveGuard(cr.get());
    cairo_rectangle(cr.get(), rg.minX, rg.minY, rg.getWidth(), rg.getHeight());
    cairo_clip(cr.get());
    wipe();
}

void Mask::reset() { cr.reset(); }

#ifdef DEBUG_MASKS
namespace {
auto getSurfaceTypeName(cairo_surface_t* surf) -> std::string {
    auto surftype = cairo_surface_get_type(surf);
    switch (surftype) {
        // Strings from https://cairographics.org/manual/cairo-cairo-surface-t.html#cairo-surface-type-t
        case CAIRO_SURFACE_TYPE_IMAGE:
            return "image";
        case CAIRO_SURFACE_TYPE_PDF:
            return "pdf";
        case CAIRO_SURFACE_TYPE_PS:
            return "ps";
        case CAIRO_SURFACE_TYPE_XLIB:
            return "xlib";
        case CAIRO_SURFACE_TYPE_XCB:
            return "xcb";
        case CAIRO_SURFACE_TYPE_GLITZ:
            return "glitz";
        case CAIRO_SURFACE_TYPE_QUARTZ:
            return "quartz";
        case CAIRO_SURFACE_TYPE_WIN32:
            return "win32";
        case CAIRO_SURFACE_TYPE_BEOS:
            return "beos";
        case CAIRO_SURFACE_TYPE_DIRECTFB:
            return "directfb";
        case CAIRO_SURFACE_TYPE_SVG:
            return "svg";
        case CAIRO_SURFACE_TYPE_OS2:
            return "os2";
        case CAIRO_SURFACE_TYPE_WIN32_PRINTING:
            return "win32 printing surface";
        case CAIRO_SURFACE_TYPE_QUARTZ_IMAGE:
            return "quartz_image";
        case CAIRO_SURFACE_TYPE_SCRIPT:
            return "script";
        case CAIRO_SURFACE_TYPE_QT:
            return "Qt";
        case CAIRO_SURFACE_TYPE_RECORDING:
            return "recording";
        case CAIRO_SURFACE_TYPE_VG:
            return "OpenVG surface";
        case CAIRO_SURFACE_TYPE_GL:
            return "OpenGL";
        case CAIRO_SURFACE_TYPE_DRM:
            return "Direct Render Manager";
        case CAIRO_SURFACE_TYPE_TEE:
            return "'tee' (a multiplexing surface)";
        case CAIRO_SURFACE_TYPE_XML:
            return "XML (for debugging)";
        case CAIRO_SURFACE_TYPE_SKIA:
            return "CAIRO_SURFACE_TYPE_SKIA";
        case CAIRO_SURFACE_TYPE_SUBSURFACE:
            return "subsurface created with cairo_surface_create_for_rectangle()";
        case CAIRO_SURFACE_TYPE_COGL:
            return "Cogl";
        default:
            return "Unknown surface type";
    }
}
};  // namespace
#endif
