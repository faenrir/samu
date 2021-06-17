#ifndef CONFIG_H
#define CONFIG_H

#define MOD Mod4Mask

#define BORDER_COLOR_ACTIVE "#56949f"
#define BORDER_COLOR_INACTIVE "#6e6a86"
#define BORDER_WIDTH 2

const char* snapLeft[]    = {"snap.sh", "left", 0};
const char* snapRight[]    = {"snap.sh", "right", 0};
const char* snapCenter[]    = {"snap.sh", "center", 0};

const char* winTl[]    = {"snap.sh", "tl", 0};
const char* winTr[]    = {"snap.sh", "tr", 0};
const char* winBl[]    = {"snap.sh", "bl", 0};
const char* winBr[]    = {"snap.sh", "br", 0};

const char* winCenter[]    = {"move", "-c", 0};

static struct key keys[] = {
    {MOD,      XK_q,   win_kill,   {0}},
    {MOD,      XK_c,   win_raise, {0}},
    {MOD,      XK_c,   run, {.com = winCenter}},
    {MOD,      XK_f,   win_fs,     {0}},
    {MOD,      XK_f,   win_raise,     {0}},

    //{MOD,  XK_k,  win_half,  {.com = (const char*[]){"n"}}},
    //{MOD,  XK_j,  win_half,  {.com = (const char*[]){"s"}}},
    //{MOD,  XK_l,  win_half,  {.com = (const char*[]){"e"}}},
    //{MOD,  XK_h,  win_half,  {.com = (const char*[]){"w"}}},

    {MOD, XK_Up, win_move, {.com = (const char*[]){"move",   "n"}, .i = 50}},
    {MOD, XK_Up, win_raise, {0}},
    {MOD, XK_Down, win_move, {.com = (const char*[]){"move",   "s"}, .i = 50}},
    {MOD, XK_Down, win_raise, {0}},
    {MOD, XK_Right, win_move, {.com = (const char*[]){"move",   "e"}, .i = 50}},
    {MOD, XK_Right, win_raise, {0}},
    {MOD, XK_Left, win_move, {.com = (const char*[]){"move",   "w"}, .i = 50}},
    {MOD, XK_Left, win_raise, {0}},

    {MOD|ShiftMask, XK_Up, win_move, {.com = (const char*[]){"resize",   "n"}, .i = 50}},
    {MOD|ShiftMask, XK_Up, win_raise, {0}},
    {MOD|ShiftMask, XK_Down, win_move, {.com = (const char*[]){"resize",   "s"}, .i = 50}},
    {MOD|ShiftMask, XK_Down, win_raise, {0}},
    {MOD|ShiftMask, XK_Right, win_move, {.com = (const char*[]){"resize",   "e"}, .i = 50}},
    {MOD|ShiftMask, XK_Right, win_raise, {0}},
    {MOD|ShiftMask, XK_Left, win_move, {.com = (const char*[]){"resize",   "w"}, .i = 50}},
    {MOD|ShiftMask, XK_Left, win_raise, {0}},

    {MOD,           XK_Tab, win_next,   {0}},
    {MOD|ShiftMask, XK_Tab, win_prev,   {0}},
    {Mod1Mask,           XK_Tab, win_next,   {0}},
    {Mod1Mask|ShiftMask, XK_Tab, win_prev,   {0}},

    {MOD, XK_o, run, {.com = snapLeft}},
    {MOD, XK_o, win_raise, {0}},
    {MOD, XK_p, run, {.com = snapRight}},
    {MOD, XK_p, win_raise, {0}},
    {MOD, XK_m, run, {.com = snapCenter}},
    {MOD, XK_m, win_raise, {0}},

    {MOD, XK_a, run, {.com = winTl}},
    {MOD, XK_a, win_raise, {0}},
    {MOD, XK_s, run, {.com = winTr}},
    {MOD, XK_s, win_raise, {0}},
    {MOD, XK_z, run, {.com = winBl}},
    {MOD, XK_z, win_raise, {0}},
    {MOD, XK_x, run, {.com = winBr}},
    {MOD, XK_x, win_raise, {0}},
    {MOD, XK_y, run, {.com = winBr}},
    {MOD, XK_y, win_raise, {0}},
    
    {MOD,           XK_1, ws_go,     {.i = 1}},
    {MOD|ShiftMask, XK_1, win_to_ws, {.i = 1}},
    {MOD,           XK_2, ws_go,     {.i = 2}},
    {MOD|ShiftMask, XK_2, win_to_ws, {.i = 2}},
    {MOD,           XK_3, ws_go,     {.i = 3}},
    {MOD|ShiftMask, XK_3, win_to_ws, {.i = 3}},
    {MOD,           XK_4, ws_go,     {.i = 4}},
    {MOD|ShiftMask, XK_4, win_to_ws, {.i = 4}},
    {MOD,           XK_5, ws_go,     {.i = 5}},
    {MOD|ShiftMask, XK_5, win_to_ws, {.i = 5}},
    {MOD,           XK_6, ws_go,     {.i = 6}},
    {MOD|ShiftMask, XK_6, win_to_ws, {.i = 6}},
    {MOD,           XK_7, ws_go,     {.i = 7}},
    {MOD|ShiftMask, XK_7, win_to_ws, {.i = 7}},
    {MOD,           XK_8, ws_go,     {.i = 8}},
    {MOD|ShiftMask, XK_8, win_to_ws, {.i = 8}},
    {MOD,           XK_9, ws_go,     {.i = 9}},
    {MOD|ShiftMask, XK_9, win_to_ws, {.i = 9}},
    {MOD,           XK_0, ws_go,     {.i = 10}},
    {MOD|ShiftMask, XK_0, win_to_ws, {.i = 10}},

    {MOD,           XK_1, ws_go,     {.i = 11}},
    {MOD|ShiftMask, XK_1, win_to_ws, {.i = 11}},
    {MOD,           XK_2, ws_go,     {.i = 12}},
    {MOD|ShiftMask, XK_2, win_to_ws, {.i = 12}},
    {MOD,           XK_3, ws_go,     {.i = 13}},
    {MOD|ShiftMask, XK_3, win_to_ws, {.i = 13}},
    {MOD,           XK_4, ws_go,     {.i = 14}},
    {MOD|ShiftMask, XK_4, win_to_ws, {.i = 14}},
    {MOD,           XK_5, ws_go,     {.i = 15}},
    {MOD|ShiftMask, XK_5, win_to_ws, {.i = 15}},
    {MOD,           XK_6, ws_go,     {.i = 16}},
    {MOD|ShiftMask, XK_6, win_to_ws, {.i = 16}},
    {MOD,           XK_7, ws_go,     {.i = 17}},
    {MOD|ShiftMask, XK_7, win_to_ws, {.i = 17}},
    {MOD,           XK_8, ws_go,     {.i = 18}},
    {MOD|ShiftMask, XK_8, win_to_ws, {.i = 18}},
    {MOD,           XK_9, ws_go,     {.i = 19}},
    {MOD|ShiftMask, XK_9, win_to_ws, {.i = 19}},
    {MOD,           XK_0, ws_go,     {.i = 20}},
    {MOD|ShiftMask, XK_0, win_to_ws, {.i = 20}},

    {0, XF86XK_Forward, ws_next, {.i = 1}},
    {0, XF86XK_Back, ws_prev, {.i = 1}},
    {Mod1Mask|ShiftMask, XK_Right, ws_next, {.i = 1}},
    {Mod1Mask|ShiftMask, XK_Left, ws_prev, {.i = 1}},
};

static struct button buttons[] = {
     {MOD,           Button1, win_raise, {0}},
     {MOD,           Button1, win_move_mouse, {0}},

     {MOD|ShiftMask, Button1, win_raise, {0}}, 
     {MOD|ShiftMask, Button1, win_resize, {0}},
     
     {MOD, Button3, win_raise, {0}}, 
     {MOD, Button3, win_resize, {0}},
     
     {MOD, Button2, win_lower, {0}},
     {Mod1Mask, Button1, win_lower, {0}},
     {MOD|ShiftMask, Button3, win_lower, {0}},
     
};

#endif
