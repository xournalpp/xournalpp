/*
 * message.cpp
 *
 *  Created on: Apr 23, 2011
 *      Author: matthew
 *
*   To make future protocol changes:
 *      1. adjust MESSAGE_SIZE macro
 *      2. update constructor definitions (the one marked decoding)
 *      3. update the variable assignments in both constructors
 *      4. update encoding function ostream
 *
 */

#include "Message.h"
#include <sstream>
#include <cstdlib>

#include <iostream>

using namespace std;

//remember to update when changing protocol
#define MESSAGE_SIZE 5 //number of fragments in message

Message::Message(int clientId, int id, int cmd, int page,
                 GString msg)   //Received decoded msg
{
	this->clientId = clientId;
	this->id = id;
	this->cmd = cmd;
	this->page = page;
	this->msg = g_strdup(msg.str);

}

Message::Message(const GString ecdMsg)   //Received encoded msg
{
	char** fragments = g_strsplit(ecdMsg.str, ":",
	                              MESSAGE_SIZE);  //allocates memory!

	clientId = atoi(fragments[0]);
	id = atoi(fragments[1]);
	cmd = atoi(fragments[2]);
	page = atoi(fragments[3]);
	msg = g_strdup(fragments[4]);

	g_strfreev(fragments);
}

Message::~Message()
{
	g_free(msg);
}

gchar* Message::getEncoded()
{
	ostringstream s;

	s << clientId << ":" << id << ":" << cmd << ":" << page << ":" << msg;

	return (gchar*)s.str().c_str();
}

int Message::getClientId()
{
	return clientId;
}

int Message::getId()
{
	return id;
}

int Message::getCmd()
{
	return cmd;
}

int Message::getPage()
{
	return page;
}

gchar* Message::getMsg()
{
	return msg;
}
