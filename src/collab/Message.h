/*
 * message.h
 *
 *  Created on: Apr 23, 2011
 *      Author: matthew
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <glib-2.0/glib.h>

class Message
{
public:
	Message(int clientId, int id, int cmd, int Page, GString msg);
	Message(const GString encodedMsg);
	virtual ~Message();

	//encoding functions
	gchar* getEncoded();

	//decoding functions
	gchar* getMsg();
	int getId();
	int getClientId();
	int getCmd();
	int getPage();

private:
	int clientId;
	int id;
	int cmd;
	int page;
	gchar* msg;
};

#endif /* MESSAGE_H_ */
