/*
 * Xournal++
 *
 * Abstract input stream for reading
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


namespace xoj::util {

class InputStream {
protected:
    InputStream() = default;

public:
    virtual ~InputStream() = default;

    /**
     * Read bytes from the input stream
     *
     * @param  buffer A buffer to which the stream contents should be written
     * @param  len    The maximal number of bytes that can be written
     * @return The number of bytes that was effectively read if successful, 0 if
     *         EOF was reached, -1 on error.
     */
    virtual int read(char* buffer, unsigned int len) noexcept = 0;
    virtual void close() = 0;
};

}  // namespace xoj::util
