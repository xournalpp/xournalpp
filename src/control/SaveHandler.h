/*
 * Xournal++
 *
 * Saves a document
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SAVEHANDLER_H__
#define __SAVEHANDLER_H__

#include "../util/OutputStream.h"
#include "../util/XournalType.h"
#include "../model/Page.h"
#include "../model/Document.h"
class XmlNode;

class SaveHandler {
public:
	SaveHandler();
	virtual ~SaveHandler();

public:
	void prepareSave(Document * doc);
	void saveTo(OutputStream * out, String filename);
	String getErrorMessage();

private:
	void visitPage(XmlNode * root, XojPage * p, Document * doc, int id);
	static String getSolidBgStr(BackgroundType type);
	static String getColorStr(int c, unsigned char alpha = 0xff);
	void visitLayer(XmlNode * page, Layer * l);

private:
	XOJ_TYPE_ATTRIB;

	XmlNode * root;
	bool firstPdfPageVisited;
	int attachBgId;

	String errorMessage;

	GList * backgroundImages;
};

#endif /* __SAVEHANDLER_H__ */
