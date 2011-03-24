#include "ObjectStream.h"
#include "Serializeable.h"
#include <string.h>

const char * XML_VERSION_STR = "XojStrm1:";

ObjectOutputStream::ObjectOutputStream() {
	XOJ_INIT_TYPE(ObjectOutputStream);

	this->data = g_string_new("");
	writeString(XML_VERSION_STR);
}

ObjectOutputStream::~ObjectOutputStream() {
	XOJ_RELEASE_TYPE(ObjectOutputStream);
}

ObjectOutputStream & ObjectOutputStream::operator <<(Serializeable * s) {
	XOJ_CHECK_TYPE(ObjectOutputStream);

	s->serialize(*this);
	return *this;
}

void ObjectOutputStream::writeObject(const char * name) {
	XOJ_CHECK_TYPE(ObjectOutputStream);

	g_string_append(this->data, "_{");
	writeString(name);
}

void ObjectOutputStream::endObject() {
	XOJ_CHECK_TYPE(ObjectOutputStream);

	g_string_append(this->data, "_}");
}

void ObjectOutputStream::writeInt(int i) {
	XOJ_CHECK_TYPE(ObjectOutputStream);

	g_string_append(this->data, "_i");

	char * c = (char *) &i;
	g_string_append_len(this->data, c, sizeof(int));
}

void ObjectOutputStream::writeDouble(double d) {
	XOJ_CHECK_TYPE(ObjectOutputStream);

	g_string_append(this->data, "_d");

	char * c = (char *) &d;
	g_string_append_len(this->data, c, sizeof(double));
}

void ObjectOutputStream::writeString(const char * str) {
	XOJ_CHECK_TYPE(ObjectOutputStream);

	String s = str;
	writeString(s);
}

void ObjectOutputStream::writeString(const String & s) {
	XOJ_CHECK_TYPE(ObjectOutputStream);

	g_string_append(this->data, "_s");
	int len = s.size();
	char * c = (char *) &len;
	g_string_append_len(this->data, c, sizeof(int));
	g_string_append_len(this->data, s.c_str(), len);
}

void ObjectOutputStream::writeData(const void * data, int len, int width) {
	XOJ_CHECK_TYPE(ObjectOutputStream);

	g_string_append(this->data, "_b");
	char * c = (char *) &len;
	g_string_append_len(this->data, c, sizeof(int));

	// size of one element
	c = (char *) &width;
	g_string_append_len(this->data, c, sizeof(int));
	if (data != NULL) {
		g_string_append_len(this->data, (const char *) data, len * width);
	}
}

static cairo_status_t cairoWriteFunction(GString * string, const unsigned char * data, unsigned int length) {
	g_string_append_len(string, (const gchar *) data, length);
	return CAIRO_STATUS_SUCCESS;
}

void ObjectOutputStream::writeImage(cairo_surface_t * img) {
	XOJ_CHECK_TYPE(ObjectOutputStream);

	GString * imgStr = g_string_sized_new(102400);

	cairo_surface_write_to_png_stream(img, (cairo_write_func_t) &cairoWriteFunction, imgStr);

	g_string_append(this->data, "_m");
	char * c = (char *) &imgStr->len;
	g_string_append_len(this->data, c, sizeof(gsize));
	g_string_append_len(this->data, (const char *) imgStr->str, imgStr->len);

	g_string_free(imgStr, true);
}

GString * ObjectOutputStream::getStr() {
	XOJ_CHECK_TYPE(ObjectOutputStream);

	return this->data;
}

/////////////////////////////////////////////////////////

InputStreamException::InputStreamException(String message, const char * filename, int line) {
	this->message = message;
	this->message += ", ";
	this->message += filename;
	this->message += ": ";
	this->message += line;
}

InputStreamException::~InputStreamException() throw () {
}

const char* InputStreamException::what() const throw () {
	return this->message.c_str();
}

////////////////////////////////////////////////////

ObjectInputStream::ObjectInputStream() {
	XOJ_INIT_TYPE(ObjectInputStream);

	this->str = NULL;
	this->pos = 0;
}

ObjectInputStream::~ObjectInputStream() {
	XOJ_CHECK_TYPE(ObjectInputStream);

	if (this->str) {
		g_string_free(this->str, true);
	}

	XOJ_RELEASE_TYPE(ObjectInputStream);
}

bool ObjectInputStream::read(const char * data, int len) throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	if (this->str) {
		g_string_free(this->str, true);
	}
	this->str = g_string_new_len(data, len);
	this->pos = 0;


//	//clipboad debug
//	FILE * fp = fopen("/home/andreas/tmp/xoj/clipboard.bin", "w");
//	fwrite(str->str, len, 1, fp);
//	fclose(fp);

	try {
		String version = readString();
		if (version != XML_VERSION_STR) {
			g_warning("ObjectInputStream version mismatch... two different Xournal versions running? (%s / %s)", version.c_str(), XML_VERSION_STR);
			return false;
		}
	} catch (InputStreamException & e) {
		g_warning("InputStreamException: %s", e.what());
		return false;
	}
	return true;
}

void ObjectInputStream::readObject(const char * name) throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	String type = readObject();
	if (type != name) {
		throw INPUT_STREAM_EXCEPTION("Try to read object type %s but read object type %s", name, type.c_str());
	}
}

String ObjectInputStream::readObject() throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	checkType('{');
	return readString();
}

String ObjectInputStream::getNextObjectName() throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	int pos = this->pos;
	checkType('{');
	String name = readString();

	this->pos = pos;

	return name;
}

void ObjectInputStream::endObject() throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	checkType('}');
}

int ObjectInputStream::readInt() throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	checkType('i');

	if (this->pos + sizeof(int) >= this->str->len) {
		throw InputStreamException("End reached, but try to read an integer", __FILE__, __LINE__);
	}

	int i = *((int *) (this->str->str + this->pos));
	this->pos += sizeof(int);
	return i;
}

double ObjectInputStream::readDouble() throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	checkType('d');

	if (this->pos + sizeof(double) >= this->str->len) {
		throw InputStreamException("End reached, but try to read an double", __FILE__, __LINE__);
	}

	double d = *((double *) (this->str->str + this->pos));
	this->pos += sizeof(double);
	return d;
}

String ObjectInputStream::readString() throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	checkType('s');

	if (this->pos + sizeof(int) >= this->str->len) {
		throw InputStreamException("End reached, but try to read an string", __FILE__, __LINE__);
	}

	int len = *((int *) (this->str->str + this->pos));
	this->pos += sizeof(int);

	if (this->pos + len >= this->str->len) {
		throw InputStreamException("End reached, but try to read an string", __FILE__, __LINE__);
	}

	String s((const char *)(this->str->str + this->pos), len);
	this->pos += len;
	return s;
}

void ObjectInputStream::readData(void ** data, int * length) throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	checkType('b');

	if (this->pos + 2 * sizeof(int) >= this->str->len) {
		throw InputStreamException("End reached, but try to read data", __FILE__, __LINE__);
	}

	int len = *((int *) (this->str->str + this->pos));
	this->pos += sizeof(int);

	int width = *((int *) (this->str->str + this->pos));
	this->pos += sizeof(int);

	if (this->pos + (len * width) >= this->str->len) {
		throw InputStreamException("End reached, but try to read data", __FILE__, __LINE__);
	}

	if (len == 0) {
		*length = 0;
		*data = NULL;
	} else {
		*data = g_malloc(len * width);
		*length = len;

		memcpy(*data, this->str->str + this->pos, len * width);

		this->pos += len * width;
	}

}

class PngDatasource {
public:
	PngDatasource(char * start, int len) {
		this->data = start;
		this->len = len;
		this->pos = 0;
	}

	char * data;
	int len;
	int pos;
};

cairo_status_t cairoReadFunction(PngDatasource * obj, unsigned char * data, unsigned int length) {
	for (int i = 0; i < length; i++, obj->pos++) {
		if (obj->pos >= obj->len) {
			return CAIRO_STATUS_READ_ERROR;
		}
		data[i] = obj->data[obj->pos];
	}

	return CAIRO_STATUS_SUCCESS;
}

cairo_surface_t * ObjectInputStream::readImage() throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	checkType('m');

	if (this->pos + sizeof(int) >= this->str->len) {
		throw InputStreamException("End reached, but try to read an image", __FILE__, __LINE__);
	}

	int len = *((int *) (this->str->str + this->pos));
	this->pos += sizeof(int);

	if (this->pos + len >= this->str->len) {
		throw InputStreamException("End reached, but try to read an image", __FILE__, __LINE__);
	}

	PngDatasource source(this->str->str + this->pos, len);
	cairo_surface_t * img = cairo_image_surface_create_from_png_stream((cairo_read_func_t) cairoReadFunction, &source);

	this->pos += len;

	return img;
}

ObjectInputStream & ObjectInputStream::operator >>(Serializeable * s) throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	s->readSerialized(*this);
	return *this;
}

void ObjectInputStream::checkType(char type) throw (InputStreamException) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	if (this->pos + 2 > this->str->len) {
		throw INPUT_STREAM_EXCEPTION("End reached, but try to read %s, index %i of %ld", getType(type).c_str(), this->pos, this->str->len);
	}
	if (this->str->str[this->pos] != '_') {
		throw INPUT_STREAM_EXCEPTION("Expected type signature of %s, index %i of %ld, but read '%c'", getType(type).c_str(), this->pos, this->str->len,this->str->str[this->pos]);
	}
	this->pos++;

	if (this->str->str[this->pos] != type) {
		throw INPUT_STREAM_EXCEPTION("Expected %s but read %s", getType(type).c_str(), getType(
						this->str->str[this->pos]).c_str());
	}

	this->pos++;
}

String ObjectInputStream::getType(char type) {
	XOJ_CHECK_TYPE(ObjectInputStream);

	String ret;
	if (type == '{') {
		ret = "Object begin";
	} else if (type == '}') {
		ret = "Object end";
	} else if (type == 'i') {
		ret = "Number";
	} else if (type == 'd') {
		ret = "Floating point";
	} else if (type == 's') {
		ret = "String";
	} else if (type == 'b') {
		ret = "Binary";
	} else if (type == 'm') {
		ret = "Image";
	} else {
		char * str = g_strdup_printf("Unknown type: %02x (%c)", type, type);
		ret = str;
		g_free(str);
	}

	return ret;
}

