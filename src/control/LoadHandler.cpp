#include "LoadHandler.h"

#include "../model/XojPage.h"
#include "../model/BackgroundImage.h"

#include <clocale>
#include <vector>
#include <iostream>
using namespace std;

#include <boost/filesystem.hpp>
namespace bf = boost::filesystem;

#include <glibmm.h>

#include <glib.h>	//only because of GError for BackgroundImage

#include <config.h>
#include <glib/gi18n-lib.h>

typedef struct
{
	const char* name;
	const int rgb;
} PredefinedColor;

PredefinedColor PREDEFINED_COLORS[] = {
	{ "black",      0x000000 },
	{ "blue",       0x3333cc },
	{ "red",        0xff0000 },
	{ "green",      0x008000 },
	{ "gray",       0x808080 },
	{ "lightblue",  0x00c0ff },
	{ "lightgreen", 0x00ff00 },
	{ "magenta",    0xff00ff },
	{ "orange",     0xff8000 },
	{ "yellow",     0xffff00 },
	{ "white",      0xffffff }
};

LoadHandler::LoadHandler() : doc(&dHanlder)
{
	XOJ_INIT_TYPE(LoadHandler);

	initAttributes();
	this->removePdfBackgroundFlag = false;
	this->pdfReplacementAttach = false;
}

LoadHandler::~LoadHandler()
{
	XOJ_RELEASE_TYPE(LoadHandler);
}

void LoadHandler::initAttributes()
{
	XOJ_CHECK_TYPE(LoadHandler);

	if (this->file.is_open()) this->file.close();
	this->pdfFilenameParsed = false;
	this->attachedPdfMissing = false;

	this->page = NULL;
	this->layer = NULL;
}

string LoadHandler::getLastError()
{
	XOJ_CHECK_TYPE(LoadHandler);

	return this->lastError;
}

bool LoadHandler::isAttachedPdfMissing()
{
	XOJ_CHECK_TYPE(LoadHandler);

	return this->attachedPdfMissing;
}

path LoadHandler::getMissingPdfFilename()
{
	XOJ_CHECK_TYPE(LoadHandler);

	return this->pdfMissing;
}

void LoadHandler::removePdfBackground()
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->removePdfBackgroundFlag = true;
}

void LoadHandler::setPdfReplacement(path filename, bool attachToDocument)
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->pdfReplacementFilename = filename;
	this->pdfReplacementAttach = attachToDocument;
}

bool LoadHandler::openFile(path filename)
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->filename = filename;
	this->file.open(filename.string(), ios_base::in | ios_base::binary);
	if (!this->file.is_open())
	{
		this->lastError = (bl::format("Could not open file: \"{1}\"") % filename.string()).str();
		return false;
	}
	
	this->inbuf.push(bio::gzip_decompressor());
	this->inbuf.push(file);
	
	return true;
}

bool LoadHandler::closeFile()
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->file.close();
	return !this->file.is_open();
}

int LoadHandler::parseColor(const string& name)
{
	XOJ_CHECK_TYPE(LoadHandler);
	
	if (name[0] == '#')
	{
		string::size_type ptr;
		int c = stoull(name.substr(1), &ptr, 16);
		
		if (ptr != name.length() - 1)
			throw ParseException((bl::format("Unknown color value \"{1}\"") % name).str());

		return (c >> 8);
	}
	else
	{
		for (PredefinedColor c : PREDEFINED_COLORS)
		{
			if (name == c.name) return c.rgb;
		}
		throw ParseException((bl::format("Color \"{1}\" unknown (not defined in default color list)!") % name).str());
	}
}

bool LoadHandler::parseXml()
{
	XOJ_CHECK_TYPE(LoadHandler);

	char* old_locale, *saved_locale;

	old_locale = setlocale(LC_NUMERIC, NULL);
	saved_locale = g_strdup(old_locale);

	setlocale(LC_NUMERIC, "C");
	
	istream in(&inbuf);
	xml_parse_result parse_result = this->xml.load(in);
	if (parse_result.status != status_ok)
	{
		cerr << bl::format("Cannot parse xoj file \"{1}\": {2}") % filename % parse_result.description() << endl; //TODO localize every message
		return false;
	}
	
	xml_node xml_xournal = xml.child("xournal");
	if (!xml_xournal.empty()) parseXmlXournal(&xml_xournal);
	else throw ParseException("xournal", true); 
	
	setlocale(LC_NUMERIC, saved_locale);
	g_free(saved_locale);

	cout << bl::format("Parsed document of creator: {1}, version {2}")
			% this->creator % this->fileversion << endl;
	doc.setCreateBackupOnSave(this->fileversion >= 2); //CPPCHECK is it ok?

	return true;
}

void LoadHandler::parseXmlXournal(xml_node* xml_xournal)
{
	XOJ_CHECK_TYPE(LoadHandler);
	
	this->fileversion = xml_xournal->attribute("fileversion").as_int(1);
	
	xml_attribute creator = xml_xournal->attribute("creator");
	if (!creator.empty()) this->creator = creator.value();
	else
	{
		xml_attribute version = xml_xournal->attribute("version");
		if (!version.empty()) this->creator = CONCAT("Xournal ", version.value());
		else this->creator = "Unknown";
	}
	
	//pages
	for (xml_node xml_page : xml_xournal->children("page"))
	{
		if (xml_page.empty()) throw ParseException("xournal.page", true);
		parseXmlPage(&xml_page);
	}		
}

void LoadHandler::parseXmlPage(xml_node* xml_page)
{
	XOJ_CHECK_TYPE(LoadHandler);
	
	//page dimensions
	double width = xml_page->attribute("width").as_double();
	if (width == 0) throw ParseException("xournal.page(width)", true);
	double height = xml_page->attribute("height").as_double();
	if (height == 0) throw ParseException("xournal.page(height)", true);

	this->page = new XojPage(width, height);
	this->doc.addPage(this->page);

	//background
	xml_node xml_background = xml_page->child("background");
	if (xml_background.empty()) throw ParseException("xournal.page.background", true);
	
	xml_attribute bg_type = xml_background.attribute("type");
	if (bg_type.empty()) throw ParseException("xournal.page.background(type)", true);
	
	const string bg_type_v(bg_type.value());
	if		(bg_type_v == "solid")	parseXmlPageBgSolid(&xml_background);
	else if (bg_type_v == "pixmap")	parseXmlPageBgPixmap(&xml_background);
	else if (bg_type_v == "pdf")
	{
		if (this->removePdfBackgroundFlag)
		{
			this->page->setBackgroundType(BACKGROUND_TYPE_NONE);
			this->page->setBackgroundColor(0xffffff);
		}
		else parseXmlPageBgPdf(&xml_background);
	}
	else throw ParseException((bl::format("Unknown background type: \"{1}\"")
								% bg_type.value()).str());

	//layers
	for (xml_node xml_layer : xml_page->children("layer"))
	{
		if (xml_layer.empty()) throw ParseException("xournal.page.layer", true);
		parseXmlLayer(&xml_layer);
	}
}

const char* RULINGSTR_NONE = "plain";
const char* RULINGSTR_LINED = "lined";
const char* RULINGSTR_RULED = "ruled";
const char* RULINGSTR_GRAPH = "graph";

void LoadHandler::parseXmlPageBgSolid(xml_node* xml_bg)
{
	XOJ_CHECK_TYPE(LoadHandler);

	xml_attribute bg_style = xml_bg->attribute("style");
	if (bg_style.empty()) throw ParseException("xournal.page.background(style)", true);

	const string bg_style_v(bg_style.value());
	if (bg_style_v == RULINGSTR_NONE)
		this->page->setBackgroundType(BACKGROUND_TYPE_NONE);
	else if (bg_style_v == RULINGSTR_LINED)
		this->page->setBackgroundType(BACKGROUND_TYPE_LINED);
	else if (bg_style_v == RULINGSTR_RULED)
		this->page->setBackgroundType(BACKGROUND_TYPE_RULED);
	else if (bg_style_v == RULINGSTR_GRAPH)
		this->page->setBackgroundType(BACKGROUND_TYPE_GRAPH);
	else
	{
		this->page->setBackgroundType(BACKGROUND_TYPE_NONE);
		cerr << bl::format("Unknown background type: \"{1}\"")
				% bg_style.value() << endl;
	}
	
	xml_attribute bg_color = xml_bg->attribute("color");
	if (bg_color.empty()) throw ParseException("xournal.page.background(color)", true);

	const string bg_color_val = bg_color.value();
	int color;
	if		(bg_color_val == "blue")	color = 0xa0e8ff;
	else if (bg_color_val == "pink")	color = 0xffc0d4;
	else if (bg_color_val == "green")	color = 0x80FFC0;
	else if (bg_color_val == "orange")	color = 0xFFC080;
	else if (bg_color_val == "yellow")	color = 0xFFFF80;
	else if (bg_color_val == "white")	color = 0xffffff;
	else								color = parseColor(bg_color_val);

	this->page->setBackgroundColor(color);
}

void LoadHandler::parseXmlPageBgPixmap(xml_node* xml_bg)
{
	XOJ_CHECK_TYPE(LoadHandler);

	const string bg_domain = xml_bg->attribute("domain").value();
	if (bg_domain.empty()) throw ParseException("xournal.page.background(domain)", true);
	
	path fileToLoad;
	bool loadFile = false;
	
	const string bg_filename = xml_bg->attribute("filename").value();
	
	if (bg_domain == "absolute")
	{
		if (bg_filename.empty()) throw ParseException("xournal.page.background(filename)", true);
		fileToLoad = path(bg_filename);
		loadFile = true;
	}
	else if (bg_domain == "attach")
	{
		if (bg_filename.empty()) throw ParseException("xournal.page.background(filename)", true);
		fileToLoad = this->filename;
		fileToLoad /= CONCAT(".", bg_filename);
		loadFile = true;
	}

	if (loadFile)
	{
		GError* error = NULL;
		BackgroundImage img;
		img.loadFile(fileToLoad, &error);

		if (error)
		{
			cerr << bl::format("Could not read image: {1}, Error message: {2}")
								% fileToLoad % error->message << endl;
			g_error_free(error);
		}

		this->page->setBackgroundImage(img);
	}
	else if (bg_domain == "clone")
	{
		PageRef p = doc.getPage(stoi(filename.string()));//TOTEST!!!

		if (p.isValid())
			this->page->setBackgroundImage(p->getBackgroundImage());
	}
	else throw ParseException((bl::format("Unknown pixmap domain type: {1}") % bg_domain).str());

	this->page->setBackgroundType(BACKGROUND_TYPE_IMAGE);
}

void LoadHandler::parseXmlPageBgPdf(xml_node* xml_bg)
{
	XOJ_CHECK_TYPE(LoadHandler);

	xml_attribute bg_pageno = xml_bg->attribute("pageno");
	if (bg_pageno.empty()) throw ParseException("xournal.page.background(pageno)", true);
	
	int pageno = bg_pageno.as_int();
	this->page->setBackgroundPdfPageNr(pageno - 1);
	
	if (!this->pdfFilenameParsed)
	{
		bool attachToDocument = false;
		path pdfFilename;
		
		if (this->pdfReplacementFilename.empty())
		{
			xml_attribute bg_domain = xml_bg->attribute("domain");
			xml_attribute bg_filename = xml_bg->attribute("filename");

			if (bg_domain.empty())		throw ParseException("xournal.page.background(domain)", true);
			if (bg_filename.empty())	throw ParseException("xournal.page.background(filename)", true);
			
			const string bg_domain_val = bg_domain.value();
			const string bg_filename_val = bg_filename.value();
			pdfFilename = path(bg_filename_val);

			if (bg_domain_val == "absolute")
			{
				if (!bf::exists(pdfFilename)) // Absolute OR relative path
				{
					path tmpFilename = xournalFilename.parent_path();
					tmpFilename /= pdfFilename.filename();

					if (bf::exists(tmpFilename)) pdfFilename = tmpFilename;
				}
			}
			else if (bg_domain_val == "attach")
			{
				attachToDocument = true;
				path tmpFilename = CONCAT(xournalFilename.string(), '.', pdfFilename.string());

				if (bf::exists(tmpFilename)) pdfFilename = tmpFilename;
			}
			else throw ParseException((bl::format("Unknown PDF domain type: {1}") % bg_domain_val).str());
		}
		else
		{
			pdfFilename = this->pdfReplacementFilename;
			attachToDocument = this->pdfReplacementAttach;
		}

		this->pdfFilenameParsed = true;

		if (bf::exists(pdfFilename))
		{
			doc.readPdf(pdfFilename, false, attachToDocument);
			if (!doc.getLastErrorMsg().empty())
				cerr << bl::format("Error reading PDF: {1}") % doc.getLastErrorMsg() << endl;
		}
		else if (attachToDocument)
			this->attachedPdfMissing = true;
		else
			this->pdfMissing = pdfFilename;
	}
}

void LoadHandler::parseXmlLayer(xml_node* xml_layer)
{
	XOJ_CHECK_TYPE(LoadHandler);
	
	this->layer = new Layer();
	this->page->addLayer(this->layer);
	
	for (xml_node child : xml_layer->children())
	{
		if (child.empty()) throw ParseException("xournal.page.layer.(child)", true);
		
		string child_n = child.name();
		if		(child_n == "stroke")	parseXmlLayerStroke(&child);
		else if (child_n == "text")		parseXmlLayerText(&child);
		else if (child_n == "image")	parseXmlLayerImage(&child);
		else if (child_n == "teximage")	parseXmlLayerTexImage(&child);
		else throw ParseException((bl::format("Unknown layer element: {1}") % child_n).str());
	}
}

void LoadHandler::parseXmlLayerStroke(xml_node* child)
{
	XOJ_CHECK_TYPE(LoadHandler);

	Stroke* stroke = new Stroke();
	this->layer->addElement(stroke);

	//color
	string xml_color = child->attribute("color").value();
	if (xml_color.empty()) throw ParseException("xournal.page.layer.stroke(color)", true);
	stroke->setColor(parseColor(xml_color));

	//tool
	string xml_tool = child->attribute("tool").value();
	if (xml_tool.empty()) throw ParseException("xournal.page.layer.stroke(tool)", true);

	if (xml_tool == "eraser")			stroke->setToolType(STROKE_TOOL_ERASER);
	else if (xml_tool == "pen")			stroke->setToolType(STROKE_TOOL_PEN);
	else if (xml_tool == "highlighter")	stroke->setToolType(STROKE_TOOL_HIGHLIGHTER);
	else
	{
		cout << bl::format("Unknown stroke type: \"{1}\", assume pen") % xml_tool << endl;
		stroke->setToolType(STROKE_TOOL_PEN);
	}
	
	//width
	string xml_width = child->attribute("width").value();
	if (xml_width.empty()) throw ParseException("xournal.page.layer.stroke(width)", true);

	//used for both widths and points parsing
	string::size_type ptr;
	double tmp;
	
	std::vector<string> swidths;
	boost::split(swidths, xml_width, boost::is_any_of(" "));

	std::vector<double> widths;
	for (string w : swidths)
	{
		tmp = std::stod(w, &ptr);
		if (ptr != w.length()) throw ParseException((bl::format("Error reading width of a stroke: {1}") % xml_width).str());
		widths.push_back(tmp);
	}
	
	bool oneWidth = (widths.size() == 1);

	stroke->setWidth(widths[0]);
	
	//points
	std::vector<string> points;
	string text = child->text().get();
	boost::split(points, text, boost::is_any_of(" "));
	
	if (points.size() % 2 != 0)
		throw ParseException((bl::format("Wrong number of point dimensions: {1}")
				% points.size()).str());
	if (!oneWidth && widths.size() != (points.size() / 2))
	{
		throw ParseException((bl::format("Wrong count of points. Given {1} widths and {2} points")
				% widths.size() % points.size()).str());
	}
	
	
	bool dim = false; //dimension: false - x; true - y
	double x;
	for (int i = 0; i < points.size(); i++)
	{
		tmp = std::stod(points[i], &ptr);
		if (ptr != points[i].length())
			throw ParseException((bl::format("Stroke point in wrong format: {1}") % points[i]).str());
		
		if (dim == false)
		{
			x = tmp;
			dim = true;
		}
		else
		{
			if (oneWidth)
				stroke->addPoint(Point(x, tmp));
			else
				stroke->addPoint(Point(x, tmp, widths[(i-1)/2+1]));
			dim = false;
		}
	}
	
	stroke->freeUnusedPointItems();
}

void LoadHandler::parseXmlLayerText(xml_node* child)
{
	XOJ_CHECK_TYPE(LoadHandler);
	
	double x = child->attribute("x").as_double(-1234);
	if (x == -1234) throw ParseException("xournal.page.layer.text(x)", true);
	
	double y = child->attribute("y").as_double(-1234);
	if (y == -1234) throw ParseException("xournal.page.layer.text(y)", true);

	string font = child->attribute("font").value();
	if (font.empty()) throw ParseException("xournal.page.layer.text(font)", true);

	double size = child->attribute("size").as_double(-1234);
	if (size == -1234) throw ParseException("xournal.page.layer.text(size)", true);

	string color = child->attribute("color").value();
	if (color.empty()) throw ParseException("xournal.page.layer.text(color)", true);

	string value = child->text().get();
	if (value.empty()) throw ParseException("xournal.page.layer.text", true);
	
	Text* text = new Text();
	text->setX(x);
	text->setY(y);
	text->getFont().setName(font);
	text->getFont().setSize(size);
	text->setColor(parseColor(color));
	text->setText(value);
	
	this->layer->addElement(text);
}

void LoadHandler::parseXmlLayerImage(xml_node* child)
{
	XOJ_CHECK_TYPE(LoadHandler);
	
	double left = child->attribute("left").as_double(-1234);
	if (left == -1234) throw ParseException("xournal.page.layer.image(left)", true);
	
	double top = child->attribute("top").as_double(-1234);
	if (top == -1234) throw ParseException("xournal.page.layer.image(top)", true);
	
	double right = child->attribute("right").as_double(-1234);
	if (right == -1234) throw ParseException("xournal.page.layer.image(right)", true);
	
	double bottom = child->attribute("bottom").as_double(-1234);
	if (bottom == -1234) throw ParseException("xournal.page.layer.image(bottom)", true);
	
	string imgenc = child->text().get();
	if (imgenc.empty()) throw ParseException("xournal.page.layer.image", true);
	
	string imgdec = Glib::Base64::decode(imgenc);

	Image* image = new Image();
	image->setX(left);
	image->setY(top);
	image->setWidth(right - left);	
	image->setHeight(bottom - top);
	image->setImage(imgdec);
	
	this->layer->addElement(image);
}

void LoadHandler::parseXmlLayerTexImage(xml_node* child)
{
	XOJ_CHECK_TYPE(LoadHandler);

	double left = child->attribute("left").as_double(-1234);
	if (left == -1234) throw ParseException("xournal.page.layer.image(left)", true);
	
	double top = child->attribute("top").as_double(-1234);
	if (top == -1234) throw ParseException("xournal.page.layer.image(top)", true);
	
	double right = child->attribute("right").as_double(-1234);
	if (right == -1234) throw ParseException("xournal.page.layer.image(right)", true);
	
	double bottom = child->attribute("bottom").as_double(-1234);
	if (bottom == -1234) throw ParseException("xournal.page.layer.image(bottom)", true);

	string imText = child->attribute("text").value();
	if (imText.empty()) throw ParseException("xournal.page.layer.image(text)", true);
	
	string imgenc = child->text().get();
	if (imgenc.empty()) throw ParseException("xournal.page.layer.teximage", true);
	
	string imgdec = Glib::Base64::decode(imgenc);

	TexImage* teximage = new TexImage();
	
	teximage->setX(left);
	teximage->setY(top);
	teximage->setWidth(right - left);
	teximage->setHeight(bottom - top);
	teximage->setText(imText);
	teximage->setImage(imgdec);
	
	this->layer->addElement(teximage);
}

/**
 * Document should not be freed, it will be freed with LoadHandler!
 */
Document* LoadHandler::loadDocument(path filename)
{
	XOJ_CHECK_TYPE(LoadHandler);

	initAttributes();
	doc.clearDocument();

	if (!openFile(filename)) return NULL;

	xournalFilename = filename;

	this->pdfFilenameParsed = false;

	if (!parseXml())
	{
		closeFile();
		return NULL;
	}
	doc.setFilename(filename);

	closeFile();

	return &this->doc;
}

ParseException::ParseException(const char* attribute, bool notFound, string* value, string convError) : std::runtime_error("Parsing Exception") {
	XOJ_INIT_TYPE(ParseException);
	
	this->attribute = attribute;
	this->notFound = notFound;
	this->value = value;
	this->convError = convError;
}

ParseException::~ParseException()
{
	XOJ_RELEASE_TYPE(ParseException);
}

ParseException::ParseException(string msg) : std::runtime_error(msg)
{
	XOJ_INIT_TYPE(ParseException);
	
	this->msg = msg;
}
	
const char* ParseException::getAttribute() const {
	XOJ_CHECK_TYPE(ParseException);
	
	return this->attribute;
}

string* ParseException::getValue() const {
	XOJ_CHECK_TYPE(ParseException);
	
	return this->value;
}

bool ParseException::isNotFound() const {
	XOJ_CHECK_TYPE(ParseException);
	
	return this->notFound;
}

string ParseException::isConvError() const {
	XOJ_CHECK_TYPE(ParseException);
	
	return this->convError;
}

const char* ParseException::what() const throw () {
	XOJ_CHECK_TYPE(ParseException);
	
	if (!this->msg.empty())
	{
		return this->msg.c_str();
	}
	else if (this->notFound)
	{
		return (bl::format("Attribute \"{1}\" not found") % this->attribute).str().c_str();
	}
	else
	{
		if (this->convError.empty()) {
			return (bl::format("Unknown parse error for attribute \"{1}\" of value \"{2}\"")
					% this->attribute % this->value).str().c_str();
		}
		else
		{
			return (bl::format("Attribute \"{1}\" of value \"{2}\" cannot be parsed as {3}")
					% this->attribute % this->value % this->convError).str().c_str();
		}
	}
}
