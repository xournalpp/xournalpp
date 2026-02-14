#include "Mask.h"

#include <iostream>

#include <cairo.h>

#include "util/Assert.h"
#include "util/Point.h"
#include "util/Range.h"
#include "util/safe_casts.h"  // for ceil_cast, floor_cast

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

/// Create a cairo context which takes ownership of surf
static auto createOwningContext(cairo_surface_t* surf, double offsetX, double offsetY, double zoom) {
    cairo_t* cr = cairo_create(surf);
    cairo_surface_destroy(surf);  // now owned by cr
    cairo_translate(cr, -offsetX, -offsetY);
    cairo_scale(cr, zoom, zoom);
    return xoj::util::CairoSPtr(cr, xoj::util::adopt);
    ;
}

template <typename DPIInfoType>
static xoj::util::CairoSPtr makeContext(DPIInfoType dpiInfo, const xoj::util::Rectangle<int>& extent, double zoom,
                                        cairo_content_t content) {
    /*
     * Create the most suitable kind of surface.
     *
     * Note that width and height are in device space coordinates (i.e. before DPI scaling).
     * If `dpiInfo` asks for a device scaling, then the number of pixels in the resulting surface will be multiplied
     * accordingly, and the scaling is applied to the new surface.
     */
    cairo_surface_t* surf = [&]() {
        if constexpr (std::is_same<DPIInfoType, int>()) {
            auto* surf =
                    cairo_image_surface_create(content == CAIRO_CONTENT_ALPHA ? CAIRO_FORMAT_A8 : CAIRO_FORMAT_ARGB32,
                                               extent.width * dpiInfo, extent.height * dpiInfo);
            cairo_surface_set_device_scale(surf, dpiInfo, dpiInfo);
            return surf;
        }
        if constexpr (std::is_same<DPIInfoType, cairo_surface_t*>()) {
            return cairo_surface_create_similar(dpiInfo, content, extent.width, extent.height);
        }
    }();

    IF_DBG_MASKS({
        std::cout << "Creating surface of type: " << getSurfaceTypeName(surf) << std::endl;
        std::cout << "  Its size: " << width << " x " << height << " (in device space)" << std::endl;
        double x;
        double y;
        cairo_surface_get_device_scale(surf, &x, &y);
        std::cout << "  Its DPI scaling: " << x << " x " << y << std::endl;
    });

    auto cr = createOwningContext(surf, extent.x, extent.y, zoom);  // surf is now owned by cr

    IF_DBG_MASKS({
        xoj::util::CairoSaveGuard saveGuard(cr.get());
        cairo_set_operator(cr.get(), CAIRO_OPERATOR_OVER);
        cairo_set_source_rgba(cr.get(), 1.0, 0.0, 0.0, 0.3);
        cairo_paint(cr.get());
    });

    if (cairo_status_t status = cairo_status(cr.get()); status != CAIRO_STATUS_SUCCESS) {
        g_warning("Unable to allocate mask: %s", cairo_status_to_string(status));
    }
    return cr;
}


Tile::Tile(int DPIScaling, const xoj::util::Rectangle<int>& extent, double zoom, cairo_content_t contentType):
        cr(makeContext(DPIScaling, extent, zoom, contentType)), extent(extent) {}
Tile::Tile(cairo_surface_t* target, const xoj::util::Rectangle<int>& extent, double zoom, cairo_content_t contentType):
        cr(makeContext(target, extent, zoom, contentType)), extent(extent) {}

void Tile::blitTo(cairo_t* targetCr) const {
    xoj_assert(cr);
    cairo_mask_surface(targetCr, cairo_get_target(cr.get()), extent.x, extent.y);
}

void Tile::paintTo(cairo_t* targetCr) const {
    xoj_assert(cr);
    cairo_set_source_surface(targetCr, cairo_get_target(cr.get()), extent.x, extent.y);
    cairo_paint(targetCr);
}

void Tile::paintToWithAlpha(cairo_t* targetCr, uint8_t alpha) const {
    xoj_assert(cr);
    cairo_set_source_surface(targetCr, cairo_get_target(cr.get()), extent.x, extent.y);
    cairo_paint_with_alpha(targetCr, alpha / 255.0);
}

void Tile::repurpose(const xoj::util::Rectangle<int>& extent, double zoom) {
    xoj_assert(extent.width == this->extent.width && extent.height == this->extent.height);
    this->cr = createOwningContext(cairo_surface_reference(cairo_get_target(this->cr.get())), extent.x, extent.y, zoom);
    this->extent = extent;
}

static xoj::util::Rectangle<int> computePixelExtent(const Range& extent, double zoom) {
    xoj_assert_message(extent.isValid(), std::string("Invalid range in Tile(): X  ") + std::to_string(extent.minX) +
                                                 " -- " + std::to_string(extent.maxX) +
                                                 "\n                         Y  " + std::to_string(extent.minY) +
                                                 " -- " + std::to_string(extent.maxY));
    xoj_assert(zoom > 0.0);
    xoj::util::Rectangle<int> res;
    res.x = floor_cast<int>(extent.minX * zoom);
    res.y = floor_cast<int>(extent.minY * zoom);
    res.width = ceil_cast<int>(extent.maxX * zoom) - res.x;
    res.height = ceil_cast<int>(extent.maxY * zoom) - res.y;
    return res;
}

Mask::Mask(cairo_surface_t* target, const Range& extent, double zoom, cairo_content_t contentType): zoom(zoom) {
    xoj_assert(target);
    this->tile = Tile(target, computePixelExtent(extent, zoom), zoom, contentType);
}
Mask::Mask(int DPIScaling, const Range& extent, double zoom, cairo_content_t contentType): zoom(zoom) {
    xoj_assert(DPIScaling > 0);
    this->tile = Tile(DPIScaling, computePixelExtent(extent, zoom), zoom, contentType);
}

void Mask::blitTo(cairo_t* targetCr) const {
    xoj_assert(isInitialized());
    xoj::util::CairoSaveGuard guard(targetCr);
    cairo_scale(targetCr, 1. / zoom, 1. / zoom);
    tile.blitTo(targetCr);
}

void Mask::paintTo(cairo_t* targetCr) const {
    xoj_assert(isInitialized());
    xoj::util::CairoSaveGuard guard(targetCr);
    cairo_scale(targetCr, 1. / zoom, 1. / zoom);
    tile.paintTo(targetCr);
}

void Mask::paintToWithAlpha(cairo_t* targetCr, uint8_t alpha) const {
    xoj_assert(isInitialized());
    xoj::util::CairoSaveGuard guard(targetCr);
    cairo_scale(targetCr, 1. / zoom, 1. / zoom);
    tile.paintToWithAlpha(targetCr, alpha);
}

void Mask::wipe() {
    xoj_assert(isInitialized());
    xoj::util::CairoSaveGuard saveGuard(tile.get());
    cairo_set_operator(tile.get(), CAIRO_OPERATOR_CLEAR);
    cairo_paint(tile.get());
    IF_DBG_MASKS({
        cairo_set_operator(tile.get(), CAIRO_OPERATOR_OVER);
        cairo_set_source_rgba(tile.get(), 1.0, 0.0, 0.0, 0.3);
        cairo_paint(tile.get());
    });
}

void Mask::wipeRange(const Range& rg) {
    xoj_assert(isInitialized());
    xoj::util::CairoSaveGuard saveGuard(tile.get());
    cairo_rectangle(tile.get(), rg.minX, rg.minY, rg.getWidth(), rg.getHeight());
    cairo_clip(tile.get());
    wipe();
}

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
