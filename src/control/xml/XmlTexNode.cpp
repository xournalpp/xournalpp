#include "XmlTexNode.h"

XmlTexNode::XmlTexNode(const char * tag) :
	XmlNode(tag) {
	XOJ_INIT_TYPE(XmlTexNode);

	this->img = NULL;
	this->out = NULL;
	this->pos = 0;
}

XmlTexNode::~XmlTexNode() {
	XOJ_CHECK_TYPE(XmlTexNode);

	if (this->img) {
		cairo_surface_destroy(this->img);
	}

	XOJ_RELEASE_TYPE(XmlTexNode);
}

void XmlTexNode::setImage(cairo_surface_t * img) {
	XOJ_CHECK_TYPE(XmlTexNode);

	if (this->img) {
		cairo_surface_destroy(this->img);
	}
	this->img = cairo_surface_reference(img);
}

cairo_status_t XmlTexNode::pngWriteFunction(XmlTexNode * image, unsigned char *data, unsigned int length) {
	for (unsigned int i = 0; i < length; i++, image->pos++) {
		if (image->pos == 30) {
			gchar * base64_str = g_base64_encode(image->buffer, image->pos);
			image->out->write(base64_str);
			g_free(base64_str);
			image->pos = 0;
		}
		image->buffer[image->pos] = data[i];
	}

	return CAIRO_STATUS_SUCCESS;
}

void XmlTexNode::writeOut(OutputStream * out) {
	XOJ_CHECK_TYPE(XmlTexNode);

	out->write("<");
	out->write(tag);
	writeAttributes(out);

	out->write(">");

	if (this->img == NULL) {
		g_error("XmlTexNode::writeOut(); this->img == NULL");
	} else {
		this->out = out;
		this->pos = 0;
		cairo_surface_write_to_png_stream(this->img, (cairo_write_func_t) &pngWriteFunction, this);
		gchar * base64_str = g_base64_encode(this->buffer, this->pos);
		out->write(base64_str);
		g_free(base64_str);

		this->out = NULL;
	}

	out->write("</");
	out->write(tag);
	out->write(">\n");
}
