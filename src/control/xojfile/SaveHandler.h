/*
 * Xournal++
 *
 * Saves a document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/Document.h"
#include "model/PageRef.h"

#include <OutputStream.h>
#include <XournalType.h>

class XmlNode;
class ProgressListener;

class SaveHandler
{
public:
	SaveHandler();
	virtual ~SaveHandler();

public:
	void prepareSave(Document* doc);
	void saveTo(OutputStream* out, path filename, ProgressListener* listener = NULL);
	string getErrorMessage();

private:
	void visitPage(XmlNode* root, PageRef p, Document* doc, int id);
	static string getSolidBgStr(BackgroundType type);
	static string getColorStr(int c, unsigned char alpha = 0xff);
	void visitLayer(XmlNode* page, Layer* l);

private:
	XOJ_TYPE_ATTRIB;

	XmlNode* root;
	bool firstPdfPageVisited;
	int attachBgId;

	string errorMessage;

	GList* backgroundImages;
};
