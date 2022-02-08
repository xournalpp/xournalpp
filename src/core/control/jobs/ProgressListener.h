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
    virtual void setMaximumState(int max) = 0;
    virtual void setCurrentState(int state) = 0;

    virtual ~ProgressListener(){};
};

class DummyProgressListener: public ProgressListener {
public:
    void setMaximumState(int max) override{};
    void setCurrentState(int state) override{};

    ~DummyProgressListener() override{};
};
