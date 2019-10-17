#include "InputStreamException.h"

const char* XML_VERSION_STR = "XojStrm1:";

InputStreamException::InputStreamException(string message, string filename, int line)
{
	this->message = message + ", " + filename + ": " + std::to_string(line);
}

InputStreamException::~InputStreamException()
{
}

const char* InputStreamException::what()
{
	return this->message.c_str();
}
