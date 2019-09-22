#include "XojExportHandler.h"

#include "control/jobs/ProgressListener.h"
#include "control/pagetype/PageTypeHandler.h"
#include "control/xml/XmlNode.h"
#include "control/xml/XmlTextNode.h"
#include "control/xml/XmlImageNode.h"
#include "control/xml/XmlTexNode.h"
#include "control/xml/XmlPointNode.h"
#include "model/Stroke.h"
#include "model/Text.h"
#include "model/Image.h"
#include "model/TexImage.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/BackgroundImage.h"

#include <config.h>
#include <i18n.h>

XojExportHandler::XojExportHandler()
{
}

XojExportHandler::~XojExportHandler()
{
}

/**
 * Export the fill attributes
 */
void XojExportHandler::visitStrokeExtended(XmlPointNode* stroke, Stroke* s)
{
	// Fill is not exported in .xoj
	// Line style is also not supported
}

void XojExportHandler::writeHeader()
{
	this->root->setAttrib("creator", PROJECT_STRING);
	// Keep this version on 2, as this is anyway not read by Xournal
	this->root->setAttrib("fileversion", "2");
	this->root->addChild(new XmlTextNode("title", "Xournal document (Compatibility) - see " PROJECT_URL));
}

void XojExportHandler::writeSolidBackground(XmlNode* background, PageRef p)
{
	background->setAttrib("type", "solid");
	background->setAttrib("color", getColorStr(p->getBackgroundColor()));

	PageTypeFormat bgFormat = p->getBackgroundType().format;
	string format;

	format = PageTypeHandler::getStringForPageTypeFormat(bgFormat);
	if (bgFormat != PageTypeFormat::Plain && bgFormat != PageTypeFormat::Ruled && bgFormat != PageTypeFormat::Lined &&
	    bgFormat != PageTypeFormat::Graph)
	{
		format = "plain";
	}

	background->setAttrib("style", format);
}

void XojExportHandler::writeTimestamp(AudioElement* audioElement, XmlAudioNode* xmlAudioNode)
{
	// Do nothing since timestamp are not supported by Xournal
}
