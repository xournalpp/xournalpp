/*
 * Xournal++
 *
 * Save help classes
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#ifndef __SAVEHANDLERHELPER_H__
#define __SAVEHANDLERHELPER_H__

#include "../model/String.h"
#include "../util/OutputStream.h"
#include <vector>

class Attribute {
public:
	Attribute(String name) {
		this->name = name;
	}

	virtual void writeOut(OutputStream * out) = 0;

	String getName() {
		return name;
	}

private:
	String name;
};

class TextAttribute: public Attribute {
public:
	TextAttribute(String name, String value) :
		Attribute(name) {
		this->value = value;
	}

	virtual void writeOut(OutputStream * out) {
		out->write(value.replace("\"", "&quot;"));
	}

private:
	String value;
};

class DoubleAttribute: public Attribute {
public:
	DoubleAttribute(String name, double value) :
		Attribute(name) {
		this->value = value;
	}

	virtual void writeOut(OutputStream * out) {
		char * str = g_strdup_printf("%0.2lf", value);
		out->write(str);
		g_free(str);
	}

private:
	double value;
};

class DoubleArrayAttribute: public Attribute {
public:
	DoubleArrayAttribute(String name, double * values, int count) :
		Attribute(name) {
		this->values = values;
		this->count = count;
	}

	~DoubleArrayAttribute() {
		delete values;
	}

	virtual void writeOut(OutputStream * out) {
		if (this->count > 0) {
			char * str = g_strdup_printf("%0.2lf", this->values[0]);
			out->write(str);
			g_free(str);
		}

		for (int i = 1; i < this->count; i++) {
			char * str = g_strdup_printf(" %0.2lf", this->values[i]);
			out->write(str);
			g_free(str);
		}
	}

private:
	double * values;
	int count;
};

class IntAttribute: public Attribute {
public:
	IntAttribute(String name, int value) :
		Attribute(name) {
		this->value = value;
	}

	virtual void writeOut(OutputStream * out) {
		char * str = g_strdup_printf("%i", value);
		out->write(str);
		g_free(str);
	}

private:
	int value;
};

class XmlNode {
public:
	XmlNode(String tag) {
		this->tag = tag;
	}

	~XmlNode() {
		std::vector<XmlNode *>::iterator it = children.begin();
		for (; it != children.end(); it++) {
			delete (*it);
		}
		children.clear();

		std::vector<Attribute *>::iterator it2;
		for (it2 = attributes.begin(); it2 != attributes.end(); it2++) {
			delete *it2;
		}
		attributes.clear();
	}

	void setAttrib(String attrib, String value) {
		if (value.c_str() == NULL) {
			value = "";
		}

		putAttrib(new TextAttribute(attrib, value));
	}

	void setAttrib(String attrib, double value) {
		putAttrib(new DoubleAttribute(attrib, value));
	}

	void setAttrib(String attrib, int value) {
		putAttrib(new IntAttribute(attrib, value));
	}

	/**
	 * The double array is now owned by XmlNode and automatically deleted!
	 */
	void setAttrib(String attrib, double * value, int count) {
		putAttrib(new DoubleArrayAttribute(attrib, value, count));
	}

	virtual void writeOut(OutputStream * out) {
		out->write("<");
		out->write(tag);
		writeAttributes(out);

		if (children.size() == 0) {
			out->write("/>\n");
		} else {
			out->write(">\n");

			std::vector<XmlNode *>::iterator it = children.begin();
			for (; it != children.end(); it++) {
				(*it)->writeOut(out);
			}

			out->write("</");
			out->write(tag);
			out->write(">\n");
		}
	}

	void addChild(XmlNode * node) {
		children.push_back(node);
	}

protected:
	void putAttrib(Attribute * a) {
		std::vector<Attribute *>::iterator it;
		for (it = attributes.begin(); it != attributes.end(); it++) {
			if ((*it)->getName() == a->getName()) {
				attributes.erase(it);
				delete (*it);
				break;
			}
		}
		attributes.push_back(a);
	}

	void writeAttributes(OutputStream * out) {
		std::vector<Attribute *>::iterator it;
		for (it = attributes.begin(); it != attributes.end(); it++) {
			out->write(" ");
			out->write((*it)->getName());
			out->write("=\"");
			(*it)->writeOut(out);
			out->write("\"");
		}
	}

protected:
	std::vector<XmlNode *> children;
	std::vector<Attribute *> attributes;

	String tag;
};

class XmlTextNode: public XmlNode {
public:
	XmlTextNode(String tag, String text) :
		XmlNode(tag) {
		this->text = text;
	}

	XmlTextNode(String tag) :
		XmlNode(tag) {
	}

	void setText(String text) {
		this->text = text;
	}

	virtual void writeOut(OutputStream * out) {
		out->write("<");
		out->write(tag);
		writeAttributes(out);

		out->write(">");

		String tmp = text.replace("&", "&amp;");
		tmp = tmp.replace("<", "&lt;");
		tmp = tmp.replace(">", "&gt;");
		out->write(tmp);

		out->write("</");
		out->write(tag);
		out->write(">\n");
	}
private:
	String text;
};

class XmlImageNode: public XmlNode {
public:
	XmlImageNode(String tag) :
		XmlNode(tag) {
		this->img = NULL;
		this->out = NULL;
		this->pos = 0;
	}

	~XmlImageNode() {
		if (this->img) {
			cairo_surface_destroy(this->img);
		}
	}

	void setImage(cairo_surface_t * img) {
		if (this->img) {
			cairo_surface_destroy(this->img);
		}
		this->img = cairo_surface_reference(img);
	}

	static cairo_status_t pngWriteFunction(XmlImageNode * image, unsigned char *data, unsigned int length) {
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

	virtual void writeOut(OutputStream * out) {
		out->write("<");
		out->write(tag);
		writeAttributes(out);

		out->write(">");

		if (this->img == NULL) {
			g_error("XmlImageNode::writeOut(); this->img == NULL");
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
private:
	cairo_surface_t * img;

	OutputStream * out;
	int pos;
	unsigned char buffer[30];
};

class XmlPointNode: public XmlNode {
public:
	XmlPointNode(String tag) :
		XmlNode(tag) {
		this->points = NULL;
	}

	~XmlPointNode() {
		delete points;
	}

	/**
	 * The point array is owned by the XML Node and automatically deleted
	 */
	void setPoints(Point * points, int count) {
		// Delete may old data
		delete this->points;

		this->points = points;
		this->count = count;
	}

	virtual void writeOut(OutputStream * out) {
		out->write("<");
		out->write(tag);
		writeAttributes(out);

		out->write(">");

		for (int i = 0; i < this->count; i++) {
			if (i != 0) {
				out->write(" ");
			}
			Point p = points[i];
			char * tmp = g_strdup_printf("%0.2lf %0.2lf", p.x, p.y);
			out->write(tmp);
			g_free(tmp);
		}

		out->write("</");
		out->write(tag);
		out->write(">\n");
	}

private:
	Point * points;
	int count;
};

class XmlStrokeNode: public XmlNode {
public:
	XmlStrokeNode(String tag) :
		XmlNode(tag) {
		this->points = NULL;
		this->width = 0;
		this->widths = NULL;
		this->widthsLength = 0;
	}
	~XmlStrokeNode() {
		delete[] this->points;
		delete[] this->widths;
	}

	void setPoints(Point * points, int pointLength) {
		if (this->points) {
			delete[] this->points;
		}
		this->points = new Point[pointLength];
		for (int i = 0; i < pointLength; i++) {
			this->points[i] = points[i];
		}
		this->pointLength = pointLength;
	}

	void setWidth(double width, double * widths, int widthsLength) {
		this->width = width;

		if (this->widths) {
			delete[] this->widths;
		}
		this->widths = new double[widthsLength];
		for (int i = 0; i < widthsLength; i++) {
			this->widths[i] = widths[i];
		}
		this->widthsLength = widthsLength;

	}

	virtual void writeOut(OutputStream * out) {
		out->write("<");
		out->write(tag);
		writeAttributes(out);

		out->write(" width=\"");
		char * tmp = g_strdup_printf("%1.2lf", width);
		out->write(tmp);
		g_free(tmp);

		for (int i = 0; i < widthsLength; i++) {
			char * tmp = g_strdup_printf(" %1.2lf", widths[i]);
			out->write(tmp);
			g_free(tmp);
		}

		out->write("\"");

		if (children.size() == 0) {
			out->write("/>");
		} else {
			out->write(">");

			if (pointLength > 0) {
				char * tmp = g_strdup_printf("%1.2lf %1.2lf", points[0].x, points[0].y);
				out->write(tmp);
				g_free(tmp);

				for (int i = 1; i < pointLength; i++) {
					char * tmp = g_strdup_printf(" %1.2lf %1.2lf", points[i].x, points[i].y);
					out->write(tmp);
					g_free(tmp);
				}
			}

			out->write("</");
			out->write(tag);
			out->write(">\n");
		}
	}
private:
	Point * points;
	int pointLength;

	double width;

	double * widths;
	int widthsLength;
};

#endif /* __SAVEHANDLERHELPER_H__ */
