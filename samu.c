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

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb_ewmh.h>

#include "samu.h"
#include "config.h"

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

#include "config.h"

void
ewmh_init(void)
{
	// thanks 2bwm :P
	if (!(ewmh = calloc(1, sizeof(xcb_ewmh_connection_t)))) {
		EPRINT("samu: error: failed to calloc() for EWMH:");
		perror("calloc()");
		return;
	}

	xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(con, ewmh);
	if (!xcb_ewmh_init_atoms_replies(ewmh, cookie, (void*) 0)) {
		EPRINT("samu: error: failed to initialize EWMH.\n");
		exit(1);
	}
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

    xcb_ewmh_set_current_desktop(ewmh, 0, ws);
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

  xcb_ewmh_set_current_desktop(ewmh, 0, ws);
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

  xcb_ewmh_set_current_desktop(ewmh, 0, ws);
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
}

void run(const Arg arg) {
    if (fork()) return;
    if (d) close(ConnectionNumber(d));

    setsid();
    execvp((char*)arg.com[0], (char**)arg.com);
}

void autostart(void) {
    system("cd ~/.config/samu; ./autostart.sh &");
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
    autostart();

    win_init();

    // setup EWMH garbage
    con         = XGetXCBConnection(d);
	  ewmh_init();
	  xcb_ewmh_set_wm_pid(ewmh, root, getpid());
	  xcb_ewmh_set_wm_name(ewmh, root, 7, "samu");
	  xcb_atom_t net_atoms[] = {
		  ewmh->_NET_WM_PID,
		  ewmh->_NET_WM_NAME,
		  ewmh->_NET_CURRENT_DESKTOP,
		  ewmh->_NET_NUMBER_OF_DESKTOPS
	  };

	  xcb_ewmh_set_supported(ewmh, 0,
			  4, 	// length of net_atoms
		  	net_atoms);
	  xcb_ewmh_set_current_desktop(ewmh, 0, ws);
	  xcb_ewmh_set_number_of_desktops(ewmh, 0, NUM_WS);

    while (1 && !XNextEvent(d, &ev)) // 1 && will forever be here.
        if (events[ev.type]) events[ev.type](&ev);
}
