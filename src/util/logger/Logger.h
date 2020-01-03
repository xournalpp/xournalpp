/*
 * Xournal++
 *
 * Logging class for debugging
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

class Log {
private:
    Log();
    virtual ~Log();
    Log(const Log&);
    Log& operator=(const Log&);

public:
    static void initlog();
    static void closelog();

    static void trace(const char* callType, const char* clazz, const char* function, long obj);
};
