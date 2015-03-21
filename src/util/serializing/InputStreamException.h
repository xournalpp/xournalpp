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

#include <StringUtils.h>
#include <exception>

#define INPUT_STREAM_EXCEPTION(description, ...) \
	InputStreamException(StringUtils::format(description, __VA_ARGS__), __FILE__, __LINE__); \
	 
class InputStreamException: public std::exception
{
public:
	InputStreamException(string message, string filename, int line);
	virtual ~InputStreamException() throw ();

public:
	virtual const char* what() const throw ();

private:
	XOJ_TYPE_ATTRIB;

	string message;
};

#endif /* __INPUTSTREAMEXCEPTION_H__ */
