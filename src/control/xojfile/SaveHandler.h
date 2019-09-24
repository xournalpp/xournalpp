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
#include "model/Stroke.h"

#include <OutputStream.h>
#include <XournalType.h>
#include <control/xml/XmlAudioNode.h>

class XmlNode;
class XmlPointNode;
class ProgressListener;

class SaveHandler
{
public:
	SaveHandler();
	virtual ~SaveHandler();

public:
	void prepareSave(Document* doc);
	void saveTo(Path filename, ProgressListener* listener = nullptr);
	void saveTo(OutputStream* out, Path filename, ProgressListener* listener = nullptr);
	string getErrorMessage();

protected:
	static string getColorStr(int c, unsigned char alpha = 0xff);

	virtual void visitPage(XmlNode* root, PageRef p, Document* doc, int id);
	virtual void visitLayer(XmlNode* page, Layer* l);
	virtual void visitStroke(XmlPointNode* stroke, Stroke* s);

	/**
	 * Export the fill attributes
	 */
	virtual void visitStrokeExtended(XmlPointNode* stroke, Stroke* s);

	virtual void writeHeader();
	virtual void writeSolidBackground(XmlNode* background, PageRef p);
	virtual void writeTimestamp(AudioElement* audioElement, XmlAudioNode* xmlAudioNode);

protected:
	XmlNode* root;
	bool firstPdfPageVisited;
	int attachBgId;

	string errorMessage;

	GList* backgroundImages;
};
