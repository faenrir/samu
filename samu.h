#include <X11/Xlib.h>

#include "types.h"
#include <xcb/xcb.h>

#define NUM_WS 23
#define win        (client *t=0, *c=list; c && t!=list->prev; t=c, c=c->next)
#define ws_save(W) ws_list[W] = list
#define ws_sel(W)  list = ws_list[ws = W]
#define MAX(a, b)  ((a) > (b) ? (a) : (b))

#define win_size(W, gx, gy, gw, gh) \
    XGetGeometry(d, W, &(Window){0}, gx, gy, gw, gh, \
                 &(unsigned int){0}, &(unsigned int){0})

// Taken from DWM. Many thanks. https://git.suckless.org/dwm
#define mod_clean(mask) (mask & ~(numlock|LockMask) & \
        (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))

typedef struct {
    const char** com;
    const int i;
    const Window w;
} Arg;

struct key {
    unsigned int mod;
    KeySym keysym;
    void (*function)(const Arg arg);
    const Arg arg;
};

struct button {
	unsigned int mod;
	unsigned int button;
	void (*function)(const Arg arg);
	const Arg arg;
};

typedef struct client {
    struct client *next, *prev;
    int f, wx, wy;
    unsigned int ww, wh;
    Window w;
} client;

void button_press(XEvent *e);
void button_release(XEvent *e);
void configure_request(XEvent *e);
void input_grab(Window root);
void key_press(XEvent *e);
void map_request(XEvent *e);
void mapping_notify(XEvent *e);
void notify_destroy(XEvent *e);
void notify_enter(XEvent *e);
void notify_motion(XEvent *e);
void run(const Arg arg);
void win_init(void);
void win_add(Window w);
void win_center(const Arg arg, bool m);
void win_up(const Arg arg);
void win_down(const Arg arg);
void win_right(const Arg arg);
void win_left(const Arg arg);
void win_del(Window w);
void win_fs(const Arg arg);
void win_focus(client *c);
void win_kill(const Arg arg);
void win_lower(const Arg arg);
void win_raise(const Arg arg);
void win_move(const Arg arg);
void win_move_mouse(const Arg arg);
void win_tiler(const Arg arg);
void win_resize(const Arg arg);
void win_prev(const Arg arg);
void win_next(const Arg arg);
void win_half(const Arg arg);
void win_to_ws(const Arg arg);
void win_border(Window w, const char *col);
void win_active(Window w);
void ws_go(const Arg arg);
void ws_next(const Arg arg);
void ws_prev(const Arg arg);
unsigned long getcolor(const char *col);
bool exists_win(Window w);

static void ewmh_init(void);
xcb_ewmh_connection_t         *ewmh;
static xcb_connection_t       *con;
static const xcb_setup_t      *setup;
static xcb_screen_iterator_t  it;
static xcb_screen_t           *screen;

#define EPRINT(...) fprintf(stderr, __VA_ARGS__);


static int xerror() { return 0; }
