/*
 * Xournal++
 *
 * Interface for progress state
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

class ProgressListener {
public:
    virtual void setMaximumState(size_t max) = 0;
    virtual void setCurrentState(size_t state) = 0;

    virtual ~ProgressListener(){};
};

class DummyProgressListener: public ProgressListener {
public:
    void setMaximumState(size_t max) override{};
    void setCurrentState(size_t state) override{};

    ~DummyProgressListener() override{};
};
