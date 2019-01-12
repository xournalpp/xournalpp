#include "LineStyle.h"

#include <serializing/ObjectOutputStream.h>
#include <serializing/ObjectInputStream.h>


LineStyle::LineStyle()
{
	XOJ_INIT_TYPE(LineStyle);
}

LineStyle::LineStyle(const LineStyle& other)
{
	XOJ_INIT_TYPE(LineStyle);
	*this = other;
}

LineStyle::~LineStyle()
{
	XOJ_CHECK_TYPE(LineStyle);

	g_free(this->dashes);
	this->dashes = NULL;
	this->dashCount = 0;

	XOJ_RELEASE_TYPE(LineStyle);
}

void LineStyle::operator=(const LineStyle& other)
{
	XOJ_CHECK_TYPE(LineStyle);

	const double* dashes = NULL;
	int dashCount = 0;

	other.getDashes(dashes, dashCount);
	setDashes(dashes, dashCount);
}


void LineStyle::serialize(ObjectOutputStream& out)
{
	XOJ_CHECK_TYPE(LineStyle);

	out.writeObject("LineStyle");

	out.writeData(this->dashes, this->dashCount, sizeof(double));

	out.endObject();
}

void LineStyle::readSerialized(ObjectInputStream& in)
{
	XOJ_CHECK_TYPE(LineStyle);

	in.readObject("LineStyle");

	g_free(this->dashes);
	this->dashes = NULL;
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
	XOJ_CHECK_TYPE(LineStyle);

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
	XOJ_CHECK_TYPE(LineStyle);

	g_free(this->dashes);
	if (dashCount == 0 || dashes == NULL)
	{
		this->dashCount = 0;
		this->dashes = NULL;
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
	XOJ_CHECK_TYPE(LineStyle);

	return this->dashCount > 0;
}
