/* poppler-action.h: glib interface to poppler
 * Copyright (C) 2004, Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __POPPLER_ACTION_H__
#define __POPPLER_ACTION_H__

#include <glib-object.h>
#include "poppler.h"

G_BEGIN_DECLS

typedef enum
{
	POPPLER_ACTION_UNKNOWN,		/* unknown action */
	POPPLER_ACTION_NONE,            /* no action specified */
	POPPLER_ACTION_GOTO_DEST,	/* go to destination */
	POPPLER_ACTION_GOTO_REMOTE,	/* go to destination in new file */
	POPPLER_ACTION_LAUNCH,		/* launch app (or open document) */
	POPPLER_ACTION_URI,		/* URI */
	POPPLER_ACTION_NAMED,		/* named action*/
	POPPLER_ACTION_MOVIE		/* movie action */
} PopplerActionType;

typedef enum
{
	POPPLER_DEST_UNKNOWN,
	POPPLER_DEST_XYZ,
	POPPLER_DEST_FIT,
	POPPLER_DEST_FITH,
	POPPLER_DEST_FITV,
	POPPLER_DEST_FITR,
	POPPLER_DEST_FITB,
	POPPLER_DEST_FITBH,
	POPPLER_DEST_FITBV,
	POPPLER_DEST_NAMED
} PopplerDestType;

/* Define the PopplerAction types */
typedef struct _PopplerActionAny        PopplerActionAny;
typedef struct _PopplerActionGotoDest   PopplerActionGotoDest;
typedef struct _PopplerActionGotoRemote PopplerActionGotoRemote;
typedef struct _PopplerActionLaunch     PopplerActionLaunch;
typedef struct _PopplerActionUri        PopplerActionUri;
typedef struct _PopplerActionNamed      PopplerActionNamed;
typedef struct _PopplerActionMovie      PopplerActionMovie;

struct _PopplerDest
{
	PopplerDestType type;

	int page_num;
	double left;
	double bottom;
	double right;
	double top;
	double zoom;
	gchar *named_dest;
	guint change_left : 1;
	guint change_top : 1;
	guint change_zoom : 1;
};


struct _PopplerActionAny
{
	PopplerActionType type;
	gchar *title;
};

struct _PopplerActionGotoDest
{
	PopplerActionType type;
	gchar *title;

	PopplerDest *dest;
};

struct _PopplerActionGotoRemote
{
	PopplerActionType type;
	gchar *title;

	gchar *file_name;
	PopplerDest *dest;
};

struct _PopplerActionLaunch
{
	PopplerActionType type;
	gchar *title;

	gchar *file_name;
	gchar *params;
};

struct _PopplerActionUri
{
	PopplerActionType type;
	gchar *title;

	char *uri;
};

struct _PopplerActionNamed
{
	PopplerActionType type;
	gchar *title;

	gchar *named_dest;
};

struct _PopplerActionMovie
{
	PopplerActionType type;
	gchar *title;
};

union _PopplerAction
{
	PopplerActionType type;
	PopplerActionAny any;
	PopplerActionGotoDest goto_dest;
	PopplerActionGotoRemote goto_remote;
	PopplerActionLaunch launch;
	PopplerActionUri uri;
	PopplerActionNamed named;
	PopplerActionMovie movie;
};

#define POPPLER_TYPE_ACTION             (poppler_action_get_type ())
#define POPPLER_ACTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_ACTION, PopplerAction))

GType          poppler_action_get_type (void) G_GNUC_CONST;

void           poppler_action_free     (PopplerAction *action);
PopplerAction *poppler_action_copy     (PopplerAction *action);


#define POPPLER_TYPE_DEST              (poppler_dest_get_type ())
GType          poppler_dest_get_type   (void) G_GNUC_CONST;

void           poppler_dest_free       (PopplerDest   *dest);
PopplerDest   *poppler_dest_copy       (PopplerDest   *dest);

G_END_DECLS

#endif /* __POPPLER_GLIB_H__ */
