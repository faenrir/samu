#ifndef CONFIG_H
#define CONFIG_H

#define MOD Mod4Mask

#define BORDER_COLOR "#706e86"
#define BORDER_COLOR_ACTIVE "#9ccfd8"
#define BORDER_COLOR_INACTIVE "#6e6a86"
//#define BORDER_COLOR_ACTIVE "#3f51b5"
//#define BORDER_COLOR_INACTIVE "#080808"
#define BORDER_WIDTH 2

// Shell functions
const char* menu[]    = {"rofi-launcher",      0};
const char* altMenu[]    = {"bash", "/home/fabian/.config/rofi/rofiMenu.sh",      0};
const char* winMenu[]    = {"bash", "/home/fabian/.config/rofi/rofiMenuWindows.sh",      0};
const char* print[]    = {"rofiMenuPrint.sh",      0};
const char* screenshot[]    = {"screenshotFull",      0};
const char* dictionary[]    = {"word-lookup",      0};
const char* power[]    = {"rofiMenuPower.sh",      0};
const char* files[]    = {"kitty", "bash", "ranger",      0};
const char* term[]    = {"kitty",             0};
const char* term2[]    = {"st",             0};
const char* tile[]    = {"tile.sh",             0};
const char* scrot[]   = {"scr",            0};
const char* briup[]   = {"brillo-up", 0};
const char* bridown[] = {"brillo-down", 0};
const char* voldown[] = {"pulse-decrease",         0};
const char* volup[]   = {"pulse-increase",         0};
const char* volmute[] = {"pulse-mute",      0};
const char* cmus[] = {"cmus-remote", "-u",      0};
const char* picom[]    = {"picomSwitch",             0};
const char* picomTrans[]    = {"picomTrans.sh",             0};
const char* bar[]    = {"barSwitch.sh",             0};
const char* redshift[]    = {"rs",             0};
const char* arandr[]    = {"arandr",             0};
const char* clock[]    = {"xclockcat", "-tiecolor", "mediumpurple",             0};
const char* i3lock[]    = {"i3lock", "-B", "~/Pictures/joleyn.jpg",             0};
//const char* findCursor[]    = {"find-cursor", "-g", "-c", "'#f1fa8c'", "-l", "10", "-d", "8", "-s", "80",             0};

// Shell Window Manager Tools
const char* snapLeft[]    = {"snap.sh", "left", 0};
const char* snapRight[]    = {"snap.sh", "right", 0};
const char* snapCenter[]    = {"snap.sh", "center", 0};

const char* moveWindows[]    = {"move_windows.sh", 0};
const char* winTeleport[]    = {"teleporter", 0};

const char* winShrink[]    = {"move", "-s", 0};
const char* winGrow[]    = {"move", "-g", 0};

const char* winTl[]    = {"snap.sh", "tl", 0};
const char* winTr[]    = {"snap.sh", "tr", 0};
const char* winBl[]    = {"snap.sh", "bl", 0};
const char* winBr[]    = {"snap.sh", "br", 0};

static struct key keys[] = {
    {MOD,      XK_q,   win_kill,   {0}},
    {MOD,      XK_c,   win_center, {0}},
    {MOD,      XK_f,   win_fs,     {0}},

    {Mod1Mask,           XK_Tab, win_next,   {0}},
    {Mod1Mask|ShiftMask, XK_Tab, win_prev,   {0}},

    {MOD, XK_space,      run, {.com = menu}},
    {Mod1Mask, XK_space,      run, {.com = altMenu}},
    {0, XK_Print, run, {.com = print}},
    {0, XF86XK_Launch5, run, {.com = print}},
    {MOD|ShiftMask, XK_Print, run, {.com = screenshot}},
    {MOD, XK_Return, run, {.com = term}},
    {MOD|ShiftMask, XK_Return, run, {.com = term2}},
    {MOD, XK_t, run, {.com = tile}},
    {MOD, XK_g, run, {.com = picom}},
    {MOD, XK_d, run, {.com = picomTrans}},
    {MOD|ShiftMask, XK_d, run, {.com = dictionary}},
    {MOD, XK_b, run, {.com = bar}},
    {MOD|ShiftMask, XK_r, run, {.com = redshift}},
    {MOD, XK_r, run, {.com = files}},
    {MOD|ShiftMask, XK_c, run, {.com = clock}},
    {MOD|ShiftMask,  XK_l,  run,  {.com = i3lock}},

    {MOD, XK_Up, win_move, {.com = (const char*[]){"move",   "n"}, .i = 50}},
    {MOD, XK_Down, win_move, {.com = (const char*[]){"move",   "s"}, .i = 50}},
    {MOD, XK_Right, win_move, {.com = (const char*[]){"move",   "e"}, .i = 50}},
    {MOD, XK_Left, win_move, {.com = (const char*[]){"move",   "w"}, .i = 50}},

    {MOD|ShiftMask, XK_Up, win_move, {.com = (const char*[]){"resize",   "n"}, .i = 50}},
    {MOD|ShiftMask, XK_Down, win_move, {.com = (const char*[]){"resize",   "s"}, .i = 50}},
    {MOD|ShiftMask, XK_Right, win_move, {.com = (const char*[]){"resize",   "e"}, .i = 50}},
    {MOD|ShiftMask, XK_Left, win_move, {.com = (const char*[]){"resize",   "w"}, .i = 50}},

    {MOD,  XK_k,  win_half,  {.com = (const char*[]){"n"}}},
    {MOD,  XK_j,  win_half,  {.com = (const char*[]){"s"}}},
    {MOD,  XK_l,  win_half,  {.com = (const char*[]){"e"}}},
    {MOD,  XK_h,  win_half,  {.com = (const char*[]){"w"}}},

    {MOD,           XK_Tab, win_next,   {0}},
    {MOD|ShiftMask, XK_Tab, win_prev,   {0}},

    {0,   XF86XK_AudioLowerVolume,  run, {.com = voldown}},
    {0,   XF86XK_AudioRaiseVolume,  run, {.com = volup}},
    {0,   XF86XK_AudioMute,         run, {.com = volmute}},
    {0,   XF86XK_AudioPlay, run, {.com = cmus}},
    {0,   XF86XK_MonBrightnessUp,   run, {.com = briup}},
    {0,   XF86XK_MonBrightnessDown, run, {.com = bridown}},

    {MOD, XK_w, run, {.com = winTeleport}},
    {MOD|ShiftMask, XK_w, run, {.com = winMenu}},

    {MOD, XK_o, run, {.com = snapLeft}},
    {MOD, XK_o, win_raise, {0}}, 
    {MOD, XK_p, run, {.com = snapRight}},
    {MOD, XK_p, win_raise, {0}}, 
    {MOD, XK_m, run, {.com = snapCenter}},
    {MOD, XK_c, win_raise, {0}}, 
    {MOD|ShiftMask, XK_m, run, {.com = moveWindows}},
    {Mod1Mask|ShiftMask, XK_p, run, {.com = arandr}},

    {MOD, XK_equal, run, {.com = winGrow}},
    {MOD, XK_minus, run, {.com = winShrink}},

    {MOD, XK_a, run, {.com = winTl}},
    {MOD, XK_a, win_raise, {0}}, 
    {MOD, XK_s, run, {.com = winTr}},
    {MOD, XK_s, win_raise, {0}}, 
    {MOD, XK_z, run, {.com = winBl}},
    {MOD, XK_z, win_raise, {0}}, 
    {MOD, XK_x, run, {.com = winBr}},
    {MOD, XK_x, win_raise, {0}}, 

    {MOD,           XK_1, ws_go,     {.i = 0}},
    {MOD|ShiftMask, XK_1, win_to_ws, {.i = 0}},
    {MOD,           XK_2, ws_go,     {.i = 1}},
    {MOD|ShiftMask, XK_2, win_to_ws, {.i = 1}},
    {MOD,           XK_3, ws_go,     {.i = 2}},
    {MOD|ShiftMask, XK_3, win_to_ws, {.i = 2}},

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
     
     {MOD, Button3, win_lower, {0}},

     {MOD|ShiftMask, Button3, win_raise, {0}},
     {MOD|ShiftMask, Button3, win_fs, {0}},
     
     {MOD|ShiftMask, Button2, win_kill, {0}},
};

#endif
