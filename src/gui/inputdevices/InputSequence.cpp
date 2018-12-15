#include "InputSequence.h"

InputSequence::InputSequence()
 : device(NULL),
   axes(NULL),
   x(-1),
   y(-1)
{
	XOJ_INIT_TYPE(InputSequence);

	printf("start input\n");
}

InputSequence::~InputSequence()
{
	XOJ_CHECK_TYPE(InputSequence);

	clearAxes();

	XOJ_RELEASE_TYPE(InputSequence);
}

/**
 * Set current input device
 */
void InputSequence::setDevice(GdkDevice* device)
{
	XOJ_CHECK_TYPE(InputSequence);

	this->device = device;
}

/**
 * Clear the last stored axes
 */
void InputSequence::clearAxes()
{
	XOJ_CHECK_TYPE(InputSequence);

	g_clear_pointer(&axes, g_free);
}

/**
 * Set the axes
 *
 * @param axes Will be handed over, and freed by InputSequence
 */
void InputSequence::setAxes(gdouble* axes)
{
	XOJ_CHECK_TYPE(InputSequence);

	clearAxes();
	this->axes = axes;
}

/**
 * Set Position
 */
void InputSequence::setCurrentPosition(double x, double y)
{
	XOJ_CHECK_TYPE(InputSequence);

	this->x = x;
	this->y = y;
}

/**
 * End / finalize input
 */
void InputSequence::endInput()
{
	XOJ_CHECK_TYPE(InputSequence);

	printf("end input\n");
}

/**
 * All data applied, do the input now
 */
void InputSequence::handleInput()
{
	XOJ_CHECK_TYPE(InputSequence);

	printf("input %s\n", gdk_device_get_name(device));
}

/**
 * Free an input sequence, used as callback for GTK
 */
void InputSequence::free(InputSequence* sequence)
{
	delete sequence;
}
