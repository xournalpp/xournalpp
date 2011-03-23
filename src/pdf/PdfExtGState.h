/*
 * Xournal++
 *
 * Handles PDF Export
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
// TODO: AA: type check

#ifndef __PDFEXTGSTATE_H__
#define __PDFEXTGSTATE_H__

class PdfExtGState {
public:
	PdfExtGState(int id);
	virtual ~PdfExtGState();

public:
	int id;
};

#endif /* __PDFEXTGSTATE_H__ */
