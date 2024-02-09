/*
 * Xournal++
 *
 * Input stream for reading
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


class InputStream {
protected:
    InputStream() = default;

public:
    virtual ~InputStream() = default;

    virtual int read(char* buffer, unsigned int len) noexcept = 0;
    virtual void close() = 0;
};
