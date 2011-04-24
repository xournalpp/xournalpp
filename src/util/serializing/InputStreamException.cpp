#include "InputStreamException.h"

const char * XML_VERSION_STR = "XojStrm1:";

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

