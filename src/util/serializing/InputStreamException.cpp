#include "InputStreamException.h"

const char* XML_VERSION_STR = "XojStrm1:";

InputStreamException::InputStreamException(string message, string filename, int line)
{
	this->message = CONCAT(message, ", ", filename, ": ", line);
}

InputStreamException::~InputStreamException() throw () { }

const char* InputStreamException::what() const throw ()
{
	return this->message.c_str();
}

