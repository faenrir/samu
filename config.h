#ifndef CONFIG_H
#define CONFIG_H

#define MOD Mod4Mask

#define BORDER_COLOR "#706e86"
#define BORDER_COLOR_ACTIVE "#9ccfd8"
#define BORDER_COLOR_INACTIVE "#6e6a86"
//#define BORDER_COLOR_ACTIVE "#3f51b5"
//#define BORDER_COLOR_INACTIVE "#080808"
#define BORDER_WIDTH 0

static struct key keys[] = {
    {MOD,      XK_q,   win_kill,   {0}},
    {MOD,      XK_c,   win_center, {0}},
    {MOD,      XK_f,   win_fs,     {0}},

    //{MOD,  XK_k,  win_half,  {.com = (const char*[]){"n"}}},
    //{MOD,  XK_j,  win_half,  {.com = (const char*[]){"s"}}},
    //{MOD,  XK_l,  win_half,  {.com = (const char*[]){"e"}}},
    //{MOD,  XK_h,  win_half,  {.com = (const char*[]){"w"}}},

    {MOD,           XK_Tab, win_next,   {0}},
    {MOD|ShiftMask, XK_Tab, win_prev,   {0}},
    {Mod1Mask,           XK_Tab, win_next,   {0}},
    {Mod1Mask|ShiftMask, XK_Tab, win_prev,   {0}},

    {MOD, XK_o, win_raise, {0}}, 
    {MOD, XK_p, win_raise, {0}}, 
    {MOD, XK_c, win_raise, {0}}, 
    
    {MOD, XK_a, win_raise, {0}}, 
    {MOD, XK_s, win_raise, {0}}, 
    {MOD, XK_z, win_raise, {0}}, 
    {MOD, XK_y, win_raise, {0}}, 
    {MOD, XK_x, win_raise, {0}}, 

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
     
     {MOD|Mod1Mask, Button1, win_lower, {0}},
     {Mod1Mask, Button1, win_lower, {0}},

     {MOD|ShiftMask, Button3, win_raise, {0}},
     {MOD|ShiftMask, Button3, win_fs, {0}},
     
     {MOD|ShiftMask, Button2, win_kill, {0}},
};

#endif
