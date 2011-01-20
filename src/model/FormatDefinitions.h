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

extern const FormatUnits XOJ_UNITS[];
extern const int XOJ_UNIT_COUNT;

#endif //__FORMATDEFINITIONS_H__
