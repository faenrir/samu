#include <X11/Xlib.h>
#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/extensions/Xinerama.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/randr.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_cursor.h>

#include "arg.h"
#include "wm.h"
#include "types.h"
#include "samu.h"
#include "config.h"

#define LEN(x) (sizeof(x)/sizeof(x[0]))

struct xatom {
	char *name;
	xcb_atom_t atom;
};

struct xgeom {
	int x, y, w, h, b;
};

enum EWMH_TYPES {
	IGNORE,
	NORMAL,
	POPUP,
};

enum {
	_NET_SUPPORTED,
	_NET_CLIENT_LIST,
	_NET_CLIENT_LIST_STACKING,
	_NET_SUPPORTING_WM_CHECK,
	_NET_ACTIVE_WINDOW,
	_NET_WM_STATE,
	_NET_WM_STATE_FULLSCREEN,
	_NET_WM_WINDOW_TYPE,
	_NET_WM_WINDOW_TYPE_DESKTOP,
	_NET_WM_WINDOW_TYPE_DOCK,
	_NET_WM_WINDOW_TYPE_TOOLBAR,
	_NET_WM_WINDOW_TYPE_MENU,
	_NET_WM_WINDOW_TYPE_UTILITY,
	_NET_WM_WINDOW_TYPE_SPLASH,
	_NET_WM_WINDOW_TYPE_DIALOG,
	_NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
	_NET_WM_WINDOW_TYPE_POPUP_MENU,
	_NET_WM_WINDOW_TYPE_TOOLTIP,
	_NET_WM_WINDOW_TYPE_NOTIFICATION,
	_NET_WM_WINDOW_TYPE_COMBO,
	_NET_WM_WINDOW_TYPE_DND,
	_NET_WM_WINDOW_TYPE_NORMAL,
	_NET_CURRENT_DESKTOP,
	_NET_NUMBER_OF_DESKTOPS
};

static void usage(const char *);
static int ewmh_init();
static int ewmh_wipe();
static int ewmh_supported();
static int ewmh_supportingwmcheck();
static int ewmh_activewindow(xcb_window_t);
static int ewmh_clientlist();
static int ewmh_type(xcb_window_t);
static int ewmh_message(xcb_client_message_event_t *);
static int ewmh_fullscreen(xcb_window_t, int);


static client       *list = {0}, *ws_list[NUM_WS] = {0}, *cur;
static int          ws = 0, sw, sh, wx, wy, numlock = 0, monitors;
static unsigned int ww, wh;

static int          s;
static Display      *d;
static XButtonEvent mouse;
enum { MOVING = 1, SIZING = 2 } drag;
static Window       root;

static void (*events[LASTEvent])(XEvent *e) = {
    [ButtonPress]      = button_press,
    [ButtonRelease]    = button_release,
    [ConfigureRequest] = configure_request,
    [KeyPress]         = key_press,
    [MapRequest]       = map_request,
    [MappingNotify]    = mapping_notify,
    [DestroyNotify]    = notify_destroy,
    [EnterNotify]      = notify_enter,
    [MotionNotify]     = notify_motion
};

xcb_connection_t *conn;
xcb_screen_t     *scrn;
xcb_window_t      ewmhwid; /* _NET_SUPPORTING_WM_CHECK target window */

struct xatom ewmh[] = {
	[_NET_SUPPORTED]                    = { .name = "_NET_SUPPORTED"                    },
	[_NET_CLIENT_LIST]                  = { .name = "_NET_CLIENT_LIST"                  },
	[_NET_CLIENT_LIST_STACKING]         = { .name = "_NET_CLIENT_LIST_STACKING"         },
	[_NET_SUPPORTING_WM_CHECK]          = { .name = "_NET_SUPPORTING_WM_CHECK"          },
	[_NET_ACTIVE_WINDOW]                = { .name = "_NET_ACTIVE_WINDOW"                },
	[_NET_WM_STATE]                     = { .name = "_NET_WM_STATE"                     },
	[_NET_WM_STATE_FULLSCREEN]          = { .name = "_NET_WM_STATE_FULLSCREEN"          },
	[_NET_WM_WINDOW_TYPE]               = { .name = "_NET_WM_WINDOW_TYPE"               },
	[_NET_WM_WINDOW_TYPE_DESKTOP]       = { .name = "_NET_WM_WINDOW_TYPE_DESKTOP"       },
	[_NET_WM_WINDOW_TYPE_DOCK]          = { .name = "_NET_WM_WINDOW_TYPE_DOCK"          },
	[_NET_WM_WINDOW_TYPE_TOOLBAR]       = { .name = "_NET_WM_WINDOW_TYPE_TOOLBAR"       },
	[_NET_WM_WINDOW_TYPE_MENU]          = { .name = "_NET_WM_WINDOW_TYPE_MENU"          },
	[_NET_WM_WINDOW_TYPE_UTILITY]       = { .name = "_NET_WM_WINDOW_TYPE_UTILITY"       },
	[_NET_WM_WINDOW_TYPE_SPLASH]        = { .name = "_NET_WM_WINDOW_TYPE_SPLASH"        },
	[_NET_WM_WINDOW_TYPE_DIALOG]        = { .name = "_NET_WM_WINDOW_TYPE_DIALOG"        },
	[_NET_WM_WINDOW_TYPE_DROPDOWN_MENU] = { .name = "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU" },
	[_NET_WM_WINDOW_TYPE_POPUP_MENU]    = { .name = "_NET_WM_WINDOW_TYPE_POPUP_MENU"    },
	[_NET_WM_WINDOW_TYPE_TOOLTIP]       = { .name = "_NET_WM_WINDOW_TYPE_TOOLTIP"       },
	[_NET_WM_WINDOW_TYPE_NOTIFICATION]  = { .name = "_NET_WM_WINDOW_TYPE_NOTIFICATION"  },
	[_NET_WM_WINDOW_TYPE_COMBO]         = { .name = "_NET_WM_WINDOW_TYPE_COMBO"         },
	[_NET_WM_WINDOW_TYPE_DND]           = { .name = "_NET_WM_WINDOW_TYPE_DND"           },
	[_NET_WM_WINDOW_TYPE_NORMAL]        = { .name = "_NET_WM_WINDOW_TYPE_NORMAL"        },
	[_NET_CURRENT_DESKTOP]              = { .name = "_NET_CURRENT_DESKTOP"              },
	[_NET_NUMBER_OF_DESKTOPS]           = { .name = "_NET_NUMBER_OF_DESKTOPS"           },
};

#include "config.h"

int
wm_init_xcb()
{
	conn = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(conn))
		return -1;
	return 0;
}

int
wm_kill_xcb()
{
	if (!conn)
		return -1;
	xcb_disconnect(conn);
	return 0;
}

int
wm_is_alive(xcb_window_t wid)
{
	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t  *r;

	c = xcb_get_window_attributes(conn, wid);
	r = xcb_get_window_attributes_reply(conn, c, NULL);

	if (r == NULL)
		return 0;

	free(r);
	return 1;
}

int
wm_is_mapped(xcb_window_t wid)
{
	int ms;
	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t  *r;

	c = xcb_get_window_attributes(conn, wid);
	r = xcb_get_window_attributes_reply(conn, c, NULL);

	if (r == NULL)
		return 0;

	ms = r->map_state;

	free(r);
	return ms == XCB_MAP_STATE_VIEWABLE;
}

int
wm_is_ignored(xcb_window_t wid)
{
	int or;
	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t  *r;

	c = xcb_get_window_attributes(conn, wid);
	r = xcb_get_window_attributes_reply(conn, c, NULL);

	if (r == NULL)
		return 0;

	or = r->override_redirect;

	free(r);
	return or;
}

int
wm_is_listable(xcb_window_t wid, int mask)
{
	if (!mask && wm_is_mapped (wid) && !wm_is_ignored(wid))
		return 1;
	if ((mask & LIST_ALL))
		return 1;
	if (!wm_is_mapped (wid) && mask & LIST_HIDDEN)
		return 1;
	if (wm_is_ignored(wid) && mask & LIST_IGNORE)
		return 1;

	return 0;
}

int
wm_get_screen()
{
	scrn = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
	if (scrn == NULL)
		return -1;
	return 0;
}

int
wm_get_windows(xcb_window_t wid, xcb_window_t **l)
{
	uint32_t childnum = 0;
	xcb_query_tree_cookie_t c;
	xcb_query_tree_reply_t *r;

	c = xcb_query_tree(conn, wid);
	r = xcb_query_tree_reply(conn, c, NULL);
	if (r == NULL)
		return -1;

	*l = malloc(sizeof(xcb_window_t) * r->children_len);
	memcpy(*l, xcb_query_tree_children(r),
			sizeof(xcb_window_t) * r->children_len);

	childnum = r->children_len;

	free(r);
	return childnum;
}

xcb_window_t
wm_get_focus(void)
{
	xcb_window_t wid = 0;
	xcb_get_input_focus_cookie_t c;
	xcb_get_input_focus_reply_t *r;

	c = xcb_get_input_focus(conn);
	r = xcb_get_input_focus_reply(conn, c, NULL);
	if (r == NULL)
		return scrn->root;

	wid = r->focus;
	free(r);
	return wid;
}


int
wm_get_attribute(xcb_window_t wid, int attr)
{
	xcb_get_geometry_cookie_t c;
	xcb_get_geometry_reply_t *r;

	c = xcb_get_geometry(conn, wid);
	r = xcb_get_geometry_reply(conn, c, NULL);

	if (r == NULL)
		return -1;

	switch (attr) {
	case ATTR_X:
		attr = r->x;
		break;
	case ATTR_Y:
		attr = r->y;
		break;
	case ATTR_W:
		attr = r->width;
		break;
	case ATTR_H:
		attr = r->height;
		break;
	case ATTR_B:
		attr = r->border_width;
		break;
	}

	free(r);
	return attr;
}

xcb_atom_t
wm_add_atom(char *name, size_t len)
{
	xcb_atom_t atom;
	xcb_intern_atom_cookie_t c;
	xcb_intern_atom_reply_t *r;

	c = xcb_intern_atom(conn, 0, len, name);
	r = xcb_intern_atom_reply(conn, c, NULL);
	if (!r)
		return 0;

	atom = r->atom;
	free(r);

	return atom;
}

int
wm_set_atom(xcb_window_t wid, xcb_atom_t atom, xcb_atom_t type, size_t len, void *data)
{
	int errcode;
	xcb_void_cookie_t c;
	xcb_generic_error_t *e;

	c = xcb_change_property_checked(conn, XCB_PROP_MODE_REPLACE,
		wid, atom, type, 32, len, data);
	e = xcb_request_check(conn, c);
	if (!e)
		return 0;

	errcode = e->error_code;
	free(e);

	return errcode;
}

void *
wm_get_atom(xcb_window_t wid, xcb_atom_t atom, xcb_atom_t type, size_t *len)
{
	void *d;
	size_t n;
	xcb_get_property_cookie_t c;
	xcb_get_property_reply_t *r;

	c = xcb_get_property(conn, 0, wid, atom, type, 0, 32);
	r = xcb_get_property_reply(conn, c, NULL);
	if (!r)
		return NULL;

	if (!(n = xcb_get_property_value_length(r))) {
		free(r);
		return NULL;
	}

	if (len)
		*len = n;

	d = xcb_get_property_value(r);

	return d;
}

char *
wm_get_atom_name(xcb_atom_t atom, size_t *len)
{
	size_t n;
	char *name;
	xcb_get_atom_name_cookie_t c;
	xcb_get_atom_name_reply_t *r;

	c = xcb_get_atom_name(conn, atom);
	r = xcb_get_atom_name_reply(conn, c, NULL);
	if (!r)
		return NULL;

	n = xcb_get_atom_name_name_length(r) + 1;
	name = malloc(xcb_get_atom_name_name_length(r) + 1);
	if (!name) {
		free(r);
		return NULL;
	}

	if (len)
		*len = n;

	memset(name, 0, xcb_get_atom_name_name_length(r) + 1);
	strncpy(name, xcb_get_atom_name_name(r), xcb_get_atom_name_name_length(r));
	free(r);

	return name;
}


int
wm_get_cursor(int mode, uint32_t wid, int *x, int *y)
{
	xcb_query_pointer_reply_t *r;
	xcb_query_pointer_cookie_t c;

	c = xcb_query_pointer(conn, wid);
	r = xcb_query_pointer_reply(conn, c, NULL);

	if (r == NULL)
		return -1;

	if (r->child != XCB_NONE) {
		*x = r->win_x;
		*y = r->win_y;
	} else {
		*x = r->root_x;
		*y = r->root_y;
	}

	return 0;
}

int
wm_set_border(int width, int color, xcb_window_t wid)
{
	uint32_t values[1];
	int mask;

	/* change width if >= 0 */
	if (width > -1) {
		values[0] = width;
		mask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
		xcb_configure_window(conn, wid, mask, values);
	}

	/*
	 * color is an ARGB representation (eg. 0x80ff0000) for
	 * translucent red.
	 * Absolutely all values are valid color representations, so we
	 * will set it no matter what.
	 */
	values[0] = color;
	mask = XCB_CW_BORDER_PIXEL;
	xcb_change_window_attributes(conn, wid, mask, values);

	return 0;
}

int
wm_set_cursor(int x, int y, int mode)
{
	xcb_warp_pointer(conn, XCB_NONE, mode ? XCB_NONE : scrn->root,
			0, 0, 0, 0, x, y);
	return 0;
}

int
wm_teleport(xcb_window_t wid, int x, int y, int w, int h)
{
	uint32_t values[4];
	uint32_t mask =   XCB_CONFIG_WINDOW_X
	                | XCB_CONFIG_WINDOW_Y
	                | XCB_CONFIG_WINDOW_WIDTH
	                | XCB_CONFIG_WINDOW_HEIGHT;
	values[0] = x;
	values[1] = y;
	values[2] = w;
	values[3] = h;
	xcb_configure_window(conn, wid, mask, values);

	return 0;
}

int
wm_move(xcb_window_t wid, int mode, int x, int y)
{
	int curx, cury, curw, curh, curb;

	if (!wm_is_mapped(wid) || wid == scrn->root)
		return -1;

	curb = wm_get_attribute(wid, ATTR_B);
	curx = wm_get_attribute(wid, ATTR_X);
	cury = wm_get_attribute(wid, ATTR_Y);
	curw = wm_get_attribute(wid, ATTR_W);
	curh = wm_get_attribute(wid, ATTR_H);

	if (mode == RELATIVE) {
		x += curx;
		y += cury;
	}

	/* the following prevent windows from moving off the screen */
	if (x < 0)
		x = 0;
	else if (x > scrn->width_in_pixels - curw - 2*curb)
		x = scrn->width_in_pixels - curw - 2*curb;

	if (y < 0)
		y = 0;
	else if (y > scrn->height_in_pixels - curh - 2*curb)
		y = scrn->height_in_pixels - curh - 2*curb;

	wm_teleport(wid, x, y, curw, curh);
	return 0;
}

int
wm_set_override(xcb_window_t wid, int or)
{
	uint32_t mask = XCB_CW_OVERRIDE_REDIRECT;
	uint32_t val[] = { or };

	xcb_change_window_attributes(conn, wid, mask, val);

	return 0;
}


int
wm_remap(xcb_window_t wid, int mode)
{
	switch (mode) {
	case MAP:
		xcb_map_window(conn, wid);
		break;
	case UNMAP:
		xcb_unmap_window(conn, wid);
		break;
	case TOGGLE:
		if (wm_is_mapped(wid))
			xcb_unmap_window(conn, wid);
		else
			xcb_map_window(conn, wid);
		break;
	}

	return 0;
}

int
wm_resize(xcb_window_t wid, int mode, int w, int h)
{
	int curx, cury, curw, curh, curb;

	if (!wm_is_mapped(wid) || wid == scrn->root)
		return -1;

	curb = wm_get_attribute(wid, ATTR_B);
	curx = wm_get_attribute(wid, ATTR_X);
	cury = wm_get_attribute(wid, ATTR_Y);
	curw = wm_get_attribute(wid, ATTR_W);
	curh = wm_get_attribute(wid, ATTR_H);

	if (mode == RELATIVE) {
		w += curw;
		h += curh;
	} else {
		w -= curx;
		h -= cury;
	}

	/*
	 * The following prevent windows from growing out of the screen, or
	 * having a negative size
	 */
	if (w < 0)
		w = curw;
	if (curx + w >  scrn->width_in_pixels)
		w = scrn->width_in_pixels - curx - 2*curb;

	if (h < 0)
		h = curh;
	if (cury + h > scrn->height_in_pixels)
		h = scrn->height_in_pixels - cury - 2*curb;

	wm_teleport(wid, curx, cury, w, h);
	return 0;
}

int
wm_restack(xcb_window_t wid, uint32_t mode)
{
	uint32_t values[1] = { mode };
	xcb_configure_window(conn, wid, XCB_CONFIG_WINDOW_STACK_MODE, values);
	return 0;
}

int
wm_set_focus(xcb_window_t wid)
{
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, wid,
	                    XCB_CURRENT_TIME);
	return 0;
}

int
wm_reg_window_event(xcb_window_t wid, uint32_t mask)
{
	uint32_t val[] = { mask };
	xcb_void_cookie_t c;
	xcb_generic_error_t *e;

	c = xcb_change_window_attributes_checked(conn, wid, XCB_CW_EVENT_MASK, val);
	e = xcb_request_check(conn, c);
	if (!e)
		return -1;

	free(e);
	return 0;
}


int
wm_reg_cursor_event(xcb_window_t wid, uint32_t mask, char *cursor)
{
	xcb_cursor_t p;
	xcb_cursor_context_t *cx;
	xcb_grab_pointer_cookie_t c;
	xcb_grab_pointer_reply_t *r;

	p = XCB_NONE;
	if (cursor) {
		if (xcb_cursor_context_new(conn, scrn, &cx) < 0)
			return -1;

		p = xcb_cursor_load_cursor(cx, cursor);
	}

	c = xcb_grab_pointer(conn, 1, scrn->root, mask,
		XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
		XCB_NONE, p, XCB_CURRENT_TIME);

	r = xcb_grab_pointer_reply(conn, c, NULL);
	if (!r || r->status != XCB_GRAB_STATUS_SUCCESS)
		return -1;

	xcb_cursor_context_free(cx);
	return 0;
}

int
wm_get_monitors(xcb_window_t wid, int *l)
{
	int n;
	xcb_randr_get_monitors_cookie_t c;
	xcb_randr_get_monitors_reply_t *r;
	xcb_randr_monitor_info_iterator_t i;

	/* get_active: ignore inactive monitors */
	c = xcb_randr_get_monitors(conn, wid, 0);
	r = xcb_randr_get_monitors_reply(conn, c, NULL);
	if (!r)
		return -1;

	i = xcb_randr_get_monitors_monitors_iterator(r);
	if (!i.data)
		return 0;

	for (n = 0; l && i.rem > 0; xcb_randr_monitor_info_next(&i))
		l[n++] = i.index;

	n = r->nMonitors;
	free(r);

	return n;
}

xcb_randr_monitor_info_t *
wm_get_monitor(int index)
{
	xcb_randr_monitor_info_t *monitor;
	xcb_randr_get_monitors_cookie_t c;
	xcb_randr_get_monitors_reply_t *r;
	xcb_randr_monitor_info_iterator_t i;

	/* get_active: ignore inactive monitors */
	c = xcb_randr_get_monitors(conn, scrn->root, 0);
	r = xcb_randr_get_monitors_reply(conn, c, NULL);
	if (!r)
		return NULL;

	i = xcb_randr_get_monitors_monitors_iterator(r);
	if (!i.data)
		return NULL;

	for (; i.rem > 0; xcb_randr_monitor_info_next(&i)) {
		if (i.index != index)
			continue;

		monitor = calloc(1, sizeof(*monitor));
		if (!monitor)
			return NULL;

		memcpy(monitor, i.data, sizeof(*monitor));
		free(r);
		return monitor;
	}

	free(r);
	return NULL;
}

int
wm_find_monitor(int x, int y)
{
	/* patch me if you use more than 64 monitors, and get a reward! */
	int i, n, monitors[64];
	xcb_randr_monitor_info_t *p;

	n = wm_get_monitors(scrn->root, monitors);
	for (i = 0; i < n; i++) {
		p = wm_get_monitor(monitors[i]);
		if (!p)
			continue;

		if (p->x <= x && p->x + p->width  >= x
		 && p->y <= y && p->y + p->height >= y) {
			free(p);
			return monitors[i];
		}
		free(p);
	}

	return -1;
}

void
cleanup()
{
	printf("cleaning up\n");
	ewmh_wipe();
	wm_kill_xcb();
}

int
ewmh_init()
{
	uint32_t i, n;
	xcb_window_t *w;

	for (i = 0; i < LEN(ewmh); i++)
		ewmh[i].atom = wm_add_atom(ewmh[i].name, strlen(ewmh[i].name));

	/* monitor focus events on existing windows */
	n = wm_get_windows(scrn->root, &w);
	for (i = 0; i < n; i++)
		wm_reg_window_event(w[i], XCB_EVENT_MASK_FOCUS_CHANGE);

	ewmh_supported();
	ewmh_supportingwmcheck();
	ewmh_clientlist();

	return 0;
}

int
ewmh_wipe()
{
	xcb_delete_property(conn, scrn->root, ewmh[_NET_SUPPORTED].atom);
	xcb_delete_property(conn, scrn->root, ewmh[_NET_CLIENT_LIST].atom);
	xcb_delete_property(conn, scrn->root, ewmh[_NET_CLIENT_LIST_STACKING].atom);
	xcb_delete_property(conn, scrn->root, ewmh[_NET_ACTIVE_WINDOW].atom);
	xcb_delete_property(conn, scrn->root, ewmh[_NET_SUPPORTING_WM_CHECK].atom);
	xcb_destroy_window(conn, ewmhwid);

	xcb_flush(conn);

	return 0;
}


int
ewmh_supported()
{
	uint32_t i;
	xcb_atom_t supported[LEN(ewmh)];

	for (i = 0; i < LEN(ewmh); i++)
		supported[i] = ewmh[i].atom;

	return wm_set_atom(scrn->root, ewmh[_NET_SUPPORTED].atom, XCB_ATOM_ATOM, i, &supported);
}

int
ewmh_supportingwmcheck()
{
	int val = 1;

	ewmhwid = xcb_generate_id(conn);

	/* dummyest window ever. */
	xcb_create_window(conn,
		XCB_COPY_FROM_PARENT, ewmhwid, scrn->root,
		0, 0, 1, 1, 0,                     /* x, y, w, h, border */
		XCB_WINDOW_CLASS_INPUT_ONLY,       /* no need for output */
		scrn->root_visual,                 /* visual */
		XCB_CW_OVERRIDE_REDIRECT, &val);   /* have the WM ignore us */

	wm_set_atom(scrn->root, ewmh[_NET_SUPPORTING_WM_CHECK].atom, XCB_ATOM_WINDOW, 1, &ewmhwid);
	wm_set_atom(ewmhwid, ewmh[_NET_SUPPORTING_WM_CHECK].atom, XCB_ATOM_WINDOW, 1, &ewmhwid);

	return 0;
}

int
ewmh_activewindow(xcb_window_t wid)
{
	wm_set_atom(scrn->root, ewmh[_NET_ACTIVE_WINDOW].atom, XCB_ATOM_WINDOW, 1, &wid);
	return 0;
}

int
ewmh_clientlist()
{
	uint32_t i, c, n;
	xcb_window_t *w, *l;

	n = wm_get_windows(scrn->root, &w);

	l = calloc(n, sizeof(*w));

	for (i=0, c=0; i<n; i++) {
		if (ewmh_type(w[i]) != NORMAL)
			xcb_change_window_attributes(conn, w[i], XCB_CW_OVERRIDE_REDIRECT, &(int){1});

		if (wm_is_listable(w[i], 0))
			l[c++] = w[i];
	}

	free(w);

	wm_set_atom(scrn->root, ewmh[_NET_CLIENT_LIST].atom, XCB_ATOM_WINDOW, c, l);
	wm_set_atom(scrn->root, ewmh[_NET_CLIENT_LIST_STACKING].atom, XCB_ATOM_WINDOW, c, l);

	free(l);

	return 0;
}

int
ewmh_type(xcb_window_t window)
{
	unsigned long n;
	int type = NORMAL; /* treat non-ewmh as normal windows */
	xcb_atom_t *atoms;

	atoms = wm_get_atom(window, ewmh[_NET_WM_WINDOW_TYPE].atom, XCB_ATOM_ATOM, &n);

	if (!atoms)
		return NORMAL;

	/*
	 * as per the EWMH spec, when multiple types are
	 * applicable, they must be listed from the most to least
	 * important.
	 * To do so, we cycle through them in reverse order, changing
	 * the window type everytime a known type is encountered.
	 * Some toolkits like to use toolkit-specific atoms as their
	 * first value for more appropriate categorization. This function
	 * only deals with standard EWMH atoms.
	 */
	while (n --> 0) {
		if (atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_DESKTOP].atom
		 || atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_DOCK].atom
		 || atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_TOOLBAR].atom
		 || atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_POPUP_MENU].atom
		 || atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_COMBO].atom
		 || atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_DND].atom)
			type = IGNORE;

		if (atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_MENU].atom
		 || atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_SPLASH].atom
		 || atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_DROPDOWN_MENU].atom
		 || atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_TOOLTIP].atom
		 || atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_NOTIFICATION].atom)
			type = POPUP;

		if (atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_DIALOG].atom
		 || atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_UTILITY].atom
		 || atoms[n] == ewmh[_NET_WM_WINDOW_TYPE_NORMAL].atom)
			type = NORMAL;
	}

	return type;
}


int
ewmh_message(xcb_client_message_event_t *ev)
{
	/* ignore all other messages */
	if (ev->type != ewmh[_NET_WM_STATE].atom)
		return -1;

	if (ev->data.data32[1] == ewmh[_NET_WM_STATE_FULLSCREEN].atom
	 || ev->data.data32[2] == ewmh[_NET_WM_STATE_FULLSCREEN].atom) {
		ewmh_fullscreen(ev->window, ev->data.data32[0]);
		return 0;
	}

	return 1;
}

int
ewmh_fullscreen(xcb_window_t wid, int state)
{
	size_t n;
	int isfullscreen;
	xcb_atom_t *atom, original_size;
	xcb_randr_monitor_info_t *m;
	struct xgeom g, *origin;

	atom = wm_get_atom(wid, ewmh[_NET_WM_STATE].atom, XCB_ATOM_ATOM, &n);
	original_size = wm_add_atom("ORIGINAL_SIZE", strlen("ORIGINAL_SIZE"));

	isfullscreen = (atom && *atom == ewmh[_NET_WM_STATE_FULLSCREEN].atom);

	switch (state) {
	case -1:
		return isfullscreen;
		break; /* NOTREACHED */

	case 0: /* _NET_WM_STATE_REMOVE */
		wm_set_atom(wid, ewmh[_NET_WM_STATE].atom, XCB_ATOM_ATOM, 0, NULL);
		origin = wm_get_atom(wid, original_size, XCB_ATOM_CARDINAL, &n);
		if (!origin || n < 5)
			return -1;

		wm_set_border(origin->b, -1, wid);
		wm_teleport(wid, origin->x, origin->y, origin->w, origin->h);
		xcb_delete_property(conn, wid, original_size);
		break;

	case 1: /* _NET_WM_STATE_ADD */
		/* save current window geometry */
		g.x = wm_get_attribute(wid, ATTR_X);
		g.y = wm_get_attribute(wid, ATTR_Y);
		g.w = wm_get_attribute(wid, ATTR_W);
		g.h = wm_get_attribute(wid, ATTR_H);
		g.b = wm_get_attribute(wid, ATTR_B);
		wm_set_atom(wid, original_size, XCB_ATOM_CARDINAL, 5, &g);

		m = wm_get_monitor(wm_find_monitor(g.x, g.y));
		if (!m)
			return -1;

		/* move window fullscreen */
		wm_set_border(0, -1, wid);
		wm_teleport(wid, m->x, m->y, m->width, m->height);
		wm_set_atom(wid, ewmh[_NET_WM_STATE].atom, XCB_ATOM_ATOM, 1, &ewmh[_NET_WM_STATE_FULLSCREEN].atom);
		free(m);
		break;

	case 2: /* _NET_WM_STATE_TOGGLE */
		printf("0x%08x !fullscreen\n", wid);
		ewmh_fullscreen(wid, !isfullscreen);
		break;
	}

	return 0;
}


void win_move(const Arg arg) {
    int  r = arg.com[0][0] == 'r';
    char m = arg.com[1][0];

    win_size(cur->w, &wx, &wy, &ww, &wh);

    XMoveResizeWindow(d, cur->w, \
        wx + (r ? 0 : m == 'e' ?  arg.i : m == 'w' ? -arg.i : 0),
        wy + (r ? 0 : m == 'n' ? -arg.i : m == 's' ?  arg.i : 0),
        MAX(10, ww + (r ? m == 'e' ?  arg.i : m == 'w' ? -arg.i : 0 : 0)),
        MAX(10, wh + (r ? m == 'n' ? -arg.i : m == 's' ?  arg.i : 0 : 0)));

    XWarpPointer(d, None, cur->w, 0, 0, 0, 0, ww/2, wh/2);
}

void win_move_mouse(const Arg arg) {
	win_size(mouse.subwindow, &wx, &wy, &ww, &wh);
	drag = MOVING;
}

void win_half(const Arg arg) {
     char m = arg.com[0][0];

     win_size(cur->w, &wx, &wy, &ww, &wh);

     XMoveResizeWindow(d, cur->w, \
        (m == 'w' ? wx : m == 'e' ? (wx + ww / 2) : wx),
        (m == 'n' ? wy : m == 's' ? (wy + wh / 2) : wy),
        (m == 'w' ? (ww / 2) : m == 'e' ? (ww / 2) : ww),
        (m == 'n' ? (wh / 2) : m == 's' ? (wh / 2) : wh));
}

unsigned long getcolor(const char *col) {
    Colormap m = DefaultColormap(d, s);
    XColor c;
    return (!XAllocNamedColor(d, m, col, &c, &c)) ? 0 : c.pixel | (0xffu << 24);
}

void win_border(Window w, const char *col) {
	if ((cur->f = cur->f ? 1 : 0)) {
		return;
	} else {
		XSetWindowBorder(d, w, getcolor(col));
		XConfigureWindow(d, cur->w, CWBorderWidth,
				&(XWindowChanges){.border_width = BORDER_WIDTH});
	}
}

void win_active(Window w) {
    for win if (c->w == w) {
	    win_border(w, BORDER_COLOR_ACTIVE);
    } else {
	    win_border(c->w, BORDER_COLOR_INACTIVE);
    }
}

void win_focus(client *c) {
    cur = c;
    XSetInputFocus(d, cur->w, RevertToParent, CurrentTime);
    win_active(cur->w);
}

void mapping_notify(XEvent *e) {
    XMappingEvent *ev = &e->xmapping;

    if (ev->request == MappingKeyboard || ev->request == MappingModifier) {
        XRefreshKeyboardMapping(ev);
        input_grab(root);
    }
}

void notify_destroy(XEvent *e) {
    win_del(e->xdestroywindow.window);

    if (list) win_focus(list->prev);
}

void notify_enter(XEvent *e) {
    while(XCheckTypedEvent(d, EnterNotify, e));

    for win if (c->w == e->xcrossing.window) win_focus(c);
}

void notify_motion(XEvent *e) {
    if (!mouse.subwindow || !drag || cur->f) return;
    if (cur->f == 1) return;

    while(XCheckTypedEvent(d, MotionNotify, e));
    while(XCheckTypedWindowEvent(d, mouse.subwindow, MotionNotify, e));

    int xd = e->xbutton.x_root - mouse.x_root;
    int yd = e->xbutton.y_root - mouse.y_root;

    XMoveResizeWindow(d, mouse.subwindow,
		    wx + (drag == MOVING ? xd : 0),
		    wy + (drag == MOVING ? yd : 0),
		    MAX(1, ww + (drag == SIZING ? xd : 0)),
		    MAX(1, wh + (drag == SIZING ? yd : 0)));

    win_size(cur->w, &cur->wx, &cur->wx, &cur->ww, &cur->wh);
}

void key_press(XEvent *e) {
    KeySym keysym = XkbKeycodeToKeysym(d, e->xkey.keycode, 0, 0);

    for (unsigned int i=0; i < sizeof(keys)/sizeof(*keys); ++i)
        if (keys[i].keysym == keysym &&
            mod_clean(keys[i].mod) == mod_clean(e->xkey.state))
            keys[i].function(keys[i].arg);

}

void win_resize(const Arg arg) {
    win_size(mouse.subwindow, &wx, &wy, &ww, &wh);
    drag = SIZING;
}

void button_press(XEvent *e) {
    if (!e->xbutton.subwindow) return;
    unsigned mod = mod_clean(e->xbutton.state);

    mouse = e->xbutton;
    drag = 0;
    for (unsigned int i = 0; i < sizeof(buttons)/sizeof(*buttons); ++i)
        if (buttons[i].button == e->xbutton.button &&
            mod_clean(buttons[i].mod) == mod)
            buttons[i].function(buttons[i].arg);
}

void button_release(XEvent *e) {
    mouse.subwindow = 0;
}

void win_init(void) {
    Window *child;
    unsigned int i, n_child;

    XQueryTree(d, RootWindow(d, DefaultScreen(d)),
               &(Window){0}, &(Window){0}, &child, &n_child);

    for (i = 0;  i < n_child; i++) {
        XSelectInput(d, child[i], StructureNotifyMask|EnterWindowMask);
        XMapWindow(d, child[i]);
        win_add(child[i]);
    }
    int tmp = ws;

    ws_save(ws);
    ws_sel(1);

    for win XMapWindow(d, c->w);

    ws_sel(tmp);

    for win XUnmapWindow(d, c->w);

    ws_sel(1);
}

void win_add(Window w) {
    client *c;

    if (!(c = (client *) calloc(1, sizeof(client))))
        exit(1);

    c->w = w;

    if (list) {
        list->prev->next = c;
        c->prev          = list->prev;
        list->prev       = c;
        c->next          = list;

    } else {
        list = c;
        list->prev = list->next = list;
    }

    ws_save(ws);
}

void win_del(Window w) {
    client *x = 0;

    for win if (c->w == w) x = c;

    if (!list || !x)  return;
    if (x->prev == x) list = 0;
    if (list == x)    list = x->next;
    if (x->next)      x->next->prev = x->prev;
    if (x->prev)      x->prev->next = x->next;

    free(x);
    ws_save(ws);
}

void win_kill(const Arg arg) {
    if (!cur) return;
    
    XEvent ev = { .type = ClientMessage };
    
    ev.xclient.window       = cur->w;
    ev.xclient.format       = 32;
    ev.xclient.message_type = XInternAtom(d, "WM_PROTOCOLS", True);
    ev.xclient.data.l[0]    = XInternAtom(d, "WM_DELETE_WINDOW", True);
    ev.xclient.data.l[1]    = CurrentTime;
    
    XSendEvent(d, cur->w, False, NoEventMask, &ev);
}

int multimonitor_center_fs (int fs) {
    if (!XineramaIsActive(d)) return 1;
    XineramaScreenInfo *screen_info = XineramaQueryScreens(d, &monitors);
    for (int i = 0; i < monitors; i++) {
        if ((cur->wx >= screen_info[i].x_org && cur->wx < screen_info[i].x_org + screen_info[i].width)
            && (cur->wy >= screen_info[i].y_org && cur->wy < screen_info[i].y_org + screen_info[i].height)) {
            if (fs) {
                XMoveResizeWindow(d, cur->w,
                                  screen_info[i].x_org, screen_info[i].y_org,
                                  screen_info[i].width, screen_info[i].height);
                XConfigureWindow(d, cur->w, CWBorderWidth,
                                  &(XWindowChanges){.border_width = 0}); 
            } else
                XMoveWindow(d, cur->w,
                        screen_info[i].x_org + ((screen_info[i].width  - ww) / 2),
                        screen_info[i].y_org + ((screen_info[i].height - wh) / 2));
            break;
        }
    }
    return 0;
}

void win_center(const Arg arg, bool m) {
    if (!cur) return;

    win_size(cur->w, &cur->wx, &cur->wy, &cur->ww, &cur->wh);

    if (multimonitor_center_fs(0))
        XMoveWindow(d, cur->w, (sw - ww) / 2, (sh - wh) / 2);

    win_size(cur->w, &cur->wx, &cur->wy, &cur->ww, &cur->wh);

    if(m) {
        XWarpPointer(d, None, cur->w, 0, 0, 0, 0, ww/2, wh/2);
    }
}

void win_up(const Arg arg) {
    wy -= 50;
    win_size(cur->w, &(int){0}, &(int){0}, &ww, &wh);
    XMoveWindow(d, cur->w, wx, wy);
}

void win_down(const Arg arg) {
    wy += 50;
    win_size(cur->w, &(int){0}, &(int){0}, &ww, &wh);
    XMoveWindow(d, cur->w, wx, wy);
}

void win_right(const Arg arg) {
    wx += 50;
    win_size(cur->w, &(int){0}, &(int){0}, &ww, &wh);
    XMoveWindow(d, cur->w, wx, wy);
}

void win_left(const Arg arg) {
    wx -= 50;
    win_size(cur->w, &(int){0}, &(int){0}, &ww, &wh);
    XMoveWindow(d, cur->w, wx, wy);
}

void win_lower(const Arg arg) {
    if (!cur) return;

    XLowerWindow(d, cur->w);
}

void win_raise(const Arg arg) {
    if (!cur) return;

    XRaiseWindow(d, cur->w);
}

void win_fs(const Arg arg) {
    if (!cur) return;

    if ((cur->f = cur->f ? 0 : 1)) {
        win_size(cur->w, &cur->wx, &cur->wy, &cur->ww, &cur->wh);
        if(multimonitor_center_fs(1)) {
            XMoveResizeWindow(d, cur->w, 0, 0, sw, sh);
            XConfigureWindow(d, cur->w, CWBorderWidth,
                    &(XWindowChanges){.border_width = 0});
        }
    } else {
        XMoveResizeWindow(d, cur->w, cur->wx, cur->wy, cur->ww, cur->wh);
        XConfigureWindow(d, cur->w, CWBorderWidth,
                &(XWindowChanges){.border_width = BORDER_WIDTH});
    }
}

void win_to_ws(const Arg arg) {
    int tmp = ws;

    if (arg.i == tmp) return;

    ws_sel(arg.i);
    win_add(cur->w);
    ws_save(arg.i);

    ws_sel(tmp);
    win_del(cur->w);
    XUnmapWindow(d, cur->w);
    ws_save(tmp);

    if (list) win_focus(list);
}

void win_prev(const Arg arg) {
    if (!cur) return;
    if (list == 0) return;

    XRaiseWindow(d, cur->prev->w);
    win_focus(cur->prev);
    win_size(cur->w, &wx, &wy, &ww, &wh);
    XWarpPointer(d, None, cur->w, 0, 0, 0, 0, ww/2, wh/2);
}

void win_next(const Arg arg) {
    if (!cur) return;
    if (list == 0) return;

    XRaiseWindow(d, cur->next->w);
    win_focus(cur->next);
    win_size(cur->w, &wx, &wy, &ww, &wh);
    XWarpPointer(d, None, cur->w, 0, 0, 0, 0, ww/2, wh/2);
}

void ws_go(const Arg arg) {
    int tmp = ws;

    if (arg.i == ws) return;

    ws_save(ws);
    ws_sel(arg.i);

    for win XMapWindow(d, c->w);

    ws_sel(tmp);

    for win XUnmapWindow(d, c->w);

    ws_sel(arg.i);

    if (list) win_focus(list); else cur = 0;

		ewmh_clientlist();

		/*wm_set_atom(scrn->root, ewmh[_NET_CURRENT_DESKTOP].atom, XCB_ATOM_WINDOW, ws);*/
		/*xcb_ewmh_set_current_desktop(ewmh[_NET_CURRENT_DESKTOP], 0, ws);*/
}

void ws_next(const Arg arg) {
	int tmp = ws;
	int nws = tmp + arg.i;

	if (ws == NUM_WS-1) return;

	ws_save(ws);
	ws_sel(nws);
	
	for win XMapWindow(d, c->w);
	
	ws_sel(tmp);
	
	for win XUnmapWindow(d, c->w);

	ws_sel(nws);

	if (list) win_focus(list); else cur = 0;

	ewmh_clientlist();
  /*xcb_ewmh_set_current_desktop(ewmh, 0, ws);*/
}

void ws_prev(const Arg arg) {
	int tmp = ws;
	int nws = tmp - arg.i;

	if (ws == 1) return;

	ws_save(ws);
	ws_sel(nws);
	
	for win XMapWindow(d, c->w);
	
	ws_sel(tmp);
	
	for win XUnmapWindow(d, c->w);

	ws_sel(nws);

	if (list) win_focus(list); else cur = 0;

	ewmh_clientlist();
  /*xcb_ewmh_set_current_desktop(ewmh, 0, ws);*/
}

void configure_request(XEvent *e) {
    XConfigureRequestEvent *ev = &e->xconfigurerequest;

    XConfigureWindow(d, ev->window, ev->value_mask, &(XWindowChanges) {
        .x          = ev->x,
        .y          = ev->y,
        .width      = ev->width,
        .height     = ev->height,
        .sibling    = ev->above,
        .stack_mode = ev->detail
    });
}

bool exists_win(Window w) {
    int tmp = ws;
    for (int i = 0; i < NUM_WS; ++i) {
        if (i == tmp) continue;
        ws_sel(i);
        for win if (c->w == w) {
            ws_sel(tmp);
            return true;
        }
    }
    ws_sel(tmp);
    return false;
}

void map_request(XEvent *e) {
    Window w = e->xmaprequest.window;
    if (exists_win(w)) return;

    XSelectInput(d, w, StructureNotifyMask|EnterWindowMask);
    win_size(w, &wx, &wy, &ww, &wh);
    win_add(w);
    cur = list->prev;

    if (wx + wy == 0) win_center((Arg){0},false); 

    XMapWindow(d, w);
    win_focus(list->prev);
		ewmh_clientlist();
}

void run(const Arg arg) {
    if (fork()) return;
    if (d) close(ConnectionNumber(d));

    setsid();
    execvp((char*)arg.com[0], (char**)arg.com);
}

void input_grab(Window root) {
    unsigned int i, j, modifiers[] = {0, LockMask, numlock, numlock|LockMask};
    XModifierKeymap *modmap = XGetModifierMapping(d);
    KeyCode code;

    for (i = 0; i < 8; i++)
        for (int k = 0; k < modmap->max_keypermod; k++)
            if (modmap->modifiermap[i * modmap->max_keypermod + k]
                == XKeysymToKeycode(d, 0xff7f))
                numlock = (1 << i);

    XUngrabKey(d, AnyKey, AnyModifier, root);

    for (i = 0; i < sizeof(keys)/sizeof(*keys); i++)
        if ((code = XKeysymToKeycode(d, keys[i].keysym)))
            for (j = 0; j < sizeof(modifiers)/sizeof(*modifiers); j++)
                XGrabKey(d, code, keys[i].mod | modifiers[j], root,
                        True, GrabModeAsync, GrabModeAsync);

    for (i = 0; i < sizeof(buttons)/sizeof(*buttons); i++)
        for (size_t j = 0; j < sizeof(modifiers)/sizeof(*modifiers); j++)
            XGrabButton(d, buttons[i].button, buttons[i].mod | modifiers[j], root, True,
                ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
                GrabModeAsync, GrabModeAsync, 0, 0);

    XFreeModifiermap(modmap);
}

int main(void) {
    XEvent ev;
		/*int mask;*/

    if (!(d = XOpenDisplay(0))) exit(1);

    signal(SIGCHLD, SIG_IGN);
    XSetErrorHandler(xerror);

    int s = DefaultScreen(d);
    root  = RootWindow(d, s);
    /*Window root = screen->root;*/
    sw    = XDisplayWidth(d, s);
    sh    = XDisplayHeight(d, s);

    XSelectInput(d,  root, SubstructureRedirectMask);
    XDefineCursor(d, root, XCreateFontCursor(d, 68));
    input_grab(root);

    win_init();

    wm_init_xcb();
    wm_get_screen();
    ewmh_init();
    signal(SIGINT,  cleanup);
    signal(SIGTERM, cleanup);

	  /*mask = XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;*/

    while (1 && !XNextEvent(d, &ev)) // 1 && will forever be here.
        if (events[ev.type]) events[ev.type](&ev);
}
