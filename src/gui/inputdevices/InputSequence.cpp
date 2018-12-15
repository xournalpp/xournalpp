#include "InputSequence.h"

InputSequence::InputSequence()
 : device(NULL),
   axes(NULL),
   x(-1),
   y(-1)
{
	XOJ_INIT_TYPE(InputSequence);
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
 * Copy axes from event
 */
void InputSequence::copyAxes(GdkEvent* event)
{
	XOJ_CHECK_TYPE(InputSequence);

	clearAxes();
	setAxes((gdouble*)g_memdup(event->motion.axes, sizeof(gdouble) * gdk_device_get_n_axes(device)));
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
 * Mouse / Pen / Touch move
 */
void InputSequence::actionMoved()
{
	XOJ_CHECK_TYPE(InputSequence);

	printf("actionMoved %s\n", gdk_device_get_name(device));
}

/**
 * Mouse / Pen down / touch start
 */
void InputSequence::actionStart()
{
	XOJ_CHECK_TYPE(InputSequence);

	printf("actionStart %s\n", gdk_device_get_name(device));
}

/**
 * Mouse / Pen up / touch end
 */
void InputSequence::actionEnd()
{
	XOJ_CHECK_TYPE(InputSequence);

	printf("actionEnd %s\n", gdk_device_get_name(device));
}

/**
 * Free an input sequence, used as callback for GTK
 */
void InputSequence::free(InputSequence* sequence)
{
	delete sequence;
}
