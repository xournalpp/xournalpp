#include "LineStyle.h"

#include <serializing/ObjectOutputStream.h>
#include <serializing/ObjectInputStream.h>


LineStyle::LineStyle()
{
}

LineStyle::LineStyle(const LineStyle& other)
{
	*this = other;
}

LineStyle::~LineStyle()
{
	g_free(this->dashes);
	this->dashes = nullptr;
	this->dashCount = 0;
}

void LineStyle::operator=(const LineStyle& other)
{
	const double* dashes = nullptr;
	int dashCount = 0;

	other.getDashes(dashes, dashCount);
	setDashes(dashes, dashCount);
}


void LineStyle::serialize(ObjectOutputStream& out)
{
	out.writeObject("LineStyle");

	out.writeData(this->dashes, this->dashCount, sizeof(double));

	out.endObject();
}

void LineStyle::readSerialized(ObjectInputStream& in)
{
	in.readObject("LineStyle");

	g_free(this->dashes);
	this->dashes = nullptr;
	this->dashCount = 0;
	in.readData((void**) &this->dashes, &this->dashCount);

	in.endObject();
}

/**
 * Get dash array and count
 *
 * @return true if dashed
 */
bool LineStyle::getDashes(const double*& dashes, int& dashCount) const
{
	dashes = this->dashes;
	dashCount = this->dashCount;

	return this->dashCount > 0;
}

/**
 * Set the dash array and count
 *
 * @param dashes Dash data, will be copied
 * @param dashCount Count of entries
 */
void LineStyle::setDashes(const double* dashes, int dashCount)
{
	g_free(this->dashes);
	if (dashCount == 0 || dashes == nullptr)
	{
		this->dashCount = 0;
		this->dashes = nullptr;
		return;
	}

	this->dashes = (double*)g_malloc(dashCount * sizeof(double));
	this->dashCount = dashCount;

	memcpy(this->dashes, dashes, this->dashCount * sizeof(double));
}

/**
 * Get dash array and count
 *
 * @return true if dashed
 */
bool LineStyle::hasDashes() const
{
	return this->dashCount > 0;
}
