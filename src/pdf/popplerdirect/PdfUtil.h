/*
 * Xournal++
 *
 * Part of the PDF export
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PDFUTIL_H__
#define __PDFUTIL_H__

class PdfUtil {
private:
	PdfUtil();
	virtual ~PdfUtil();

public:
	static bool isWhitespace(int c);
};

#endif /* __PDFUTIL_H__ */
