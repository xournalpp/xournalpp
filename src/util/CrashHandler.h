/*
 * Xournal++
 *
 * Error handler, prints a stacktrace if Xournal++ crashes
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __CRASH_HANDLER_H__
#define __CRASH_HANDLER_H__

class Document;
void setEmergencyDocument(Document * doc);
void installCrashHandlers(void);

#endif // __CRASH_HANDLER_H__
