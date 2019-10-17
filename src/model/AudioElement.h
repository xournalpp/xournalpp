/*
 * Xournal++
 *
 * Element that is audio enabled
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Element.h"
#include <XournalType.h>
#include <serializing/ObjectInputStream.h>
#include <serializing/ObjectOutputStream.h>

class AudioElement : public Element
{
protected:
	AudioElement(ElementType type);

public:
	~AudioElement() override;

	void setTimestamp(size_t timestamp);
	size_t getTimestamp() const;

	void setAudioFilename(string fn);
	string getAudioFilename() const;

	virtual bool intersects(double x, double y, double halfSize) = 0;
	virtual bool intersects(double x, double y, double halfSize, double* gap) = 0;

protected:
	void serializeAudioElement(ObjectOutputStream& out);
	void readSerializedAudioElement(ObjectInputStream& in);

	void cloneAudioData(const AudioElement *other);

private:
	// Stroke timestamp, to match it to the audio stream
	size_t timestamp = 0;
	string audioFilename = "";
};


