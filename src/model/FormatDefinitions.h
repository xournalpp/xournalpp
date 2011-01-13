/*
 * Xournal++
 *
 * Abstract gui class, which loads the glade objcts
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __FORMATDEFINITIONS_H__
#define __FORMATDEFINITIONS_H__

typedef struct {
	const char * name;
	const double scale;
} FormatUnits;

// TODO: we have a zoom DPI calibration setting, should we use this?
#define DISPLAY_DPI_DEFAULT 96.0

extern FormatUnits XOJ_UNITS[];

extern const int XOJ_UNIT_COUNT;


typedef struct {
	const char * name;
	const double width;
	const double height;
} PaperFormat;


extern PaperFormat XOJ_FORMATS[];

extern const int XOJ_FORMAT_COUNT;
extern const int XOJ_FORMAT_CUSTOM_ID;


#endif //__FORMATDEFINITIONS_H__
