/*
 * Xournal++
 *
 * Error handler, prints a stacktrace if Xournal++ crashes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

class Document;
class Control;

void setEmergencyDocument(const Document* doc);
void setControl(Control* ctrl);
void installCrashHandlers();
void emergencySave();
