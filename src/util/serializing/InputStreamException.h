/*
 * Xournal++
 *
 * Input stream exception
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __INPUTSTREAMEXCEPTION_H__
#define __INPUTSTREAMEXCEPTION_H__

#include <String.h>
#include <exception>

#define INPUT_STREAM_EXCEPTION(description, ...) \
	InputStreamException(String::format(description, __VA_ARGS__), __FILE__, __LINE__); \

class InputStreamException: public std::exception {
public:
	InputStreamException(String message, const char * filename, int line);
	virtual ~InputStreamException() throw ();

public:
	virtual const char * what() const throw ();

private:
	XOJ_TYPE_ATTRIB;

	String message;
};

#endif /* __INPUTSTREAMEXCEPTION_H__ */
