/*
 * Xournal++
 *
 * Debugoutput
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */


#ifndef __DEBUG_H__
#define __DEBUG_H__

class Debug {
private:
	Debug();
	virtual ~Debug();
public:
	static void warning(const char * msg, ...);
};

#endif /* __DEBUG_H__ */
