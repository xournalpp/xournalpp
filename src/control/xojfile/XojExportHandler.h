/*
 * Xournal++
 *
 * Export a document for as .xoj compatible for Xournal,
 * remove all additional features which break the compatibility
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "SaveHandler.h"

#include <XournalType.h>

class XojExportHandler : public SaveHandler
{
public:
	XojExportHandler();
	virtual ~XojExportHandler();

protected:
	/**
	 * Export the fill attributes
	 */
	virtual void visitStrokeExtended(XmlPointNode* stroke, Stroke* s);

	virtual void writeHeader();
	virtual void writeSolidBackground(XmlNode* background, PageRef p);
	virtual void writeTimestamp(AudioElement* audioElement, XmlAudioNode* xmlAudioNode);

private:
	};
