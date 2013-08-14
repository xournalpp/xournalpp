/* poppler-action.cc: glib wrapper for poppler	      -*- c-basic-offset: 8 -*-
 * Copyright (C) 2005, Red Hat, Inc.
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

#include "poppler.h"
#include "poppler-private.h"

POPPLER_DEFINE_BOXED_TYPE (PopplerDest, poppler_dest, poppler_dest_copy, poppler_dest_free)

/**
 * poppler_dest_copy:
 * @dest: a #PopplerDest
 *
 * Copies @dest, creating an identical #PopplerDest.
 *
 * Return value: a new destination identical to @dest
 **/
PopplerDest *
poppler_dest_copy (PopplerDest *dest)
{
	PopplerDest *new_dest;

	new_dest = g_new0 (PopplerDest, 1);
	memcpy (new_dest, dest, sizeof (PopplerDest));

	if (dest->named_dest)
		new_dest->named_dest = g_strdup (dest->named_dest);

	return new_dest;
}


/**
 * poppler_dest_free:
 * @dest: a #PopplerDest
 *
 * Frees @dest
 **/
void
poppler_dest_free (PopplerDest *dest)
{
	if (!dest)
		return;
	
	if (dest->named_dest)
		g_free (dest->named_dest);
	
	g_free (dest);
}

POPPLER_DEFINE_BOXED_TYPE (PopplerAction, poppler_action, poppler_action_copy, poppler_action_free)

/**
 * poppler_action_free:
 * @action: a #PopplerAction
 * 
 * Frees @action
 **/
void
poppler_action_free (PopplerAction *action)
{
	if (action == NULL)
		return;

	/* Action specific stuff */
	switch (action->type) {
	case POPPLER_ACTION_GOTO_DEST:
		poppler_dest_free (action->goto_dest.dest);
		break;
	case POPPLER_ACTION_GOTO_REMOTE:
		poppler_dest_free (action->goto_remote.dest);
		g_free (action->goto_remote.file_name);
		break;
	case POPPLER_ACTION_URI:
		g_free (action->uri.uri);
		break;
	case POPPLER_ACTION_LAUNCH:
		g_free (action->launch.file_name);
		g_free (action->launch.params);
		break;
	case POPPLER_ACTION_NAMED:
		g_free (action->named.named_dest);
		break;
	case POPPLER_ACTION_MOVIE:
		/* TODO */
		break;
	default:
		break;
	}
	
	g_free (action->any.title);
	g_free (action);
}

/**
 * poppler_action_copy:
 * @action: a #PopplerAction
 * 
 * Copies @action, creating an identical #PopplerAction.
 * 
 * Return value: a new action identical to @action
 **/
PopplerAction *
poppler_action_copy (PopplerAction *action)
{
	PopplerAction *new_action;

	g_return_val_if_fail (action != NULL, NULL);

	/* Do a straight copy of the memory */
	new_action = g_new0 (PopplerAction, 1);
	memcpy (new_action, action, sizeof (PopplerAction));

	if (action->any.title != NULL)
		new_action->any.title = g_strdup (action->any.title);

	switch (action->type) {
	case POPPLER_ACTION_GOTO_DEST:
		new_action->goto_dest.dest = poppler_dest_copy (action->goto_dest.dest);
		break;
	case POPPLER_ACTION_GOTO_REMOTE:
		new_action->goto_remote.dest = poppler_dest_copy (action->goto_remote.dest);
		if (action->goto_remote.file_name)
			new_action->goto_remote.file_name = g_strdup (action->goto_remote.file_name);
		break;
	case POPPLER_ACTION_URI:
		if (action->uri.uri)
			new_action->uri.uri = g_strdup (action->uri.uri);
		break;
	case POPPLER_ACTION_LAUNCH:
		if (action->launch.file_name)
			new_action->launch.file_name = g_strdup (action->launch.file_name);
		if (action->launch.params)
			new_action->launch.params = g_strdup (action->launch.params);
		break;
	case POPPLER_ACTION_NAMED:
		if (action->named.named_dest)
			new_action->named.named_dest = g_strdup (action->named.named_dest);
		break;
	case POPPLER_ACTION_MOVIE:
		/* TODO */
		break;
	default:
		break;
	}
	    
	return new_action;
}

PopplerDest *
dest_new_goto (PopplerDocument *document,
	       LinkDest        *link_dest)
{
	PopplerDest *dest;

	dest = g_new0 (PopplerDest, 1);

	if (link_dest == NULL) {
		dest->type = POPPLER_DEST_UNKNOWN;
		return dest;
	}

	switch (link_dest->getKind ()) {
	case destXYZ:
		dest->type = POPPLER_DEST_XYZ;
		break;
	case destFit:
		dest->type = POPPLER_DEST_FIT;
		break;
	case destFitH:
		dest->type = POPPLER_DEST_FITH;
		break;
	case destFitV:
		dest->type = POPPLER_DEST_FITV;
		break;
	case destFitR:
		dest->type = POPPLER_DEST_FITR;
		break;
	case destFitB:
		dest->type = POPPLER_DEST_FITB;
		break;
	case destFitBH:
		dest->type = POPPLER_DEST_FITBH;
		break;
	case destFitBV:
		dest->type = POPPLER_DEST_FITBV;
		break;
	default:
		dest->type = POPPLER_DEST_UNKNOWN;
	}

	if (link_dest->isPageRef ()) {
		if (document) {
			Ref page_ref = link_dest->getPageRef ();
			dest->page_num = document->doc->findPage (page_ref.num, page_ref.gen);
		} else {
			/* FIXME: We don't keep areound the page_ref for the
			 * remote doc, so we can't look this up.  Guess that
			 * it's 0*/
			dest->page_num = 0;
		}
	} else {
		dest->page_num = link_dest->getPageNum ();
	}

	dest->left = link_dest->getLeft ();
	dest->bottom = link_dest->getBottom ();
	dest->right = link_dest->getRight ();
	dest->top = link_dest->getTop ();
	dest->zoom = link_dest->getZoom ();
	dest->change_left = link_dest->getChangeLeft ();
	dest->change_top = link_dest->getChangeTop ();
	dest->change_zoom = link_dest->getChangeZoom ();
	
	if (document && dest->page_num > 0) {
		PopplerPage *page;

		page = poppler_document_get_page (document, dest->page_num - 1);

		if (page) {
			dest->left -= page->page->getCropBox ()->x1;
			dest->bottom -= page->page->getCropBox ()->x1;
			dest->right -= page->page->getCropBox ()->y1;
			dest->top -= page->page->getCropBox ()->y1;

			g_object_unref (page);
		} else {
			g_warning ("Invalid page %d in Link Destination\n", dest->page_num);
			dest->page_num = 0;
		}
	}
	
	return dest;
}

static PopplerDest *
dest_new_named (GooString *named_dest)
{
	PopplerDest *dest;

	dest = g_new0 (PopplerDest, 1);

	if (named_dest == NULL) {
		dest->type = POPPLER_DEST_UNKNOWN;
		return dest;
	}

	dest->type = POPPLER_DEST_NAMED;
	dest->named_dest = g_strdup (named_dest->getCString ());

	return dest;
}

static void
build_goto_dest (PopplerDocument *document,
		 PopplerAction   *action,
		 LinkGoTo        *link)
{
	LinkDest *link_dest;
	GooString *named_dest;

	/* Return if it isn't OK */
	if (! link->isOk ()) {
		action->goto_dest.dest = dest_new_goto (NULL, NULL);
		return;
	}
	
	link_dest = link->getDest ();
	named_dest = link->getNamedDest ();

	if (link_dest != NULL) {
		action->goto_dest.dest = dest_new_goto (document, link_dest);
	} else if (named_dest != NULL) {
		action->goto_dest.dest = dest_new_named (named_dest);
	} else {
		action->goto_dest.dest = dest_new_goto (document, NULL);
	}
}

static void
build_goto_remote (PopplerAction *action,
		   LinkGoToR     *link)
{
	LinkDest *link_dest;
	GooString *named_dest;
	
	/* Return if it isn't OK */
	if (! link->isOk ()) {
		action->goto_remote.dest = dest_new_goto (NULL, NULL);
		return;
	}

	action->goto_remote.file_name = _poppler_goo_string_to_utf8 (link->getFileName());

	link_dest = link->getDest ();
	named_dest = link->getNamedDest ();
	
	if (link_dest != NULL) {
		action->goto_remote.dest = dest_new_goto (NULL, link_dest);
	} else if (named_dest != NULL) {
		action->goto_remote.dest = dest_new_named (named_dest);
	} else {
		action->goto_remote.dest = dest_new_goto (NULL, NULL);
	}
}

static void
build_launch (PopplerAction *action,
	      LinkLaunch    *link)
{
	if (link->getFileName()) {
		action->launch.file_name = g_strdup (link->getFileName()->getCString ());
	}
	if (link->getParams()) {
		action->launch.params = g_strdup (link->getParams()->getCString ());
	}
}

static void
build_uri (PopplerAction *action,
	   LinkURI       *link)
{
	gchar *uri;

	uri = link->getURI()->getCString ();
	if (uri != NULL)
		action->uri.uri = g_strdup (uri);
}

static void
build_named (PopplerAction *action,
	     LinkNamed     *link)
{
	gchar *name;

	name = link->getName ()->getCString ();
	if (name != NULL)
		action->named.named_dest = g_strdup (name);
}

static void
build_movie (PopplerAction *action,
	     LinkAction    *link)
{
	/* FIXME: Write */
}

PopplerAction *
_poppler_action_new (PopplerDocument *document,
		     LinkAction      *link,
		     const gchar     *title)
{
	PopplerAction *action;

	action = g_new0 (PopplerAction, 1);

	if (title)
		action->any.title = g_strdup (title);

	if (link == NULL) {
		action->type = POPPLER_ACTION_NONE;
		return action;
	}

	switch (link->getKind ()) {
	case actionGoTo:
		action->type = POPPLER_ACTION_GOTO_DEST;
		build_goto_dest (document, action, dynamic_cast <LinkGoTo *> (link));
		break;
	case actionGoToR:
		action->type = POPPLER_ACTION_GOTO_REMOTE;
		build_goto_remote (action, dynamic_cast <LinkGoToR *> (link));
		break;
	case actionLaunch:
		action->type = POPPLER_ACTION_LAUNCH;
		build_launch (action, dynamic_cast <LinkLaunch *> (link));
		break;
	case actionURI:
		action->type = POPPLER_ACTION_URI;
		build_uri (action, dynamic_cast <LinkURI *> (link));
		break;
	case actionNamed:
		action->type = POPPLER_ACTION_NAMED;
		build_named (action, dynamic_cast <LinkNamed *> (link));
		break;
	case actionMovie:
		action->type = POPPLER_ACTION_MOVIE;
		build_movie (action, link);
		break;
	case actionUnknown:
	default:
		action->type = POPPLER_ACTION_UNKNOWN;
		break;
	}

	return action;
}

PopplerDest *
_poppler_dest_new_goto (PopplerDocument *document,
			LinkDest        *link_dest)
{
	return dest_new_goto (document, link_dest);
}
