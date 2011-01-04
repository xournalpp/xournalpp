#include "StringBuffer.h"
#include <string.h>

StringBuffer::StringBuffer(int initialLenght) {
	this->pos = 0;
	this->lenght = 0;
	this->data = NULL;

	reserveBuffer(initialLenght);
}

StringBuffer::~StringBuffer() {
	g_free(this->data);
}

void StringBuffer::append(String s) {
	append(s.c_str(), s.length());
}

void StringBuffer::append(const char * str) {
	append(str, strlen(str));
}

void StringBuffer::append(const char * str, int len) {
	if (this->pos + len >= this->lenght) {
		this->reserveBuffer(len * 2 + 100);
	}

	for (int i = 0; i < len; i++) {
		this->data[i + this->pos] = str[i];
	}
	this->pos += lenght;
	this->data[this->pos] = 0;
}

void StringBuffer::appendFormat(const char * format, ...) {
	va_list args;
	g_return_if_fail (format != NULL);

	printf("appendFormat %s\n", format);

	va_start (args, format);
	char * str = g_strdup_vprintf(format, args);
	printf("appendFormat2 %s\n", str);
	va_end (args);

	this->append(str);
	g_free(str);
}

String StringBuffer::toString() {
	return String(this->data);
}

void StringBuffer::reserveBuffer(int len) {
	if (len < 1) {
		return;
	}

	printf("reserver memory = %i\n", len + this->lenght);

	this->data = (char *)g_realloc(this->data, len + this->lenght);
	this->lenght += len;
}
