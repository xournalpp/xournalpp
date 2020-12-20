#include "Shadow.h"

#include <cmath>

#include "ShadowCode.c"

Shadow* Shadow::instance = new Shadow();

using u8ptr = const unsigned char*;

Shadow::Shadow() {
    const int sBrSize = shadowBottomRightSize;
    const int sTlSize = shadowTopLeftSize;

    this->edgeTopLeft = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sTlSize * 2, sTlSize * 2);
    cairo_t* cr = cairo_create(this->edgeTopLeft);
    drawShadowEdge(cr, 0, 0, sTlSize, sTlSize, reinterpret_cast<u8ptr>(shadowEdge1), 0, 0, 0);
    cairo_destroy(cr);

    this->edgeTopRight = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sBrSize * 2, sTlSize * 2);
    cr = cairo_create(this->edgeTopRight);
    drawShadowEdge(cr, 0, 0, sBrSize, sTlSize, reinterpret_cast<u8ptr>(shadowEdge2), 0, 0, 0);
    cairo_destroy(cr);

    this->edgeBottomRight = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sBrSize * 2, sBrSize * 2);
    cr = cairo_create(this->edgeBottomRight);
    drawShadowEdge(cr, 0, 0, sBrSize, sBrSize, reinterpret_cast<u8ptr>(shadowEdge3), 0, 0, 0);
    cairo_destroy(cr);

    this->edgeBottomLeft = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sTlSize * 2, sBrSize * 2);
    cr = cairo_create(this->edgeBottomLeft);
    drawShadowEdge(cr, 0, 0, sTlSize, sBrSize, reinterpret_cast<u8ptr>(shadowEdge4), 0, 0, 0);
    cairo_destroy(cr);
}

Shadow::~Shadow() {
    cairo_surface_destroy(this->edgeBottomLeft);
    cairo_surface_destroy(this->edgeBottomRight);
    cairo_surface_destroy(this->edgeTopLeft);
    cairo_surface_destroy(this->edgeTopRight);
    this->edgeBottomLeft = nullptr;
    this->edgeBottomRight = nullptr;
    this->edgeTopLeft = nullptr;
    this->edgeTopRight = nullptr;

    if (this->left) {
        cairo_surface_destroy(this->left);
    }
    if (this->top) {
        cairo_surface_destroy(this->top);
    }
    if (this->right) {
        cairo_surface_destroy(this->right);
    }
    if (this->bottom) {
        cairo_surface_destroy(this->bottom);
    }

    this->left = nullptr;
    this->top = nullptr;
    this->right = nullptr;
    this->bottom = nullptr;
}

auto Shadow::getShadowBottomRightSize() -> int { return shadowBottomRightSize; }

auto Shadow::getShadowTopLeftSize() -> int { return shadowTopLeftSize; }

void Shadow::drawShadowEdge(cairo_t* cr, int x, int y, int width, int height, const unsigned char* edge, double r,
                            double g, double b) {
    int w = width * 2;
    int h = height * 2;

    for (int u = 0; u < h; u++) {
        for (int i = 0; i < w; i++) {
            double a = edge[u * w + i] / 255.0;
            cairo_set_source_rgba(cr, r, g, b, a);

            cairo_rectangle(cr, x + i, y + u, 1, 1);
            cairo_fill(cr);
        }
    }
}

void Shadow::paintEdge(cairo_t* cr, cairo_surface_t* image, int x, int y, int width, int height) {
    cairo_set_source_surface(cr, image, x - width, y - height);
    cairo_rectangle(cr, x - width, y - height, width * 2, height * 2);
    cairo_fill(cr);
}

void Shadow::drawShadowTop(cairo_t* cr, int x, int y, int width, double r, double g, double b) {
    int w = this->top ? cairo_image_surface_get_width(this->top) : 0;

    if (w < width) {
        if (this->top) {
            cairo_surface_destroy(this->top);
        }

        this->top = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, shadowTopLeftSize);
        cairo_t* cr2 = cairo_create(this->top);

        double a = NAN;

        // draw shadow on top and left
        for (int i = 0; i < shadowTopLeftSize; i++) {
            a = shadowTopLeft[i] / 255.0;
            cairo_set_source_rgba(cr2, r, g, b, a);

            cairo_rectangle(cr2, 0, i, width, 1);
            cairo_fill(cr2);
        }

        cairo_destroy(cr2);
    }

    cairo_set_source_surface(cr, this->top, x + shadowTopLeftSize, y - shadowTopLeftSize);
    cairo_rectangle(cr, x + shadowTopLeftSize, y - shadowTopLeftSize, width - 2 * shadowTopLeftSize, shadowTopLeftSize);
    cairo_fill(cr);
}

void Shadow::drawShadowLeft(cairo_t* cr, int x, int y, int height, double r, double g, double b) {
    int h = this->left ? cairo_image_surface_get_height(this->left) : 0;

    if (h < height) {
        if (this->left) {
            cairo_surface_destroy(this->left);
        }

        this->left = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, shadowTopLeftSize, height);
        cairo_t* cr2 = cairo_create(this->left);

        double a = NAN;

        // draw shadow on top and left
        for (int i = 0; i < shadowTopLeftSize; i++) {
            a = shadowTopLeft[i] / 255.0;
            cairo_set_source_rgba(cr2, r, g, b, a);

            cairo_rectangle(cr2, i, 0, 1, height);
            cairo_fill(cr2);
        }

        cairo_destroy(cr2);
    }

    cairo_set_source_surface(cr, this->left, x - shadowTopLeftSize, y + shadowTopLeftSize);
    cairo_rectangle(cr, x - shadowTopLeftSize, y + shadowTopLeftSize, shadowTopLeftSize,
                    height - 2 * shadowTopLeftSize);
    cairo_fill(cr);
}

void Shadow::drawShadowRight(cairo_t* cr, int x, int y, int height, double r, double g, double b) {
    int h = this->right ? cairo_image_surface_get_height(this->right) : 0;

    if (h < height) {
        if (this->right) {
            cairo_surface_destroy(this->right);
        }

        this->right = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, shadowBottomRightSize, height);
        cairo_t* cr2 = cairo_create(this->right);

        double a = NAN;

        // draw shadow on top and left
        for (int i = 0; i < shadowBottomRightSize; i++) {
            a = shadowBottomRight[i] / 255.0;
            cairo_set_source_rgba(cr2, r, g, b, a);

            cairo_rectangle(cr2, i, 0, 1, height);
            cairo_fill(cr2);
        }

        cairo_destroy(cr2);
    }

    cairo_set_source_surface(cr, this->right, x, y + shadowBottomRightSize);
    cairo_rectangle(cr, x, y + shadowBottomRightSize, shadowBottomRightSize, height - 2 * shadowBottomRightSize);
    cairo_fill(cr);
}

void Shadow::drawShadowBottom(cairo_t* cr, int x, int y, int width, double r, double g, double b) {
    int w = this->bottom ? cairo_image_surface_get_width(this->bottom) : 0;

    if (w < width) {
        if (this->bottom) {
            cairo_surface_destroy(this->bottom);
        }

        this->bottom = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, shadowBottomRightSize);
        cairo_t* cr2 = cairo_create(this->bottom);


        // draw shadow on top and left
        for (int i = 0; i < shadowBottomRightSize; i++) {
            auto a = shadowBottomRight[i] / 255.0;
            cairo_set_source_rgba(cr2, r, g, b, a);

            cairo_rectangle(cr2, 0, i, width, 1);
            cairo_fill(cr2);
        }

        cairo_destroy(cr2);
    }

    cairo_set_source_surface(cr, this->bottom, x + shadowBottomRightSize, y);
    cairo_rectangle(cr, x + shadowBottomRightSize, y, width - 2 * shadowBottomRightSize, shadowBottomRightSize);
    cairo_fill(cr);
}

void Shadow::drawShadowImpl(cairo_t* cr, int x, int y, int width, int height) {
    const int sBrSize = shadowBottomRightSize;
    const int sTlSize = shadowTopLeftSize;

    drawShadowTop(cr, x, y, width - 3, 0, 0, 0);
    drawShadowLeft(cr, x, y, height - 3, 0, 0, 0);
    drawShadowRight(cr, x + width, y - 4, height + 5, 0, 0, 0);
    drawShadowBottom(cr, x - 4, y + height, width + 5, 0, 0, 0);

    paintEdge(cr, this->edgeTopLeft, x, y, sTlSize, sTlSize);
    paintEdge(cr, this->edgeTopRight, x + width + 1, y, sBrSize, sTlSize);
    paintEdge(cr, this->edgeBottomLeft, x, y + height + 1, sTlSize, sBrSize);
    paintEdge(cr, this->edgeBottomRight, x + width + 1, y + height + 1, sBrSize, sBrSize);
}

void Shadow::drawShadow(cairo_t* cr, int x, int y, int width, int height) {
    Shadow::instance->drawShadowImpl(cr, x, y, width, height);
}
